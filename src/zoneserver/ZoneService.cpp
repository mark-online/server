#include "ZoneServerPCH.h"
#include "ZoneService.h"
#include "s2s/ZoneLoginServerProxy.h"
#include "s2s/ZoneCommunityServerProxy.h"
#include "s2s/arena/ArenaServerSideProxyManager.h"
#include "s2s/ZoneArenaServerProxy.h"
#include "world/World.h"
#include "world/WorldMap.h"
#include "world/region/WorldMapSpawnMapRegion.h"
#include "service/item/ItemIdGenerator.h"
#include "service/skill/SkillService.h"
#include "service/spawn/SpawnService.h"
#include "service/mail/MailService.h"
#include "service/auction/AuctionService.h"
#include "service/party/PartyService.h"
#include "service/guild/GuildService.h"
#include "service/duel/DuelService.h"
#include "service/auction/AuctionService.h"
#include "service/anchor/AnchorService.h"
#include "service/item/GraveStoneService.h"
#include "service/trade/TradeManager.h"
#include "service/inven/OrderInvenItemMoveService.h"
#include "service/teleport/TeleportService.h"
#include "service/arena/ArenaService.h"
#include "service/distance/DistanceChecker.h"
#include "service/world_event//WorldEventService.h"
#include "user/ZoneUserManager.h"
#include "user/ZoneUserAllocator.h"
#include "user/ZoneUser.h"
#include "model/gameobject/Harvest.h"
#include "model/gameobject/Treasure.h"
#include "model/gameobject/Device.h"
#include "model/quest/QuestRepository.h"
#include "ai/evt/EventTriggerManager.h"
#include <gideon/server/data/ServerAddress.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/cs/datatable/WorldMapTable.h>
#include <gideon/cs/datatable/PropertyTable.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

namespace {

inline FullShardInfo dbGetShardInfo(ShardId shardId)
{
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getShardInfo(shardId);
    if ((! future) || (! future->waitForDone())) {
        return FullShardInfo();
    }

    const serverbase::GetShardInfoRequestFuture& requestFuture =
        static_cast<const serverbase::GetShardInfoRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        return FullShardInfo();
    }

    return requestFuture.shardInfo_;
}


inline ZoneInfo dbGetZoneInfo(const std::string& serviceName)
{
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getZoneInfo(serviceName);
    if ((! future) || (! future->waitForDone())) {
        return ZoneInfo();
    }

    const serverbase::GetZoneInfoRequestFuture& requestFuture =
        static_cast<const serverbase::GetZoneInfoRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        return ZoneInfo();
    }

    return requestFuture.zoneInfo_;
}


inline ZoneInfo getZoneInfo(const std::string& serviceName)
{
    ZoneInfo zoneInfo = dbGetZoneInfo(serviceName);
    const gdt::map_t* globalWorldMapTemplate =
        WORLDMAP_TABLE->getMap(zoneInfo.globalMapInfo_.mapCode_);
    if (globalWorldMapTemplate != nullptr) {
        zoneInfo.globalMapInfo_.xSectorCount_ = globalWorldMapTemplate->x_sector_count();
        zoneInfo.globalMapInfo_.ySectorCount_ = globalWorldMapTemplate->y_sector_count();
        zoneInfo.globalMapInfo_.zSectorCount_ = globalWorldMapTemplate->z_sector_count();
        zoneInfo.globalMapInfo_.sectorLength_ = globalWorldMapTemplate->sector_length();
        zoneInfo.globalMapInfo_.aggressiveLevelDiff_ =
            globalWorldMapTemplate->aggressive_level_diff();
    }
    else {
        zoneInfo.reset();
    }

    return zoneInfo;
}

} // namespace

// = ZoneService

SNE_DEFINE_SINGLETON(ZoneService);

ZoneService::ZoneService()
{
    ZoneService::instance(this);
}


ZoneService::~ZoneService()
{
    finalize();

    ZoneService::instance(nullptr);
}


