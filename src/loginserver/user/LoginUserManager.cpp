#include "LoginServerPCH.h"
#include "LoginUserManager.h"
#include "../s2s/LoginServerSideProxyManager.h"
#include "../c2s/LoginClientSessionCallback.h"
#include "../LoginService.h"
#include "../shard/ShardManager.h"
#include <gideon/serverbase/database/AccountDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Logger.h>
#include <sne/base/utility/Assert.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <boost/algorithm/string/classification.hpp>

namespace gideon { namespace loginserver {

namespace {

ErrorCode checkNickname(const Nickname& nickname)
{
    using namespace boost;

    if (nickname.size() < minNicknameLength) {
        return ecCharacterNicknameTooShort;
    }
    if (! all(nickname, ! is_space() && ! is_cntrl() && ! is_punct())) {
        return ecCharacterInvalidNickname;
    }
    return ecOk;
}


ErrorCode dbGetAccountInfo(AccountInfo& accountInfo, AccountId accountId)
{
    sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->getAccountInfo(accountInfo, accountId)) {
        return ecDatabaseInternalError;
    }

    return ecOk;
}


ErrorCode dbGetCharacterCounts(CharacterCountPerShardMap& characterCounts,
    AccountId accountId)
{
    sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->getCharacterCounts(characterCounts, accountId)) {
        return ecDatabaseInternalError;
    }

    return ecOk;
}


ErrorCode checkAccountInfo(const UserId& userId, const UserPassword& userPassword,
    const std::string& userEmail)
{
    using namespace boost;

    if (userId.size() < minUserIdLength) {
        return ecLoginUserIdTooShort;
    }
    if (! all(userId, is_alnum())) {
        return ecLoginInvalidUserId;
    }

    if (userPassword.size() < minUserPasswordLength) {
        return ecLoginUserPasswordTooShort;
    }
    // TODO: 비밀번호 문자 검사??
    //if (! all(userPassword, ! is_space() && ! is_cntrl())) {
    //    return ecLoginInvalidUserPassword;
    //}

    if (userEmail.size() > maxUserEmailLength) {
        return ecLoginInvalidUserEmail;
    }

    if (! isValidEmail(userEmail)) {
        return ecLoginInvalidUserEmail;
    }
    return ecOk;
}

} // namespace

// = LoginUserManager

SNE_DEFINE_SINGLETON(LoginUserManager);

LoginUserManager::LoginUserManager(size_t maxUserCount, time_t migrationEffectivePeriod) :
    maxUserCount_(maxUserCount),
    migrationEffectivePeriod_(migrationEffectivePeriod),
    loginCount_(0)
{
    if (TASK_SCHEDULER != nullptr) {
        migrationCheckTask_ = TASK_SCHEDULER->schedule(*this, 0, 10000);
    }
}


LoginUserManager::~LoginUserManager()
{
    sne::base::Future::Ref task = migrationCheckTask_.lock();
    if (task.get() != nullptr) {
        task->cancel();
    }
}


ErrorCode LoginUserManager::login(AccountInfo& accountInfo, CertificateMap& certificateMap,
    const UserId& userId, const UserPassword& userPassword,
    LoginClientSessionCallback* callback)
{
    ++loginCount_;

    AccountId accountId = invalidAccountId;
    {
        sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

        if (! db->authenticate(accountId, userId, userPassword)) {
            return ecAuthenticateFailed;
        }

        if (! isValidAccountId(accountId)) {
            return ecAuthenticateFailed;
        }
    }

    accountInfo.accountId_ = accountId;

    {
        std::unique_lock<LockType> lock(lock_);

        if (isLoggedIn(accountId)) {
            return ecLoginAlreadyLoggedIn;
        }

        const ErrorCode errorCode = registerAuthenticatedUser(accountId, callback);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        LoginUser* user = getLoginUser(accountId);
        if (! user) {
            assert(false);
            return ecAuthenticateFailed;
        }

        accountInfo = user->getAccountInfo();
        certificateMap = user->getCertificateMap();
    }

    return ecOk;
}


ErrorCode LoginUserManager::relogin(Certificate& reinssuedCertificate,
    const Certificate& certificate)
{
    std::unique_lock<LockType> lock(lock_);

    LoginUser* user = getLoginUser(certificate.accountId_);
    if (! user) {
        return ecLoginInvalidCertificate;
    }

    const ErrorCode errorCode = user->relogin(certificate);
    if (isSucceeded(errorCode)) {
        reinssuedCertificate = user->getCertificate(certificate.serverType_);
    }
    return errorCode;
}


