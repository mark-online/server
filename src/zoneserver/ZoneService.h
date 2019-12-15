#pragma once

#include "zoneserver_export.h"
#include "world/WorldCallback.h"
#include <gideon/serverbase/service/SessionService.h>
#include <gideon/server/data/ShardInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/AuctionInfo.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/server/s2s/S2sCertificate.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

namespace go {
class EntityAllocator;
} // namespace go {

class ZoneClientSessionCallback;

class World;
class WorldMapHelper;

class ZoneUserManager;
class ZoneUserAllocator;

class ArenaServerSideProxyManager;
class ZoneLoginServerProxy;
class ZoneCommunityServerProxy;
class ZoneArenaServerProxy;

class TradeManager;
class OrderInvenItemMoveService;
class SkillService;
class AnchorService;
class SpawnService;
class GraveStoneService;
class TeleportService;
class PartyService;
class GuildService;
class MailService;
class AuctionService;
class ArenaService;
class ItemIdGenerator;
class DistanceChecker;
class DuelService;
class WorldEventService;

/**
 * @class ZoneService
 *
 * Zone 서비스
 */
class ZoneServer_Export ZoneService :
    public serverbase::SessionService,
    private WorldCallback
{
    SNE_DECLARE_SINGLETON(ZoneService);

protected:
    typedef std::mutex LockType;

public:
    ZoneService();
    virtual ~ZoneService();

    /// 서비스를 초기화한다.
    bool initialize(const std::string& serviceName);

    /// 서비스 준비 완료
    void ready();

    /// 서비스를 종료한다.
    void finalize();

public:
    /// 사용자를 로그인 처리한다
    ErrorCode login(const Certificate& certificate, ObjectId arenaId, 
		RegionCode spawnRegionCode, MapCode preGlobalMapCode, 
        const Position& position, ZoneClientSessionCallback& sessionCallback);

    /// 플레이어의 존 서버 이동 예약. 인증서를 발급 받고 로그인 서버에 알린다
    ErrorCode reserveMigration(MigrationTicket& ticket, AccountId accountId,
        MapCode targetMapCode, RegionCode targetRegionCode, ObjectId arenaId,
        const Position& position);

public:
    /// S2S 연결 인증
    ErrorCode authenticate(const sne::server::S2sCertificate& certificate);

public:
    void communityServerConnected();

    void zoneServerActivated(ZoneId zoneId);
    void zoneServerDeactivated(ZoneId zoneId);

public:
    const ZoneServerInfo* getZoneServerInfoByMapCode(MapCode mapCode) const;
    ZoneServerInfo* getZoneServerInfo(ZoneId zoneId);

    ZoneLoginServerProxy& getLoginServerProxy() {
        assert(loginServerProxy_.get() != nullptr);
        return *loginServerProxy_;
    }

    ZoneCommunityServerProxy& getCommunityServerProxy() {
        assert(communityServerProxy_.get() != nullptr);
        return *communityServerProxy_;
    }

	ZoneArenaServerProxy& getArenaServerProxy() {
		assert(arenaServerProxy_.get() != nullptr);
		return *arenaServerProxy_;
	}

    bool isArenaServer() const {
        return serverType_ == zstArena;
    }

protected:
    LockType& getLock() const {
        return lock_;
    }

private:
    // = properties
    virtual const sne::server::S2sCertificate& getServerCertificate() const = 0;
    virtual size_t getMaxUserCount() const = 0;

    // = Factory Method
    virtual std::unique_ptr<ZoneUserAllocator> createZoneUserAllocator() = 0;
    virtual std::unique_ptr<ZoneLoginServerProxy> createLoginServerProxy() const = 0;
    virtual std::unique_ptr<ZoneCommunityServerProxy> createCommunityServerProxy() const = 0;
	virtual std::unique_ptr<ZoneArenaServerProxy> createArenaServerProxy() const = 0;

    virtual go::EntityAllocator& getPlayerAllocator() = 0;
    virtual go::EntityAllocator& getNpcAllocator() = 0;
    virtual go::EntityAllocator& getHarvestAllocator() = 0;
    virtual go::EntityAllocator& getTreasureAllocator() = 0;
    virtual go::EntityAllocator& getRandomDungeonAllocator() = 0;
    virtual go::EntityAllocator& getObeliskAllocator() = 0;
    virtual go::EntityAllocator& getDeviceAllocator() = 0;
    virtual go::EntityAllocator& getAnchorAllocator() = 0;

    // = helper
    virtual WorldMapHelper& getWorldMapHelper() = 0;

private:
    // = WorldCallback overriding
    virtual void worldMapOpened(MapCode worldMapCode, ObjectId mapId);
    virtual void worldMapClosed(MapCode worldMapCode, ObjectId mapId);

private:
	bool initServerSideProxyManager();
    bool initShardInfo();
    bool initWorld();
    bool createWorld();
    void initTradeManager();
    void initOrderInvenItemMoveService();
    bool initDistanceChecker();
    bool initZoneUserManager();
    bool initLoginServerProxy();
    bool initCommunityServerProxy();
	bool initArenaServerProxy();
    bool initSkillService();
    bool initSpawnService();
    bool initGraveStoneService();
    bool initTeleportService();
    bool initPartyService();
    bool initAnchorService();
    bool initGuildService();
    bool initMailService();
    bool initAuctionService();
	bool initArenaService();
    bool initDuelService();
    bool initWorldEventService();
    bool initItemIdGenerator();
    bool initEventTriggerManager();
    
private:
    void notifyOpenedWorldMaps();

    bool fillGuildInfos(GuildInfos& guildInfos) const;
    bool fillAuctionInfos(ServerAuctionInfoMap& auctionMap) const;


private:
    // = for test
    virtual bool timerResultSkip() const {
        return false;
    }

private:
    std::string serviceName_;
	ZoneServerType serverType_;

    std::unique_ptr<ArenaServerSideProxyManager> serverManager_;

    std::unique_ptr<World> world_;
    std::unique_ptr<TradeManager> tradeManager_;
    std::unique_ptr<OrderInvenItemMoveService> orderInvenItemMoveService_;
    std::unique_ptr<ItemIdGenerator> itemIdGenerator_;

    std::unique_ptr<ZoneUserManager> userManager_;
    std::unique_ptr<ZoneLoginServerProxy> loginServerProxy_;
    std::unique_ptr<ZoneCommunityServerProxy> communityServerProxy_;
	std::unique_ptr<ZoneArenaServerProxy> arenaServerProxy_;

    std::unique_ptr<SkillService> skillService_;
    std::unique_ptr<SpawnService> spawnService_;
    std::unique_ptr<GraveStoneService> dropService_;
    std::unique_ptr<TeleportService> teleportService_;
    std::unique_ptr<PartyService> partyService_;
    std::unique_ptr<AnchorService> anchorService_;
    std::unique_ptr<GuildService> guildService_;
    std::unique_ptr<MailService> mailService_;
    std::unique_ptr<AuctionService> auctionService_;
	std::unique_ptr<ArenaService> arenaService_;
    std::unique_ptr<DuelService> duelService_;
    std::unique_ptr<WorldEventService> worldEventService_;

	std::unique_ptr<DistanceChecker> distanceChecker_;

    FullShardInfo shardInfo_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {

#define ZONE_SERVICE gideon::zoneserver::ZoneService::instance()