bool ZoneService::initialize(const std::string& serviceName)
{
    sne::server::Profiler profiler(__FUNCTION__);

    serviceName_ = serviceName;
    if (serviceName_ == getArenaServerName()) {
        serverType_ = zstArena;
    }
    else {
        serverType_ = zstNormal;
    }

    if (! serverbase::SessionService::loadProperties()) {
        return false;
    }

    if (serverType_ == zstArena) {
        if (! initServerSideProxyManager()) {
            SNE_LOG_ERROR("ZoneService::initServerSideProxyManager() FAILED!");
            return false;
        }
    }
    if (! initSpawnService()) {
        SNE_LOG_ERROR("ZoneService::initSpawnService() FAILED!");
        return false;
    }

    if (! initSkillService()) {
        SNE_LOG_ERROR("ZoneService::initSkillService() FAILED!");
        return false;
    }

    if (! initEventTriggerManager()) {
        SNE_LOG_ERROR("ZoneService::initEventTriggerManager() FAILED!");
        return false;
    }

    if (! createWorld()) {
        SNE_LOG_ERROR("ZoneService::createWorld() FAILED!");
        return false;
    }


    if (! initShardInfo()) {
        SNE_LOG_ERROR("ZoneService::initZoneInfos() FAILED!");
        return false;
    }

    if (! initZoneUserManager()) {
        SNE_LOG_ERROR("ZoneService::initZoneUserManager() FAILED!");
        return false;
    }

    if (! initLoginServerProxy()) {
        SNE_LOG_ERROR("ZoneService::initLoginServerProxy() FAILED!");
        return false;
    }

    if (! initCommunityServerProxy()) {
        SNE_LOG_ERROR("ZoneService::initCommunityServerProxy() FAILED!");
        return false;
    }

    if (! initWorld()) {
        SNE_LOG_ERROR("ZoneService::initWorld() FAILED!");
        return false;
    }

    initTradeManager();
    initOrderInvenItemMoveService();
    if (! initDistanceChecker()) {
        SNE_LOG_ERROR("ZoneService::initDistanceChecker() FAILED!");
        return false;
    }
    
    if (! initGraveStoneService()) {
        SNE_LOG_ERROR("ZoneService::initGraveStoneService() FAILED!");
        return false;
    }

    if (! initPartyService()) {
        SNE_LOG_ERROR("ZoneService::initPartyService() FAILED!");
        return false;
    }

    if (! initDuelService()) {
        SNE_LOG_ERROR("ZoneService::initDuelService() FAILED!");
        return false;
    }

    if (! initGuildService()) {
        SNE_LOG_ERROR("ZoneService::initGuildService() FAILED!");
        return false;
    }

    if (serverType_ == zstNormal) {
        if (! initArenaServerProxy()) {
            SNE_LOG_ERROR("ZoneService::initArenaServerProxy() FAILED!");
            return false;
        }
    }

    if (! initAnchorService()) {
        SNE_LOG_ERROR("ZoneService::initAnchorService() FAILED!");
        return false;
    }

    if (! initMailService()) {
        SNE_LOG_ERROR("ZoneService::initMailService() FAILED!");
        return false;
    }

    if (! initAuctionService()) {
        SNE_LOG_ERROR("ZoneService::initAuctionService() FAILED!");
        return false;
    }

    if (! initWorldEventService()) {
        SNE_LOG_ERROR("ZoneService::initWorldEventService() FAILED!");
        return false;
    }

    if (! initItemIdGenerator()) {
        SNE_LOG_ERROR("ZoneService::initItemIdGenerator() FAILED!");
        return false;
    }

    if (serverType_ == zstArena) {
        if (! initArenaService()) {
            SNE_LOG_ERROR("ZoneService::initArenaService() FAILED!");
            return false;
        }
    }

    return true;
}


void ZoneService::finalize()
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (world_.get() != nullptr) {
        world_->finalize();
    }

    spawnService_.reset();
}


void ZoneService::ready()
{
    loginServerProxy_->ready();
    communityServerProxy_->ready();
}


