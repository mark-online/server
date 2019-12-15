#include "ZoneServerPCH.h"
#include "ZoneUserManager.h"
#include "ZoneUserAllocator.h"
#include "detail/ZoneUserImpl.h"
#include "../ZoneService.h"
#include "../service/item/ItemIdGenerator.h"
#include "../s2s/ZoneLoginServerProxy.h"
#include "../c2s/ZoneClientSessionCallback.h"
#include "../world/World.h"
#include "../world/WorldMap.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/base/memory/ObjectPool.h>
#include <sne/core/utility/CppUtils.h>

namespace gideon { namespace zoneserver {

namespace {

const bool growable = false;

/**
 * @class ZoneUserPool
 */
class ZoneUserPool : public sne::base::ObjectPool<ZoneUser, ZoneUserAllocator>
{
public:
    ZoneUserPool(ZoneUserAllocator& zoneUserAllocator,
        size_t poolSize) :
        sne::base::ObjectPool<ZoneUser, ZoneUserAllocator>(
            poolSize, zoneUserAllocator, growable) {}
};


/**
 * @class GetFullUserInfoFutureObserverForUser
 */
class GetFullUserInfoFutureObserverForUser : public sne::base::FutureObserver,
    public sne::core::ThreadSafeMemoryPoolMixin<GetFullUserInfoFutureObserverForUser>
{
public:
    GetFullUserInfoFutureObserverForUser() {}

private:
    virtual void update(const sne::base::Future& future) {
        const serverbase::GetFullUserInfoRequestFuture& requestFuture =
            static_cast<const serverbase::GetFullUserInfoRequestFuture&>(future);
        ZONEUSER_MANAGER->userInfoResponsed(requestFuture.errorCode_,
            requestFuture.accountInfo_, requestFuture.userInfo_);
    }

    virtual void deleteFutureObserver() {
        delete this;
    }
};

} // namespace

// = ZoneUserManager

SNE_DEFINE_SINGLETON(ZoneUserManager);

ZoneUserManager::ZoneUserManager(
    std::unique_ptr<ZoneUserAllocator> zoneUserAllocator,
    size_t maxUserCount) :
    zoneUserAllocator_(std::move(zoneUserAllocator)),
    maxUserCount_(maxUserCount),
    loginCount_(0)
{
    userPool_ = std::make_unique<ZoneUserPool>(*zoneUserAllocator_, maxUserCount_);
    userPool_->initialize();
}


ZoneUserManager::~ZoneUserManager()
{
}


ErrorCode ZoneUserManager::login(const Certificate& certificate, ObjectId arenaId,
	RegionCode spawnRegionCode, MapCode prevGlobalMapCode,
    const Position& position, ZoneClientSessionCallback& sessionCallback)
{
    {
        std::lock_guard<LockType> lock(lock_);

        ++loginCount_;

        ZoneUser* user = getUser_i(certificate.accountId_);
        if (user != nullptr) {
            if (user->isLoggingOut()) {
                return ecLoginLoggingOut;
            }
        }

        const LoginRequestMap::const_iterator pos =
            loginRequestMap_.find(certificate.accountId_);
        if (pos != loginRequestMap_.end()) {
            return ecLoginAlreadyLoggingIn;
        }

        if (! ZONE_SERVICE->getLoginServerProxy().isActivated()) {
            return ecLoginServerIsDown;
        }

        loginRequestMap_.insert(
            LoginRequestMap::value_type(certificate.accountId_,
                LoginRequest(prevGlobalMapCode, arenaId, certificate, spawnRegionCode, position, &sessionCallback)));
    }

    ZONE_SERVICE->getLoginServerProxy().z2l_loginZoneUser(certificate);
    return ecOk;
}


ErrorCode ZoneUserManager::logout(AccountId accountId)
{
    ZoneUser* user = getUser(accountId);
    if (user != nullptr) {
        (void)user->leaveFromWorld();
    }

    ErrorCode errorCode = ecOk;
    {
        std::lock_guard<LockType> lock(lock_);

        loginRequestMap_.erase(accountId);

        errorCode = removeUser(accountId);
    }

    if (isSucceeded(errorCode)) {
        ZONE_SERVICE->getLoginServerProxy().z2l_logoutUser(accountId);
    }
    return errorCode;
}


void ZoneUserManager::loginResponsed(ErrorCode errorCode, const AccountInfo& accountInfo,
    const Certificate& reissuedCertificate)
{
    if (isSucceeded(errorCode)) {
        sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);
        sne::base::Future::Ref future = db->asyncGetFullUserInfo(accountInfo);
        if (future.get() != nullptr) {
            certificateReissued(accountInfo.accountId_, reissuedCertificate);
            future->attach(new GetFullUserInfoFutureObserverForUser);
            return;
        }
        errorCode = ecDatabaseInternalError;
    }