ErrorCode LoginUserManager::reserveNickname(AccountId accountId, const Nickname& nickname)
{
    const ErrorCode errorCode = checkNickname(nickname);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    {
        std::unique_lock<LockType> lock(lock_);

        if (reserveNicknames_.find(nickname) != reserveNicknames_.end()) {
            return ecCharacterDuplicatedNickname;
        }

        reserveNicknames_.insert(nickname);
        accountReserveNicknames_.emplace(accountId, nickname);
    }
    return ecOk;    
}


void LoginUserManager::deleteReservedNickname(AccountId accountId)
{
    std::unique_lock<LockType> lock(lock_);

    deleteReservedNickname_i(accountId);
}


ErrorCode LoginUserManager::loginZoneUser(AccountInfo& accountInfo,
    Certificate& reinssuedCertificate,
    sne::server::ServerId zoneServerId, ShardId shardId, const Certificate& certificate)
{
    const AccountId accountId = certificate.accountId_;

    ErrorCode errorCode = ecOk;
    {
        std::unique_lock<LockType> lock(lock_);

        LoginUser* user = getLoginUser(accountId);
        if (! user) {
            return ecLoginIsNotLoginUser;
        }

        errorCode = user->loginZoneServer(zoneServerId, shardId, certificate);
        if (isSucceeded(errorCode)) {
            migratingUsers_.erase(accountId);
            accountInfo = user->getAccountInfo();
            reinssuedCertificate = user->getCertificate(certificate.serverType_);
        }
        else {
            if (errorCode != ecLoginAlreadyLoggedIn) {
                removeLoginUser(accountId);
            }
        }        
    }

    return errorCode;
}


ErrorCode LoginUserManager::loginCommunityUser(AccountInfo& accountInfo,
    Certificate& reinssuedCertificate,
    sne::server::ServerId communityServerId, const Certificate& certificate)
{
    const AccountId accountId = certificate.accountId_;

    ErrorCode errorCode = ecOk;
    {
        std::unique_lock<LockType> lock(lock_);

        LoginUser* user = getLoginUser(accountId);
        if (! user) {
            return ecLoginIsNotLoginUser;
        }

        errorCode = user->loginCommunityServer(communityServerId, certificate);
        if (isSucceeded(errorCode)) {
            accountInfo = user->getAccountInfo();
            reinssuedCertificate = user->getCertificate(certificate.serverType_);
        }
        else {
            if (errorCode != ecLoginAlreadyLoggedIn) {
                removeLoginUser(accountId);
            }
        }        
    }

    return errorCode;
}


bool LoginUserManager::logout(AccountId accountId, ServerType serverType,
    LoginClientSessionCallback* callback)
{
    if (serverType == stLoginServer) {
        SHARD_MANAGER->cancelRequests(accountId, callback);
    }

    {
        std::unique_lock<LockType> lock(lock_);

        LoginUser* loginUser = getLoginUser(accountId);
        if (!loginUser) {
            return false;
        }

        loginUser->logout(serverType, callback);

        if (loginUser->shouldLogout()) {
            logout(accountId, *loginUser);
        }

        deleteReservedNickname_i(accountId);
    }
    return true;
}


void LoginUserManager::shardChanged(const FullUserInfo& userInfo, ShardId shardId)
{
    std::unique_lock<LockType> lock(lock_);

    LoginUser* loginUser = getLoginUser(userInfo.accountId_);
    if (! loginUser) {
        return;
    }

    loginUser->shardChanged(shardId);
    loginUser->userInfoChanged(userInfo);
}


void LoginUserManager::expelUser(AccountId accountId, ExpelReason reason)
{
    LOGINSERVERSIDEPROXY_MANAGER->expelUser(accountId, erDuplicatedLogin);

    {
        std::unique_lock<LockType> lock(lock_);

        LoginUser* loginUser = getLoginUser(accountId);
        if (! loginUser) {
            return;
        }

        LoginClientSessionCallback* callback = loginUser->getCallback();
        if (callback != nullptr) {
            callback->expelled(reason);
            SHARD_MANAGER->cancelRequests(accountId, callback);
        }

        removeLoginUser(accountId);
    }
}


void LoginUserManager::reserveMigration(AccountId accountId)
{
    std::unique_lock<LockType> lock(lock_);

    LoginUser* loginUser = getLoginUser(accountId);
    if (! loginUser) {
        return;
    }

    loginUser->reserveMigration();

    migratingUsers_.insert(accountId);
}


void LoginUserManager::zoneServerConnected(sne::server::ServerId zoneServerId,
    ShardId shardId, const AccountIds& onlineUsers)
{
    for (const AccountId accountId : onlineUsers) {
        (void)loginFromZoneServer(accountId, zoneServerId, shardId);
    }
}