ErrorCode ZoneService::login(const Certificate& certificate, ObjectId arenaId, 
    RegionCode spawnRegionCode, MapCode preGlobalMapCode, 
    const Position& position, ZoneClientSessionCallback& sessionCallback)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (isServiceBusy()) {
        return ecZoneIsBusy;
    }

    return ZONEUSER_MANAGER->login(certificate, arenaId, spawnRegionCode, preGlobalMapCode,
        position, sessionCallback);
}


ErrorCode ZoneService::reserveMigration(MigrationTicket& ticket, AccountId accountId,
    MapCode targetMapCode, RegionCode targetRegionCode, ObjectId arenaId,
    const Position& position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    ZoneUser* user = ZONEUSER_MANAGER->getUser(accountId);
    if (! user) {
        return ecTeleportNotAvailable;
    }

    
    const ZoneServerInfo* serverInfo = getZoneServerInfoByMapCode(targetMapCode);
    if (! serverInfo) {
        return ecTeleportNotAvailable;
    }
    if (! serverInfo->isEnabled_) {
        return ecZoneServerIsDown;
    }

    user->reserveMigration();

    ticket.zoneServerInfo_ = *serverInfo;
    ticket.spawnRegionCode_ = targetRegionCode;
    ticket.arenaId_ = arenaId;
    ticket.position_ = position;

    loginServerProxy_->z2l_reserveMigration(accountId);

    return ecOk;
}


bool ZoneService::initServerSideProxyManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    serverManager_ = std::make_unique<ArenaServerSideProxyManager>();
    ArenaServerSideProxyManager::instance(serverManager_.get());
    return true;
}


bool ZoneService::createWorld()
{
    sne::server::Profiler profiler(__FUNCTION__);

    const ZoneInfo zoneInfo = getZoneInfo(serviceName_);
    if (! zoneInfo.isValid()) {
        SNE_LOG_ERROR("Can't find zone info(%s)", serviceName_.c_str());
        return false;
    }

    WorldCallback& worldCallback = *this;
    world_= std::make_unique<World>(zoneInfo, getWorldMapHelper(), worldCallback);

    return true;
}


bool ZoneService::initWorld()
{
    sne::server::Profiler profiler(__FUNCTION__);
    if (world_.get() == nullptr) {
        return false;
    }

    if (! world_->initialize()) {
        return false;
    }

    // 커뮤니티 서버가 먼저 있어야 한다
    // TODO: 커뮤니티 서버에 연결되면 정보를 보내도록 개선
    assert(communityServerProxy_.get());
    notifyOpenedWorldMaps();

    return true;
}


void ZoneService::initTradeManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    tradeManager_= std::make_unique<TradeManager>();
    TradeManager::instance(tradeManager_.get());
}


void ZoneService::initOrderInvenItemMoveService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    orderInvenItemMoveService_= std::make_unique<OrderInvenItemMoveService>();
    OrderInvenItemMoveService::instance(orderInvenItemMoveService_.get());
}


bool ZoneService::initDistanceChecker()
{
    distanceChecker_= std::make_unique<DistanceChecker>();
    return distanceChecker_->initialize();
}


bool ZoneService::initSkillService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    skillService_= std::make_unique<SkillService>();
    if (! skillService_->initialize()) {
        return false;
    }
    return true;
}


bool ZoneService::initShardInfo()
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! WORLD) {
        return false;
    }

    shardInfo_ = dbGetShardInfo(WORLD->getShardId());
    return shardInfo_.isValid();
}


bool ZoneService::initSpawnService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    spawnService_= std::make_unique<SpawnService>(getNpcAllocator(),
        getHarvestAllocator(), getObeliskAllocator(), getTreasureAllocator(),
        getRandomDungeonAllocator(), getDeviceAllocator(), getAnchorAllocator());
    if (! spawnService_->initialize()) {
        return false;
    }
    return true;
}


bool ZoneService::initGraveStoneService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    dropService_= std::make_unique<GraveStoneService>();
    if (! dropService_->initialize(timerResultSkip())) {
        return false;
    }
    return true;
}


bool ZoneService::initTeleportService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    teleportService_= std::make_unique<TeleportService>();
    return true;
}


bool ZoneService::initPartyService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    partyService_= std::make_unique<PartyService>();
    partyService_->initialize();
    return true;
}


