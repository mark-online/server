#include "LoginServerPCH.h"
#include "LoginService.h"
#include "s2s/LoginServerSideProxyManager.h"
#include "c2s/LoginClientSessionCallback.h"
#include "user/LoginUserManager.h"
#include "ban/BanManager.h"
#include "shard/ShardManager.h"
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/serverbase/database/AccountDatabase.h>
#include <sne/server/utility/Profiler.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace loginserver {

SNE_DEFINE_SINGLETON(LoginService);

LoginService::LoginService(const CommunityServerInfo& communityServerInfo) :
    communityServerInfo_(communityServerInfo)
{
    LoginService::instance(this);
}


LoginService::~LoginService()
{
    LoginService::instance(0);
}


bool LoginService::initialize()
{
    if (! serverbase::SessionService::loadProperties()) {
        return false;
    }

    if (! initServerSideProxyManager()) {
        SNE_LOG_ERROR("LoginService::initServerSideProxyManager() FAILED!");
        return false;
    }

    if (! initLoginUserManager()) {
        SNE_LOG_ERROR("LoginService::initLoginUserManager() FAILED!");
        return false;
    }

	if (! initShardManager()) {
		SNE_LOG_ERROR("LoginService::initShardManager() FAILED!");
		return false;
	}

	if (! initBanManager()) {
		SNE_LOG_ERROR("LoginService::initBanManager() FAILED!");
		return false;
	}

    return true;
}


void LoginService::finalize()
{
}


bool LoginService::initServerSideProxyManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    serverManager_= std::make_unique<LoginServerSideProxyManager>();
    LoginServerSideProxyManager::instance(serverManager_.get());
    return true;
}


bool LoginService::initLoginUserManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    const time_t migrationEffectivePeriod =
        SNE_PROPERTIES::getProperty<time_t>("migration.effective_period");

    loginUserManager_ = std::make_unique<LoginUserManager>(
        getMaxUserCount(), migrationEffectivePeriod);
    LoginUserManager::instance(loginUserManager_.get());
    return true;
}


bool LoginService::initShardManager()
{
    shardManager_= std::make_unique<ShardManager>();
    if (! shardManager_->initialize()) {
        return false;
    }
    ShardManager::instance(shardManager_.get());
    return true;
}


bool LoginService::initBanManager()
{
	banManager_= std::make_unique<BanManager>();
	banManager_->initialize();

	BanManager::instance(banManager_.get());
	return true;
}


ErrorCode LoginService::login(AccountInfo& accountInfo, CertificateMap& certificateMap,
    const UserId& userId, const UserPassword& userPassword,
    LoginClientSessionCallback* callback)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (isServiceBusy()) {
        return ecLoginIsBusy;
    }

    const ErrorCode errorCode =
        LOGINUSER_MANAGER->login(accountInfo, certificateMap, userId, userPassword, callback);
    if (errorCode == ecLoginAlreadyLoggedIn) {
        LOGINUSER_MANAGER->expelUser(accountInfo.accountId_, erDuplicatedLogin);
    }
    return errorCode;
}


ErrorCode LoginService::authenticate(Certificate& reinssuedCertificate,
    const Certificate& certificate)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (isServiceBusy()) {
        return ecLoginIsBusy;
    }

    return LOGINUSER_MANAGER->relogin(reinssuedCertificate, certificate);
}


void LoginService::clientDisconnected(AccountId accountId,
    LoginClientSessionCallback* callback)
{
    sne::server::Profiler profiler(__FUNCTION__);

    LOGINUSER_MANAGER->logout(accountId, stLoginServer, callback);
}


ErrorCode LoginService::authenticate(const sne::server::S2sCertificate& certificate)
{
    // 호출할 필요 없다
    //std::unique_lock<LockType> lock(lock_);

    if (certificate != getS2sCertificate()) {
        return ecServerAuthenticateFailed;
    }

    return ecOk;
}


CommunityServerInfo LoginService::getCommunityServerInfo() const
{
    std::unique_lock<LockType> lock(lock_);

    return communityServerInfo_;
}

}} // namespace gideon { namespace loginserver {
