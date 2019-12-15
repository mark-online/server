#pragma once

#include "../loginserver_export.h"
#include "LoginUser.h"
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/container/Containers.h>
#include <sne/core/utility/Singleton.h>
#include <sne/base/concurrent/Runnable.h>
#include <sne/base/concurrent/Future.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace loginserver {

class LoginClientSessionCallback;

/**
 * @class LoginUserManager
 *
 * 로그인 서버 사용자 관리자
 * - 중복 로그인 방지
 */
class LoginServer_Export LoginUserManager : private sne::base::Runnable
{
    typedef sne::core::HashMap<AccountId, LoginUser> OnlineUsers;

    typedef sne::core::Set<AccountId> AccountIdSet;

    typedef sne::core::Set<Nickname> NicknameSet;

    typedef sne::core::HashMap<AccountId, Nickname> NicknameMap;


    typedef std::mutex LockType;

    SNE_DECLARE_SINGLETON(LoginUserManager);
public:
    LoginUserManager(size_t maxUserCount, time_t migrationEffectivePeriod);
    ~LoginUserManager();

    /// 사용자 로그인
    ErrorCode login(AccountInfo& accountInfo, CertificateMap& certificateMap,
        const UserId& userId, const UserPassword& userPassword,
        LoginClientSessionCallback* callback);

    /// 사용자 인증
    ErrorCode relogin(Certificate& reinssuedCertificate, const Certificate& certificate);

    /// 예약된 닉네임
    ErrorCode reserveNickname(AccountId accountId, const Nickname& nickname);

    void deleteReservedNickname(AccountId accountId);

    /// 존서버에서 인증을 시도한다
    ErrorCode loginZoneUser(AccountInfo& accountInfo, Certificate& reinssuedCertificate,
        sne::server::ServerId zoneServerId, ShardId shardId, const Certificate& certificate);

    /// 커뮤니티 서버에서 인증을 시도한다
    ErrorCode loginCommunityUser(AccountInfo& accountInfo, Certificate& reinssuedCertificate,
        sne::server::ServerId communityServerId, const Certificate& certificate);

    /// 로그아웃 하였다
    bool logout(AccountId accountId, ServerType serverType,
        LoginClientSessionCallback* callback = nullptr);

    /// Shard가 변경되었다
    void shardChanged(const FullUserInfo& userInfo, ShardId shardId);

    /// 접속을 해제한다
    void expelUser(AccountId accountId, ExpelReason reason);

public:
    /// 존 서버 이동을 예약하였다
    void reserveMigration(AccountId accountId);

public:
    /// 존 서버가 접속을 하였다. 접속자가 있을 경우 로그인 처리한다.
    void zoneServerConnected(sne::server::ServerId zoneServerId, ShardId shardId,
        const AccountIds& onlineUsers);

    /// 존 서버와의 연결이 끊겼다.
    void zoneServerDisconnected(sne::server::ServerId zoneServerId);

public:
    /// 커뮤니티 서버가 접속을 하였다. 접속자가 있을 경우 로그인 처리한다.
    void communityServerConnected(sne::server::ServerId communityServerId,
        const AccountIds& onlineUsers);

    /// 커뮤니티 서버와의 연결이 끊겼다.
    void communityServerDisconnected();

public:
    void characterCreated(ShardId shardId, AccountId accountId,
        const FullCharacterInfo& characterInfo);
    void characterDeleted(ShardId shardId, AccountId accountId, ObjectId characterId);

public:
    /// 동접 수
    size_t getUserCount() const;

    /// 로그인 횟수
    size_t getLoginCount() const;

    bool isOnline(AccountId accountId) const;

public:
    CharacterCountPerShardMap getCharacterCounts(AccountId accountId) const;

    bool queryZoneServerInfo(ShardId& shardId, ZoneId& zoneId,
        AccountId accountId, ObjectId characterId);

    ShardId getSelectedShardId(AccountId accountId) const;

private:
    ErrorCode loginFromZoneServer(AccountId accountId, sne::server::ServerId zoneServerId,
        ShardId shardId);
    ErrorCode loginFromCommunityServer(AccountId accountId,
        sne::server::ServerId communityServerId);
    void logout(AccountId accountId, LoginUser& loginUser);

    /// @pre ! isLoggedIn(accountId)
    ErrorCode registerAuthenticatedUser(AccountId accountId,
        LoginClientSessionCallback* callback = nullptr);

    void addLoginUser(const AccountInfo& accountInfo,
        const CharacterCountPerShardMap& characterCounts,
        LoginClientSessionCallback* callback);
    void removeLoginUser(AccountId accountId);

    void checkExpiredMigration();

    void deleteReservedNickname_i(AccountId accountId);

private:
    LoginUser* getLoginUser(AccountId accountId) {
        const OnlineUsers::iterator pos = onlineUsers_.find(accountId);
        if (pos != onlineUsers_.end()) {
            return &(*pos).second;
        }
        return 0;
    }

    const LoginUser* getLoginUser(AccountId accountId) const {
        const OnlineUsers::const_iterator pos = onlineUsers_.find(accountId);
        if (pos !=  onlineUsers_.end()) {
            return &(*pos).second;
        }
        return 0;
    }

    bool isLoggedIn(AccountId accountId) const {
        return getLoginUser(accountId) != nullptr;
    }

    bool hasTooManyUsers() const {
        return onlineUsers_.size() >= maxUserCount_;
    }

private:
    // = sne::base::Runnable overriding
    virtual void run();

private:
    const size_t maxUserCount_;
    const time_t migrationEffectivePeriod_;

    OnlineUsers onlineUsers_;
    AccountIdSet migratingUsers_;

    NicknameSet reserveNicknames_;
    NicknameMap accountReserveNicknames_;

    size_t loginCount_;

    sne::base::Future::WeakRef migrationCheckTask_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace loginserver {

#define LOGINUSER_MANAGER \
    gideon::loginserver::LoginUserManager::instance()