bool ZoneService::initAnchorService()
{
    sne::server::Profiler profiler(__FUNCTION__);
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    sne::base::Future::Ref future = db->loadBuildingInfos(WORLD->getZoneId());
    if ((! future) || (! future->waitForDone())) {
        return false;
    }

    const serverbase::LoadBuildingRequestFuture& requestFuture =
        static_cast<const serverbase::LoadBuildingRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        return false;
    }

    anchorService_= std::make_unique<AnchorService>();
    anchorService_->initialize(requestFuture.maxBuildingId_, requestFuture.buildingInfoMap_);

    return static_cast<WorldMapSpawnMapRegion&>(world_->getGlobalWorldMap().getGlobalMapRegion()).spawnBuildings();
}


bool ZoneService::initGuildService()
{
    sne::server::Profiler profiler(__FUNCTION__);
    GuildInfos infos;
    fillGuildInfos(infos);
    guildService_= std::make_unique<GuildService>();
    
    return guildService_->initialize(infos);
}


bool ZoneService::initMailService()
{
    sne::server::Profiler profiler(__FUNCTION__);
    mailService_= std::make_unique<MailService>();

    return mailService_->initialize();
}


bool ZoneService::initAuctionService()
{
    sne::server::Profiler profiler(__FUNCTION__);
    ServerAuctionInfoMap infos;
    fillAuctionInfos(infos);

    auctionService_= std::make_unique<AuctionService>(WORLD->getZoneId());

    return auctionService_->initialize(infos);
}


bool ZoneService::initArenaService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    arenaService_= std::make_unique<ArenaService>(getWorldMapHelper());

    return arenaService_->initialize();
}


bool ZoneService::initDuelService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    duelService_= std::make_unique<DuelService>();

    return duelService_->initialize();
}


bool ZoneService::initWorldEventService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    worldEventService_= std::make_unique<WorldEventService>();

    return worldEventService_->initialize(WORLD->getGlobalWorldMap(), getWorldMapHelper());
}


bool ZoneService::initItemIdGenerator()
{
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getMaxInventoryId(WORLD->getZoneId());
    if ((! future) || (! future->waitForDone())) {
        return false;
    }

    const serverbase::GetMaxItemInventoryIdRequestFuture& requestFuture =
        static_cast<const serverbase::GetMaxItemInventoryIdRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        return false;
    }

    itemIdGenerator_= std::make_unique<ItemIdGenerator>(requestFuture.maxInventoryId_);
    ItemIdGenerator::instance(itemIdGenerator_.get());
    return true;
}


bool ZoneService::initZoneUserManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    userManager_ = std::make_unique<ZoneUserManager>(
        createZoneUserAllocator(), getMaxUserCount());
    ZoneUserManager::instance(userManager_.get());
    return true;
}


bool ZoneService::initLoginServerProxy()
{
    sne::server::Profiler profiler(__FUNCTION__);

    loginServerProxy_.reset(createLoginServerProxy().release());
    if (! loginServerProxy_.get()) {
        return false;
    }

    return true;
}


bool ZoneService::initCommunityServerProxy()
{
    sne::server::Profiler profiler(__FUNCTION__);

    communityServerProxy_.reset(createCommunityServerProxy().release());
    if (! communityServerProxy_.get()) {
        return false;
    }

    return true;
}


bool ZoneService::initArenaServerProxy()
{
    if (serverType_ == zstNormal) {
        arenaServerProxy_.reset(createArenaServerProxy().release());
        if (! arenaServerProxy_.get()) {
            return false;
        }
    }
    return true;
}


bool ZoneService::initEventTriggerManager()
{
    assert(! world_.get());

    /// FYI: 모든 테이블을 로딩한 후 호출해야 한다!
    sne::server::Profiler profiler("ZoneService::initEventTriggerManager");

    const ZoneInfo zoneInfo = getZoneInfo(serviceName_);
    if (! zoneInfo.isValid()) {
        SNE_LOG_ERROR("Can't find zone info(%s)", serviceName_.c_str());
        return false;
    }

    return EVT_MANAGER.initialize(zoneInfo.globalMapInfo_.mapCode_);
}