void LoginUserManager::zoneServerDisconnected(sne::server::ServerId zoneServerId)
{
    sne::core::Vector<LoginUser> logoutUsers;
    logoutUsers.reserve(1000);

    {
        std::unique_lock<LockType> lock(lock_);

        for (OnlineUsers::value_type& value : onlineUsers_) {
            LoginUser& loginUser = value.second;

            if (loginUser.getZoneServerId() == zoneServerId) {
                loginUser.logout(stZoneServer);
                logoutUsers.push_back(loginUser);
            }
        }

        for (LoginUser& loginUser : logoutUsers) {
            if (loginUser.shouldLogout()) {
                logout(loginUser.getAccountId(), loginUser);
            }
        }
    }
}


void LoginUserManager::communityServerConnected(
    sne::server::ServerId communityServerId, const AccountIds& onlineUsers)
{
    for (const AccountId accountId : onlineUsers) {
        (void)loginFromCommunityServer(accountId, communityServerId);
    }
}


void LoginUserManager::communityServerDisconnected()
{
    OnlineUsers logoutUsers;
    {
        std::unique_lock<LockType> lock(lock_);

        for (OnlineUsers::value_type& value : onlineUsers_) {
            LoginUser& loginUser = value.second;
            loginUser.logout(stCommunityServer);
            logoutUsers.insert(value);
        }
    }

    for (OnlineUsers::value_type& value : logoutUsers) {
        const AccountId accountId = value.first;
        LoginUser& loginUser = value.second;

        std::unique_lock<LockType> lock(lock_);

        if (loginUser.shouldLogout()) {
            logout(accountId, loginUser);
        }
    }
}


void LoginUserManager::characterCreated(ShardId shardId, AccountId accountId,
    const FullCharacterInfo& characterInfo)
{
    uint8_t characterCount = 0;
    {
        std::unique_lock<LockType> lock(lock_);

        LoginUser* loginUser = getLoginUser(accountId);
        if (! loginUser) {
            return;
        }

        characterCount = loginUser->characterCreated(shardId, characterInfo);
    }

    {
        sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

        (void)db->updateCharacterCount(accountId, shardId, characterCount);
    }
}


void LoginUserManager::characterDeleted(ShardId shardId, AccountId accountId,
    ObjectId characterId)
{
    uint8_t characterCount = 0;
    {
        std::unique_lock<LockType> lock(lock_);

        LoginUser* loginUser = getLoginUser(accountId);
        if (! loginUser) {
            return;
        }

        characterCount = loginUser->characterDeleted(shardId, characterId);
    }

    {
        sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

        (void)db->updateCharacterCount(accountId, shardId, characterCount);
    }
}


size_t LoginUserManager::getUserCount() const
{
    std::unique_lock<LockType> lock(lock_);

    return onlineUsers_.size();
}


size_t LoginUserManager::getLoginCount() const
{
    std::unique_lock<LockType> lock(lock_);

    return loginCount_;
}


bool LoginUserManager::isOnline(AccountId accountId) const
{
    std::unique_lock<LockType> lock(lock_);

    return isLoggedIn(accountId);
}


CharacterCountPerShardMap LoginUserManager::getCharacterCounts(AccountId accountId) const
{
    std::unique_lock<LockType> lock(lock_);

    const LoginUser* loginUser = getLoginUser(accountId);
    if (! loginUser) {
        return CharacterCountPerShardMap();
    }

    return loginUser->getCharacterCounts();
}


bool LoginUserManager::queryZoneServerInfo(ShardId& shardId, ZoneId& zoneId,
    AccountId accountId, ObjectId characterId)
{
    std::unique_lock<LockType> lock(lock_);

    LoginUser* loginUser = getLoginUser(accountId);
    if (! loginUser) {
        return invalidShardId;
    }

    shardId = loginUser->getCurrentShardId();
    zoneId = loginUser->getLastAccessZoneId(characterId);

    return isValidShardId(shardId) && isValidZoneId(zoneId);
}


ShardId LoginUserManager::getSelectedShardId(AccountId accountId) const
{
    std::unique_lock<LockType> lock(lock_);

    const LoginUser* loginUser = getLoginUser(accountId);
    if (! loginUser) {
        return invalidShardId;
    }

    return loginUser->getCurrentShardId();
}


