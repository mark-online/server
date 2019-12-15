#include "CommunityServerPCH.h"
#include "CommunityUserManager.h"
#include "CommunityUserAllocator.h"
#include "detail/CommunityUserImpl.h"
#include "../CommunityService.h"
#include "../channel/WorldMapChannelManager.h"
#include "../s2s/CommunityLoginServerProxy.h"
#include "../c2s/CommunityClientSessionCallback.h"
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/base/memory/ObjectPool.h>
#include <sne/core/utility/CppUtils.h>

namespace gideon { namespace communityserver {

namespace {

const bool growable = false;

/**
 * @class CommunityUserPool
 */
class CommunityUserPool : public sne::base::ObjectPool<CommunityUser, CommunityUserAllocator>
{
public:
    CommunityUserPool(CommunityUserAllocator& userAllocator, size_t poolSize) :
        sne::base::ObjectPool<CommunityUser, CommunityUserAllocator>(
            poolSize, userAllocator, growable) {}
};

} // namespace

// = CommunityUserManager

SNE_DEFINE_SINGLETON(CommunityUserManager);

CommunityUserManager::CommunityUserManager(
    std::unique_ptr<CommunityUserAllocator> communityUserAllocator,
    size_t maxUserCount) :
    communityUserAllocator_(std::move(communityUserAllocator)),
    maxUserCount_(maxUserCount),
    loginCount_(0)
{
    userPool_ = std::make_unique<CommunityUserPool>(*communityUserAllocator_, maxUserCount_);
    userPool_->initialize();
}


CommunityUserManager::~CommunityUserManager()
{
}


ErrorCode CommunityUserManager::login(const Certificate& certificate,
    CommunityClientSessionCallback& sessionCallback)
{
    {
        std::unique_lock<LockType> lock(lock_);

        ++loginCount_;

        const LoginRequestMap::const_iterator pos =
            loginRequestMap_.find(certificate.accountId_);
        if (pos != loginRequestMap_.end()) {
            return ecLoginAlreadyLoggingIn;
        }

        if (! COMMUNITY_SERVICE->getLoginServerProxy().isActivated()) {
            return ecLoginServerIsDown;
        }

        loginRequestMap_.insert(
            LoginRequestMap::value_type(certificate.accountId_,
                LoginRequest(certificate, &sessionCallback)));
    }

    COMMUNITY_SERVICE->getLoginServerProxy().m2l_loginCommunityUser(certificate);
    return ecOk;
}


ErrorCode CommunityUserManager::logout(AccountId accountId, bool shouldNotifyToLoginServer)
{
    WORLDMAP_CHANNEL_MANAGER->userLoggedOut(accountId);

    ErrorCode errorCode = ecOk;
    {
        std::unique_lock<LockType> lock(lock_);

        loginRequestMap_.erase(accountId);

        errorCode = removeUser(accountId);
    }

    if (isSucceeded(errorCode) && shouldNotifyToLoginServer) {
        COMMUNITY_SERVICE->getLoginServerProxy().m2l_logoutUser(accountId);
    }
    return errorCode;
}


void CommunityUserManager::addNickname(const Nickname& nickname, AccountId accountId)
{
    std::unique_lock<LockType> lock(lock_);

    nicknameMap_.emplace(nickname, accountId);
}


void CommunityUserManager::removeNickname(const Nickname& nickname)
{
    std::unique_lock<LockType> lock(lock_);

    nicknameMap_.erase(nickname);
}


void CommunityUserManager::loginResponsed(ErrorCode errorCode, const AccountInfo& accountInfo,
    const Certificate& reissuedCertificate)
{
    if (isSucceeded(errorCode)) {
        if (isLoggedIn(accountInfo.accountId_)) {
            loginRejected(accountInfo.accountId_, ecLoginAlreadyLoggedIn);
            return;
        }
        loginAccepted(accountInfo, reissuedCertificate);
    }
    else {
        loginRejected(accountInfo.accountId_, errorCode);
    }
}


void CommunityUserManager::userExpelledFromLoginServer(AccountId accountId, ExpelReason reason)
{
    userExpelled(accountId, reason);
}


CommunityUser* CommunityUserManager::getUser(AccountId accountId)
{
    std::unique_lock<LockType> lock(lock_);

    return getUser_i(accountId);
}


AccountIds CommunityUserManager::getOnlineUsers() const
{
    AccountIds users;

    std::unique_lock<LockType> lock(lock_);

    users.reserve(userMap_.size());

    CommunityUserMap::const_iterator pos = userMap_.begin();
    CommunityUserMap::const_iterator end = userMap_.end();
    for (; pos != end; ++pos) {
        const AccountId accountId = (*pos).first;
        users.push_back(accountId);
    }
    return users;
}


WorldUserInfos CommunityUserManager::getWorldUserInfos() const
{
    WorldUserInfos users;

    std::unique_lock<LockType> lock(lock_);

    users.reserve(userMap_.size());

    CommunityUserMap::const_iterator pos = userMap_.begin();
    CommunityUserMap::const_iterator end = userMap_.end();
    for (; pos != end; ++pos) {
        const CommunityUser* user = (*pos).second;
        users.push_back(WorldUserInfo(user->getPlayerId(), user->getNickname(),
            user->getLevel(), user->getWorldMapCode()));
    }
    return users;
}


size_t CommunityUserManager::getUserCount() const
{
    std::unique_lock<LockType> lock(lock_);

    return userMap_.size();
}


size_t CommunityUserManager::getLoginCount() const
{
    std::unique_lock<LockType> lock(lock_);

    return loginCount_;
}


void CommunityUserManager::loginAccepted(const AccountInfo& accountInfo,
    const Certificate& reissuedCertificate)
{
    ErrorCode errorCode = ecOk;
    const LoginRequest* loginRequest = nullptr;
    {
        std::unique_lock<LockType> lock(lock_);

        const LoginRequestMap::iterator pos =
            loginRequestMap_.find(accountInfo.accountId_);
        if (pos == loginRequestMap_.end()) {
            return;
        }
        loginRequest = &(*pos).second;

        errorCode = addUser(accountInfo, *loginRequest);
    }

    {
        std::unique_lock<LockType> lock(lock_);

        loginRequest->sessionCallback_->loginResponsed(errorCode, reissuedCertificate);
        loginRequestMap_.erase(accountInfo.accountId_);
    }

    if (isFailed(errorCode)) {
        COMMUNITY_SERVICE->getLoginServerProxy().m2l_logoutUser(accountInfo.accountId_);
    }
}

void CommunityUserManager::loginRejected(AccountId accountId, ErrorCode errorCode)
{
    std::unique_lock<LockType> lock(lock_);

    const LoginRequestMap::iterator pos =
        loginRequestMap_.find(accountId);
    if (pos == loginRequestMap_.end()) {
        return;
    }
    LoginRequest& loginRequest = (*pos).second;

    loginRequest.sessionCallback_->loginResponsed(errorCode, Certificate());
}


ErrorCode CommunityUserManager::addUser(const AccountInfo& accountInfo,
    const LoginRequest& loginRequest)
{
    if (isExists(accountInfo.accountId_)) {
        return ecLoginAlreadyLoggedIn;
    }

    if (userMap_.size() >= maxUserCount_) {
        SNE_LOG_WARNING("Too many users(%u >= %u).",
            userMap_.size(), maxUserCount_);
        return ecCommunityIsFull;
    }

    CommunityUser* newUser = userPool_->acquire();
    if (! newUser) {
        SNE_LOG_WARNING("Too many users(%u >= %u).",
            userMap_.size(), maxUserCount_);
        return ecCommunityIsFull;
    }

    newUser->initialize(accountInfo, *loginRequest.sessionCallback_, *this);

    userMap_.emplace(accountInfo.accountId_, newUser);

    // TODO 수정
    SNE_LOG_INFO("User(A%" PRIu64 "[%W]) logged in. total=%u.",
        accountInfo.accountId_, accountInfo.userId_.c_str(),
        userMap_.size());

    return ecOk;
}


ErrorCode CommunityUserManager::removeUser(AccountId accountId)
{
    CommunityUser* user = getUser_i(accountId);
    if (! user) {
        return ecCommunityUserNotExist;
    }

    userMap_.erase(accountId);

    user->finalize();

    userPool_->release(user);

    SNE_LOG_INFO("User(A%" PRIu64 ") logged out. total=%u.",
        accountId, userMap_.size());

    return ecOk;
}


void CommunityUserManager::userExpelled(AccountId accountId, ExpelReason reason)
{
    CommunityUser* user = getUser(accountId);
    if (user != nullptr) {
        user->expelledFromServer(reason);
    }

    (void)logout(accountId, false);

    SNE_LOG_INFO("User(A%" PRIu64 ") expelled(%d).",
        accountId, reason);
}


AccountId CommunityUserManager::getAccountId(const Nickname& nickname) const
{
    std::unique_lock<LockType> lock(lock_);

    const CommunityNicknameMap::const_iterator pos = nicknameMap_.find(nickname);
    if (pos != nicknameMap_.end()) {
        return (*pos).second;
    }
    return invalidAccountId;
}


CommunityUser* CommunityUserManager::getUser(AccountId accountId, ObjectId playerId)
{
    std::unique_lock<LockType> lock(lock_);

    CommunityUser* user = getUser_i(accountId);
    if (! user) {
        return nullptr;
    }
    
    return user->getPlayerId() == playerId ? user : nullptr;
}

}} // namespace gideon { namespace communityserver {
