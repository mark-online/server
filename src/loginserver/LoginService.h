#pragma once

#include "loginserver_export.h"
#include <gideon/serverbase/service/SessionService.h>
#include <gideon/server/data/ServerAddress.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include "gideon/cs/shared/data/ExpelReason.h"
#include <gideon/cs/shared/data/ServerType.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <sne/server/s2s/S2sCertificate.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/server/common/SocketAddress.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { 
struct Certificate;
} // namespace gideon { 

namespace gideon { namespace loginserver {

class LoginClientSessionCallback;
class LoginServerSideProxyManager;
class LoginUserManager;
class ShardManager;
class BanManager;

/**
 * @class LoginService
 *
 * 로그인 서비스
 */
class LoginServer_Export LoginService :
    public serverbase::SessionService
{
    SNE_DECLARE_SINGLETON(LoginService);

    typedef std::mutex LockType;

public:
    LoginService(const CommunityServerInfo& communityServerInfo);
    virtual ~LoginService();

    /// 서비스를 초기화한다.
    bool initialize();

    /// 서비스를 종료한다.
    void finalize();

public:
    /// 사용자 로그인
    ErrorCode login(AccountInfo& accountInfo, CertificateMap& certificateMap,
        const UserId& userId, const UserPassword& userPassword,
        LoginClientSessionCallback* callback = nullptr);

    /// 사용자 인증(로그인 서버)
    ErrorCode authenticate(Certificate& reinssuedCertificate, const Certificate& certificate);

public:
    /// 클라이언트가 접속을 해제하였다
    void clientDisconnected(AccountId accountId, LoginClientSessionCallback* callback = nullptr);

public:
    /// 서버를 인증한다
    ErrorCode authenticate(const sne::server::S2sCertificate& certificate);

public:
    CommunityServerInfo getCommunityServerInfo() const;

private:
    virtual const sne::server::S2sCertificate& getS2sCertificate() const = 0;

    virtual size_t getMaxUserCount() const = 0;	

private:
    bool initServerSideProxyManager();
    bool initLoginUserManager();
	bool initShardManager();
	bool initBanManager();

private:
    std::unique_ptr<LoginServerSideProxyManager> serverManager_;
    std::unique_ptr<LoginUserManager> loginUserManager_;
    std::unique_ptr<ShardManager> shardManager_;
	std::unique_ptr<BanManager> banManager_;
    CommunityServerInfo communityServerInfo_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace loginserver {

#define LOGIN_SERVICE gideon::loginserver::LoginService::instance()