ErrorCode ZoneService::authenticate(const sne::server::S2sCertificate& certificate)
{
    // 호출할 필요 없다
    //std::lock_guard<LockType> lock(lock_);

    if (certificate != getServerCertificate()) {
        return ecServerAuthenticateFailed;
    }

    return ecOk;
}


void ZoneService::communityServerConnected()
{
    {
        std::lock_guard<LockType> lock(lock_);

        if (! communityServerProxy_.get()) {
            return;
        }
    }

    notifyOpenedWorldMaps();
}


void ZoneService::zoneServerActivated(ZoneId zoneId)
{
    std::lock_guard<LockType> lock(lock_);

    ZoneServerInfo* serverInfo = getZoneServerInfo(zoneId);
    if (! serverInfo) {
        return;
    }

    serverInfo->isEnabled_ = true;
}


void ZoneService::zoneServerDeactivated(ZoneId zoneId)
{
    std::lock_guard<LockType> lock(lock_);

    ZoneServerInfo* serverInfo = getZoneServerInfo(zoneId);
    if (! serverInfo) {
        return;
    }

    serverInfo->isEnabled_ = false;
}


const ZoneServerInfo* ZoneService::getZoneServerInfoByMapCode(MapCode mapCode) const
{
    std::lock_guard<LockType> lock(lock_);

    for (const ZoneServerInfoMap::value_type& value : shardInfo_.zoneServerInfoMap_) {
        const ZoneServerInfo& zoneServerInfo = value.second;
        if (zoneServerInfo.mapCode_ == mapCode) {
            return &zoneServerInfo;
        }
    }

    return nullptr;
}


ZoneServerInfo* ZoneService::getZoneServerInfo(ZoneId zoneId)
{
    std::lock_guard<LockType> lock(lock_);

    const ZoneServerInfoMap::iterator pos = shardInfo_.zoneServerInfoMap_.find(zoneId);
    if (pos != shardInfo_.zoneServerInfoMap_.end()) {
        return &(*pos).second;
    }

    return nullptr;
}


void ZoneService::notifyOpenedWorldMaps()
{
    if ((! communityServerProxy_.get()) || (! communityServerProxy_->isActivated())) {
        return;
    }

    WORLD->notifyOpenedWorldMaps();
}


bool ZoneService::fillGuildInfos(GuildInfos& guildInfos) const
{
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getGuildInfos();
    if ((! future) || (! future->waitForDone())) {
        return false;
    }

    const serverbase::GetGuildInfosRequestFuture& requestFuture =
        static_cast<const serverbase::GetGuildInfosRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {        
        return false;
    }

    guildInfos = requestFuture.guildInfos_;

    return true;
}


bool ZoneService::fillAuctionInfos(ServerAuctionInfoMap& auctionMap) const
{
    sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getAuctionInfos(WORLD->getZoneId());
    if ((! future) || (! future->waitForDone())) {
        return false;
    }

    const serverbase::GetAuctionInfosRequestFuture& requestFuture =
        static_cast<const serverbase::GetAuctionInfosRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {        
        return false;
    }

    auctionMap = requestFuture.auctionMap_;

    return true;
}

// = WorldCallback overriding

void ZoneService::worldMapOpened(MapCode worldMapCode, ObjectId mapId)
{
    if (! hasChatChannel(getMapType(worldMapCode))) {
        return;
    }

    if ((! communityServerProxy_.get()) || (! communityServerProxy_->isActivated())) {
        return;
    }

    communityServerProxy_->z2m_worldMapOpened(worldMapCode, mapId);
}


void ZoneService::worldMapClosed(MapCode worldMapCode, ObjectId mapId)
{
    if (! hasChatChannel(getMapType(worldMapCode))) {
        return;
    }

    if ((! communityServerProxy_.get()) || (! communityServerProxy_->isActivated())) {
        return;
    }

    communityServerProxy_->z2m_worldMapClosed(worldMapCode, mapId);
}

}} // namespace gideon { namespace zoneserver {
