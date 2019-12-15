#include "CommunityServerPCH.h"
#include "CommunityService.h"
#include "s2s/CommunityLoginServerProxy.h"
#include "s2s/CommunityServerSideProxyManager.h"
#include "channel/WorldMapChannelManager.h"
#include "party/CommunityPartyManager.h"
#include "guild/CommunityGuildManager.h"
#include "buddy/CommunityBuddyManager.h"
#include "dominion/CommunityDominionManager.h"
#include "user/CommunityUserManager.h"
#include "user/CommunityUserAllocator.h"
#include "user/CommunityUser.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/serverbase/datatable/DataTableLoader.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace communityserver {

// = CommunityService

SNE_DEFINE_SINGLETON(CommunityService);

CommunityService::CommunityService()
{
    CommunityService::instance(this);
}


CommunityService::~CommunityService()
{
    finalize();

    CommunityService::instance(nullptr);
}


bool CommunityService::initialize(const std::string& serviceName)
{
    sne::server::Profiler profiler(__FUNCTION__);

    serviceName_ = serviceName;

    if (! serverbase::SessionService::loadProperties()) {
        return false;
    }

    if (! initServerSideProxyManager()) {
        SNE_LOG_ERROR("LoginService::initServerSideProxyManager() FAILED!");
        return false;
    }

    if (! initWorldMapChannelManager()) {
        SNE_LOG_ERROR("CommunityService::initWorldMapChannelManager() FAILED!");
        return false;
    }

    if (! initCommunityUserManager()) {
        SNE_LOG_ERROR("CommunityService::initCommunityUserManager() FAILED!");
        return false;
    }

    if (! initLoginServerProxy()) {
        SNE_LOG_ERROR("CommunityService::initLoginServerProxy() FAILED!");
        return false;
    }

    if (! initCommunityPartyManager()) {
        SNE_LOG_ERROR("CommunityService::initCommunityPartyManager() FAILED!");
        return false;
    }


    if (! initCommunityGuildManager()) {
        SNE_LOG_ERROR("CommunityService::initCommunityGuildManager() FAILED!");
        return false;
    }

	if (! initCommunityDominionManager()) {
		SNE_LOG_ERROR("CommunityService::initCommunityDominionManager() FAILED!");
		return false;
	}

	if (! initCommunityBuddyManager()) {
		SNE_LOG_ERROR("CommunityService::initCommunityBuddyManager() FAILED!");
		return false;
	}



    return true;
}


void CommunityService::finalize()
{
    //sne::server::Profiler profiler(__FUNCTION__);
}


bool CommunityService::initServerSideProxyManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    serverManager_= std::make_unique<CommunityServerSideProxyManager>();
    CommunityServerSideProxyManager::instance(serverManager_.get());
    return true;
}


void CommunityService::ready()
{
    loginServerProxy_->ready();
}


ErrorCode CommunityService::login(const Certificate& certificate,
    CommunityClientSessionCallback& sessionCallback)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (isServiceBusy()) {
        return ecCommunityIsBusy;
    }

    return COMMUNITYUSER_MANAGER->login(certificate, sessionCallback);
}


bool CommunityService::initWorldMapChannelManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    worldMapChannelManager_= std::make_unique<WorldMapChannelManager>();
    worldMapChannelManager_->initialize();
    WorldMapChannelManager::instance(worldMapChannelManager_.get());
    return true;
}


bool CommunityService::initCommunityUserManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    userManager_ = std::make_unique<CommunityUserManager>(
        createCommunityUserAllocator(), getMaxUserCount());
    CommunityUserManager::instance(userManager_.get());
    return true;
}


bool CommunityService::initLoginServerProxy()
{
    sne::server::Profiler profiler(__FUNCTION__);

    loginServerProxy_.reset(createLoginServerProxy().release());
    if (! loginServerProxy_.get()) {
        return false;
    }

    return true;
}


bool CommunityService::initCommunityPartyManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    communityPartyManager_= std::make_unique<CommunityPartyManager>();
    if (! communityPartyManager_.get()) {
        return false;
    }
    communityPartyManager_->initialize();
    CommunityPartyManager::instance(communityPartyManager_.get());
    return true;
}


bool CommunityService::initCommunityGuildManager()
{
    sne::server::Profiler profiler(__FUNCTION__);
    
    GuildId guildId = getMaxGuildId();
    if (! isValidGuildId(guildId)) {
        return false;
    }

    GuildInfos guildInfos;
    if (! fillGuildInfos(guildInfos)) {
        return false;
    }

    communityGuildManager_= std::make_unique<CommunityGuildManager>();   
    return communityGuildManager_->initialize(guildInfos, guildId);
}


bool CommunityService::initCommunityDominionManager()
{
	communityCommunityDominionManager_= std::make_unique<CommunityDominionManager>();  
	CommunityDominionManager::instance(communityCommunityDominionManager_.get());
	return true;
}


GuildId CommunityService::getMaxGuildId() const
{
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getMaxGuildId();
    if ((! future) || (! future->waitForDone())) {
        return invalidGuildId;
    }

    const auto& requestFuture =
        static_cast<const serverbase::GetMaxGuildIdRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        return invalidGuildId;
    }

    return requestFuture.maxGuildId_ + 1;
}


bool CommunityService::fillGuildInfos(GuildInfos& guildInfos) const
{
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getGuildInfos();
    if ((! future) || (! future->waitForDone())) {
        SNE_LOG_ERROR("fillGuildInfos - 1");
        return false;
    }

    const auto& requestFuture =
        static_cast<const serverbase::GetGuildInfosRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {        
        SNE_LOG_ERROR("fillGuildInfos - 2(%d)", requestFuture.errorCode_);
        return false;
    }

    guildInfos = requestFuture.guildInfos_;

    return true;
}


ErrorCode CommunityService::authenticate(const sne::server::S2sCertificate& certificate)
{
    // 호출할 필요 없다
    //std::lock_guard<LockType> lock(lock_);

    if (certificate != getServerCertificate()) {
        return ecServerAuthenticateFailed;
    }

    return ecOk;
}


bool CommunityService::initCommunityBuddyManager()
{
	communityBuddyManager_= std::make_unique<CommunityBuddyManager>();  
	return communityBuddyManager_->initialize();
}

}} // namespace gideon { namespace communityserver {
