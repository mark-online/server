#pragma once

#include "../communityserver_export.h"
#include "CommunityUserHelper.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/CheatInfo.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/server/session/ClientId.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace communityserver {

class CommunityClientSessionCallback;
class CommunityUser;
class CommunityUserAllocator;

namespace {
class CommunityUserPool;
} // namespace

/**
 * @class CommunityUserManager
 *
 * Community 사용자 관리자
 */
class CommunityServer_Export CommunityUserManager : public boost::noncopyable,
    public CommunityUserHelper
{
    typedef sne::core::HashMap<Nickname, AccountId> CommunityNicknameMap;
    typedef sne::core::HashMap<AccountId, CommunityUser*> CommunityUserMap;
    typedef sne::core::HashMap<sne::server::ClientId, AccountId> ClientIds;

    typedef std::mutex LockType;

    /**
     * @struct LoginRequest
     */
    struct LoginRequest {
        Certificate certificate_;
        CommunityClientSessionCallback* sessionCallback_;

        explicit LoginRequest(const Certificate& certificate = Certificate(),
            CommunityClientSessionCallback* sessionCallback = nullptr) :
            certificate_(certificate),
            sessionCallback_(sessionCallback) {}
    };

    typedef sne::core::HashMap<AccountId, LoginRequest> LoginRequestMap;

    SNE_DECLARE_SINGLETON(CommunityUserManager);
public:
    CommunityUserManager(
        std::unique_ptr<CommunityUserAllocator> communityUserAllocator,
        size_t maxUserCount);
    ~CommunityUserManager();

public:
    /// 사용자를 로그인 처리한다
    ErrorCode login(const Certificate& certificate,
        CommunityClientSessionCallback& sessionCallback);

    /// 사용자를 로그아웃 처리한다
    ErrorCode logout(AccountId accountId, bool shouldNotifyToLoginServer = true);

    void addNickname(const Nickname& nickname, AccountId accountId);
    void removeNickname(const Nickname& nickname);

public:
    /// 로그인 서버가 로그인 요청에 대한 응답을 하였다
    void loginResponsed(ErrorCode errorCode, const AccountInfo& accountInfo,
        const Certificate& reissuedCertificate);
    
    /// 사용자가 로그인 서버로 부터 강제 퇴장 되었다.
    void userExpelledFromLoginServer(AccountId accountId, ExpelReason reason);
public:
    // = CommunityUserHelper overriding
    virtual CommunityUser* getUser(AccountId accountId);
    
public:
    AccountIds getOnlineUsers() const;

    WorldUserInfos getWorldUserInfos() const;

    size_t getUserCount() const;
    size_t getLoginCount() const;

    bool isLoggedIn(AccountId accountId) const {
        return getUser_i(accountId) != nullptr;
    }

private:
    void loginAccepted(const AccountInfo& accountInfo, const Certificate& reissuedCertificate);
    void loginRejected(AccountId accountId, ErrorCode errorCode);

    ErrorCode addUser(const AccountInfo& accountInfo, const LoginRequest& loginRequest);
    ErrorCode removeUser(AccountId accountId);

    void userExpelled(AccountId accountId, ExpelReason reason);

private:
    CommunityUser* getUser_i(AccountId accountId) {
        return sne::core::search_map(userMap_, accountId, nullptr);
    }

    const CommunityUser* getUser_i(AccountId accountId) const {
        return sne::core::search_map(userMap_, accountId, nullptr);
    }

    bool isExists(AccountId accountId) const {
        return userMap_.find(accountId) != userMap_.end();
    }

private:
    // = CommunityUserHelper overriding
    virtual AccountId getAccountId(const Nickname& nickname) const;
    virtual CommunityUser* getUser(AccountId accountId, ObjectId playerId);

private:
    const size_t maxUserCount_;

    std::unique_ptr<CommunityUserAllocator> communityUserAllocator_;
    std::unique_ptr<CommunityUserPool> userPool_;
    CommunityUserMap userMap_; ///< 로그인한 사용자 목록
    CommunityNicknameMap nicknameMap_; /// 로그인한 사용자 닉네임
    LoginRequestMap loginRequestMap_;

    size_t loginCount_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace communityserver {

#define COMMUNITYUSER_MANAGER gideon::communityserver::CommunityUserManager::instance()