    loginRejected(accountInfo.accountId_, errorCode);
}


void ZoneUserManager::userInfoResponsed(ErrorCode errorCode, const AccountInfo& accountInfo,
    const FullUserInfo& userInfo)
{
    if (isSucceeded(errorCode)) {
        if (isLoggedIn(userInfo.accountId_)) {
            errorCode = ecLoginAlreadyLoggedIn;
        }
        else if (! userInfo.hasCharacter()) {
            errorCode = ecCharacterNotFound;
        }
    }

    if (isSucceeded(errorCode)) {
        loginAccepted(accountInfo, userInfo);
    }
    else {
        loginRejected(userInfo.accountId_, errorCode);
    }
}


void ZoneUserManager::userExpelled(AccountId accountId, ExpelReason reason)
{
    ZoneUser* user = getUser(accountId);
    if (! user) {
        return;
    }

    if (user->isLoggingOut()) {
        return;
    }

    user->leaveFromWorld();
    user->expelledFromServer(reason);

    SNE_LOG_INFO("User(A%" PRIu64 ") expelled(%d).",
        accountId, reason);

    {
        std::lock_guard<LockType> lock(lock_);

        (void)removeUser(accountId);
    }
}


AccountIds ZoneUserManager::getOnlineUsers() const
{
    AccountIds users;

    std::lock_guard<LockType> lock(lock_);

    users.reserve(userMap_.size());

    ZoneUserMap::const_iterator pos = userMap_.begin();
    ZoneUserMap::const_iterator end = userMap_.end();
    for (; pos != end; ++pos) {
        const AccountId accountId = (*pos).first;
        users.push_back(accountId);
    }
    return users;
}


ZoneUser* ZoneUserManager::getUser(AccountId accountId)
{
    std::lock_guard<LockType> lock(lock_);

    return getUser_i(accountId);
}


size_t ZoneUserManager::getUserCount() const
{
    std::lock_guard<LockType> lock(lock_);

    return userMap_.size();
}


size_t ZoneUserManager::getLoginCount() const
{
    std::lock_guard<LockType> lock(lock_);

    return loginCount_;
}


bool ZoneUserManager::hasNickname(AccountId accountId, const Nickname& nickname) const
{
    const ZoneUser* user = getUser_i(accountId);
    if (! user) {
        return false;
    }
    return user->hasNickname(nickname);
}


void ZoneUserManager::loginAccepted(const AccountInfo& accountInfo,
    const FullUserInfo& userInfo)
{
    ErrorCode errorCode = ecOk;
    const LoginRequest* loginRequest = nullptr;
    {
        std::lock_guard<LockType> lock(lock_);

        const LoginRequestMap::iterator pos =
            loginRequestMap_.find(userInfo.accountId_);
        if (pos == loginRequestMap_.end()) {
            return;
        }
        loginRequest = &(*pos).second;

        errorCode = addUser(accountInfo, userInfo, *loginRequest);
    }

    {
        std::lock_guard<LockType> lock(lock_);

        loginRequest->sessionCallback_->loginResponsed(errorCode,
            loginRequest->reissuedCertificate_);
        loginRequestMap_.erase(userInfo.accountId_);
    }

    if (isFailed(errorCode)) {
        ZONE_SERVICE->getLoginServerProxy().z2l_logoutUser(userInfo.accountId_);
    }
}


void ZoneUserManager::loginRejected(AccountId accountId, ErrorCode errorCode)
{
    std::lock_guard<LockType> lock(lock_);

    const LoginRequestMap::iterator pos =
        loginRequestMap_.find(accountId);
    if (pos == loginRequestMap_.end()) {
        return;
    }
    LoginRequest& loginRequest = (*pos).second;

    loginRequest.sessionCallback_->loginResponsed(errorCode, Certificate());
}


void ZoneUserManager::certificateReissued(AccountId accountId, const Certificate& certificate)
{
    std::lock_guard<LockType> lock(lock_);

    const LoginRequestMap::iterator pos =
        loginRequestMap_.find(accountId);
    if (pos == loginRequestMap_.end()) {
        return;
    }
    LoginRequest& loginRequest = (*pos).second;

    loginRequest.reissuedCertificate_ = certificate;
}


ErrorCode ZoneUserManager::addUser(const AccountInfo& accountInfo, const FullUserInfo& userInfo,
    const LoginRequest& loginRequest)
{
    if (isExists(userInfo.accountId_)) {
        return ecLoginAlreadyLoggedIn;
    }

    if (userMap_.size() >= maxUserCount_) {
        SNE_LOG_WARNING("Too many users(%u >= %u).",
            userMap_.size(), maxUserCount_);
        return ecZoneIsFull;
    }

    ZoneUser* newUser = userPool_->acquire();
    if (! newUser) {
        SNE_LOG_WARNING("Too many users(%u >= %u).",
            userMap_.size(), maxUserCount_);
        return ecZoneIsFull;
    }

    newUser->initialize(accountInfo, userInfo, loginRequest.certificate_.authId_,
        loginRequest.spawnRegionCode_, loginRequest.arenaId_, loginRequest.preGlobalMapCode_,
        loginRequest.position_, *loginRequest.sessionCallback_);

    userMap_.emplace(userInfo.accountId_, newUser);

	// TODO 수정
    SNE_LOG_INFO("User(A%" PRIu64 "[%W]) logged in. total=%u.",
        userInfo.accountId_, accountInfo.userId_.c_str(),
        userMap_.size());

    return ecOk;
}


ErrorCode ZoneUserManager::removeUser(AccountId accountId)
{
    ZoneUser* user = getUser_i(accountId);
    if (! user) {
        return ecZoneUserNotExist;
    }

    userMap_.erase(accountId);

    user->finalize();

    userPool_->release(user);

    SNE_LOG_INFO("User(A%" PRIu64 ") logged out. total=%u.",
        accountId, userMap_.size());

    return ecOk;
}

}} // namespace gideon { namespace zoneserver {