ErrorCode LoginUserManager::loginFromZoneServer(AccountId accountId,
    sne::server::ServerId zoneServerId, ShardId shardId)
{
    std::unique_lock<LockType> lock(lock_);

    LoginUser* loginUser = getLoginUser(accountId);
    if (loginUser != nullptr) {
        if (loginUser->isConnectedToZoneServer()) {
            LOGINSERVERSIDEPROXY_MANAGER->expelUser(accountId, erDuplicatedLogin,
                zoneServerId);
            return ecOk;
        }
    }
    else {
        const ErrorCode errorCode = registerAuthenticatedUser(accountId);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }

    loginUser = getLoginUser(accountId);
    if (! loginUser) {
        assert(false);
        return ecServerInternalError;
    }

    loginUser->loggedInZoneServer(zoneServerId, shardId);
    // TODO: loginUser->setCharacterId(...);

    SNE_LOG_INFO("User(A%" PRIu64 ") logged in S%u. total=%u.",
        accountId, zoneServerId, onlineUsers_.size());
    return ecOk;
}


ErrorCode LoginUserManager::loginFromCommunityServer(AccountId accountId,
    sne::server::ServerId communityServerId)
{
    std::unique_lock<LockType> lock(lock_);

    LoginUser* loginUser = getLoginUser(accountId);
    if (loginUser != nullptr) {
        if (loginUser->isConnectedToCommunityServer()) {
            LOGINSERVERSIDEPROXY_MANAGER->expelUser(accountId, erDuplicatedLogin,
                communityServerId);
            return ecOk;
        }
    }
    else {
        const ErrorCode errorCode = registerAuthenticatedUser(accountId);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }

    loginUser = getLoginUser(accountId);
    if (! loginUser) {
        assert(false);
        return ecServerInternalError;
    }

    loginUser->loggedInCommunityServer(communityServerId);

    SNE_LOG_INFO("User(A%" PRIu64 ") logged in CommunityServer(S%u). total=%u.",
        accountId, communityServerId, onlineUsers_.size());
    return ecOk;
}


void LoginUserManager::logout(AccountId accountId, LoginUser& loginUser)
{
    const AccountInfo& accountInfo = loginUser.getAccountInfo();
    {
        // TODO: 비동기 DB I/O
        sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

        (void)db->logLogout(accountId, accountInfo.lastAccessShardId_,
            loginUser.getSessionSeconds());
    }

    loginUser.disconnect();

    removeLoginUser(accountId);

    SNE_LOG_INFO("User(A%" PRIu64 ") logged out. total=%u.",
        accountId, onlineUsers_.size());
}


ErrorCode LoginUserManager::registerAuthenticatedUser(AccountId accountId,
    LoginClientSessionCallback* callback)
{
    assert(! isLoggedIn(accountId));

    AccountInfo accountInfo;
    const ErrorCode errorCode = dbGetAccountInfo(accountInfo, accountId);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    CharacterCountPerShardMap characterCounts;
    (void)dbGetCharacterCounts(characterCounts, accountId);

    addLoginUser(accountInfo, characterCounts, callback);

    {
        sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

        (void)db->updateLoginAt(accountId);
    }

    return ecOk;
}


void LoginUserManager::addLoginUser(const AccountInfo& accountInfo,
    const CharacterCountPerShardMap& characterCounts,
    LoginClientSessionCallback* callback)
{
    LoginUser loginUser(callback, accountInfo, characterCounts);
    loginUser.loginLoginServer();
    onlineUsers_.emplace(accountInfo.accountId_, loginUser);
}


void LoginUserManager::removeLoginUser(AccountId accountId)
{
    onlineUsers_.erase(accountId);
    migratingUsers_.erase(accountId);

    // TODO: DB 테이블에서 삭제한다
}


void LoginUserManager::checkExpiredMigration()
{
    std::unique_lock<LockType> lock(lock_);

    AccountIdSet::iterator pos = migratingUsers_.begin();
    for (; pos != migratingUsers_.end();) {
        const AccountId accountId = *pos;
        bool shouldErase = false;
        bool shouldLogout = false;
        LoginUser* user = getLoginUser(accountId);
        if (user != nullptr) {
            if (user->isMigrationExpired(migrationEffectivePeriod_)) {
                user->migrationFailed();
                if (user->shouldLogout()) {
                    shouldLogout = true;
                }
                shouldErase = true;
            }
        }
        else {
            shouldErase = true;
        }

        if (shouldLogout) {
            ++pos;
            logout(accountId, *user);
        }
        else {
            if (shouldErase) {
                migratingUsers_.erase(pos++);
            }
            else {
                ++pos;
            }
        }
    }
}


void LoginUserManager::deleteReservedNickname_i(AccountId accountId)
{
    NicknameMap::iterator pos = accountReserveNicknames_.find(accountId);
    if (pos != accountReserveNicknames_.end()) {
        reserveNicknames_.erase((*pos).second);
    }
    accountReserveNicknames_.erase(accountId);
}


// = sne::base::Runnable overriding

void LoginUserManager::run()
{
    checkExpiredMigration();
}

}} // namespace gideon { namespace loginserver {
