#pragma once

#include "../zoneserver_export.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/server/session/ClientId.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

class ZoneClientSessionCallback;
class ZoneUser;
class ZoneUserAllocator;

namespace {
class ZoneUserPool;
} // namespace

/**
 * @class ZoneUserManager
 *
 * Zone 사용자 관리자
 */
class ZoneServer_Export ZoneUserManager : public boost::noncopyable
{
    typedef sne::core::HashMap<AccountId, ZoneUser*> ZoneUserMap;
    typedef sne::core::HashMap<sne::server::ClientId, AccountId> ClientIds;

    using LockType = std::mutex;

    /**
     * @struct LoginRequest
     */
    struct LoginRequest {
        Certificate certificate_;
        RegionCode spawnRegionCode_;
		ObjectId arenaId_;
        MapCode preGlobalMapCode_;
        Position position_;
        ZoneClientSessionCallback* sessionCallback_;
        Certificate reissuedCertificate_;

        explicit LoginRequest(MapCode preGlobalMapCode, ObjectId arenaId,
            const Certificate& certificate = Certificate(),
            RegionCode spawnRegionCode = invalidRegionCode,
            const Position& position = Position(),
            ZoneClientSessionCallback* sessionCallback = nullptr) :
			arenaId_(arenaId),
            preGlobalMapCode_(preGlobalMapCode),
            certificate_(certificate),
            spawnRegionCode_(spawnRegionCode),
            position_(position),
            sessionCallback_(sessionCallback) {}
    };

    typedef sne::core::HashMap<AccountId, LoginRequest> LoginRequestMap;

    SNE_DECLARE_SINGLETON(ZoneUserManager);
public:
    ZoneUserManager(std::unique_ptr<ZoneUserAllocator> zoneUserAllocator,
        size_t maxUserCount);
    ~ZoneUserManager();

public:
    /// 사용자를 로그인 처리한다
    ErrorCode login(const Certificate& certificate, ObjectId arenaId, 
		RegionCode spawnRegionCode, MapCode prevGlobalMapCode, 
        const Position& position, ZoneClientSessionCallback& sessionCallback);

    /// 사용자를 로그아웃 처리한다
    ErrorCode logout(AccountId accountId);

public:
    /// 로그인 서버로 부터 로그인 요청에 대한 응답이 도착하였다
    void loginResponsed(ErrorCode errorCode, const AccountInfo& accountInfo,
        const Certificate& reissuedCertificate);

    /// 사용자가 강제 퇴장 되었다.
    void userExpelled(AccountId accountId, ExpelReason reason);

public:
    /// DB proxy로 부터 사용자 정보가 도착하였다
    void userInfoResponsed(ErrorCode errorCode, const AccountInfo& accountInfo,
        const FullUserInfo& userInfo);

public:
    AccountIds getOnlineUsers() const;

    ZoneUser* getUser(AccountId accountId);

    size_t getMaxUserCount() const {
        return maxUserCount_;
    }

    size_t getUserCount() const;
    size_t getLoginCount() const;

    bool isLoggedIn(AccountId accountId) const {
        return getUser_i(accountId) != nullptr;
    }

    bool hasNickname(AccountId accountId, const Nickname& nickname) const;

private:
    void loginAccepted(const AccountInfo& accountInfo, const FullUserInfo& userInfo);
    void loginRejected(AccountId accountId, ErrorCode errorCode);

    void certificateReissued(AccountId accountId, const Certificate& certificate);

private:
    ErrorCode addUser(const AccountInfo& accountInfo, const FullUserInfo& userInfo,
        const LoginRequest& loginRequest);
    ErrorCode removeUser(AccountId accountId);

private:
    ZoneUser* getUser_i(AccountId accountId) {
        return sne::core::search_map(userMap_, accountId, nullptr);
    }

    const ZoneUser* getUser_i(AccountId accountId) const {
        return sne::core::search_map(userMap_, accountId, nullptr);
    }

    bool isExists(AccountId accountId) const {
        return userMap_.find(accountId) != userMap_.end();
    }

private:
    const size_t maxUserCount_;

    std::unique_ptr<ZoneUserAllocator> zoneUserAllocator_;
    std::unique_ptr<ZoneUserPool> userPool_;
    ZoneUserMap userMap_; ///< 로그인한 사용자 목록

    LoginRequestMap loginRequestMap_;

    size_t loginCount_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {

#define ZONEUSER_MANAGER gideon::zoneserver::ZoneUserManager::instance()
