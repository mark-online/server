#pragma once

#include "communityserver_export.h"
#include <gideon/serverbase/service/SessionService.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/server/s2s/S2sCertificate.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace communityserver {

class CommunityLoginServerProxy;
class CommunityServerSideProxyManager;

class WorldMapChannelManager;
class CommunityPartyManager;
class CommunityGuildManager;
class CommunityDominionManager;
class CommunityBuddyManager;

class CommunityClientSessionCallback;
class CommunityUserManager;
class CommunityUserAllocator;

/**
 * @class CommunityService
 *
 * Community 서비스
 */
class CommunityServer_Export CommunityService :
    public serverbase::SessionService
{
    SNE_DECLARE_SINGLETON(CommunityService);

    using LockType = std::mutex;

public:
    CommunityService();
    virtual ~CommunityService();

    /// 서비스를 초기화한다.
    bool initialize(const std::string& serviceName);

    /// 서비스 준비 완료
    void ready();

    /// 서비스를 종료한다.
    void finalize();

public:
    /// 사용자를 로그인 처리한다
    ErrorCode login(const Certificate& certificate,
        CommunityClientSessionCallback& sessionCallback);

public:
    /// S2S 연결 인증
    ErrorCode authenticate(const sne::server::S2sCertificate& certificate);

public:
    CommunityLoginServerProxy& getLoginServerProxy() {
        assert(loginServerProxy_.get() != nullptr);
        return *loginServerProxy_;
    }

private:
    // = properties
    virtual const sne::server::S2sCertificate& getServerCertificate() const = 0;
    virtual size_t getMaxUserCount() const = 0;

    // = Factory Method
    virtual std::unique_ptr<CommunityUserAllocator> createCommunityUserAllocator() const = 0;
    virtual std::unique_ptr<CommunityLoginServerProxy> createLoginServerProxy() const = 0;

private:
    bool initServerSideProxyManager();
    bool initWorldMapChannelManager();
	bool initCommunityUserManager();
    bool initLoginServerProxy();
    bool initCommunityPartyManager();
    bool initCommunityGuildManager();
	bool initCommunityDominionManager();
	bool initCommunityBuddyManager();

private:
    GuildId getMaxGuildId() const;
    bool fillGuildInfos(GuildInfos& guildInfos) const;

private:
    std::string serviceName_;

    std::unique_ptr<CommunityLoginServerProxy> loginServerProxy_;
    std::unique_ptr<CommunityServerSideProxyManager> serverManager_;

    std::unique_ptr<WorldMapChannelManager> worldMapChannelManager_;

    std::unique_ptr<CommunityUserManager> userManager_;
    std::unique_ptr<CommunityPartyManager> communityPartyManager_;
    std::unique_ptr<CommunityGuildManager> communityGuildManager_;
	std::unique_ptr<CommunityDominionManager> communityCommunityDominionManager_;
	std::unique_ptr<CommunityBuddyManager> communityBuddyManager_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace communityserver {

#define COMMUNITY_SERVICE gideon::communityserver::CommunityService::instance()
