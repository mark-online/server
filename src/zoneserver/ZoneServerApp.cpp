#include "ZoneServerPCH.h"
#include "ZoneServerApp.h"
#include "ZoneService.h"
#include "world/WorldMapHelper.h"
#include "service/time/GameTimer.h"
#include "s2s/ZoneLoginServerProxy.h"
#include "s2s/ZoneCommunityServerProxy.h"
#include "s2s/ZoneArenaServerProxy.h"
#include "s2s/arena/ArenaServerSideProxyFactory.h"
#include "s2s/arena/ArenaServerSideProxyManager.h"
#include "c2s/ZoneClientSessionFactory.h"
#include "user/detail/ZoneUserImpl.h"
#include "user/ZoneUserManager.h"
#include "user/ZoneUserAllocator.h"
#include "model/gameobject/allocator/PlayerAllocator.h"
#include "model/gameobject/allocator/NpcAllocator.h"
#include "model/gameobject/allocator/HarvestAllocator.h"
#include "model/gameobject/allocator/TreasureAllocator.h"
#include "model/gameobject/allocator/RandomDungeonAllocator.h"
#include "model/gameobject/allocator/ObeliskAllocator.h"
#include "model/gameobject/allocator/DeviceAllocator.h"
#include "model/gameobject/allocator/AnchorAllocator.h"
#include "controller/PlayerController.h"
#include <gideon/serverbase/datatable/DataTableLoader.h>
#include <gideon/server/data/ServerAddress.h>
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/datatable/RegionSpawnTable.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>
#include <gideon/cs/datatable/EntityPathTable.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/session/ServerSideSessionManager.h>
#include <sne/server/s2s/ClientSideProxyFactory.h>
#include <sne/server/utility/Profiler.h>
#include <sne/server/app/Factory.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <atomic>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class ZoneUserAllocatorImpl
 */
class ZoneUserAllocatorImpl : public ZoneUserAllocator
{
public:
    ZoneUserAllocatorImpl(go::EntityAllocator& playerAllocator) :
        ZoneUserAllocator(playerAllocator) {}
    virtual ~ZoneUserAllocatorImpl() {}

    ZoneUser* malloc() {
        return new ZoneUserImpl(getPlayerAllocator());
    }

    void free(ZoneUser* user) {
        boost::checked_delete(user);
    }
};



/**
 * @class SessionFactoryHelperForServers
 * S2S 세션 팩토리 도우미
 */
class SessionFactoryHelperForServers : public sne::server::SessionFactoryHelper
{
public:
    virtual std::unique_ptr<sne::base::SessionFactory> createSessionFactory(
        const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask) {
        return std::unique_ptr<sne::base::SessionFactory>(
            new ArenaServerSideProxyFactory(serverSpec, ioContextTask));
    }
};


/**
 * @class GideonZoneService
 */
class GideonZoneService :
    public ZoneService,
    private WorldMapHelper
{
    typedef ZoneService::LockType LockType;

    typedef std::shared_ptr<datatable::RegionTable> RegionTablePtr;
    typedef sne::core::HashMap<MapCode, RegionTablePtr> RegionTableMap;

    typedef std::shared_ptr<datatable::RegionCoordinates>  RegionCoordinatesPtr;
    typedef sne::core::HashMap<MapCode, RegionCoordinatesPtr> RegionCoordinatesMap;

    typedef std::shared_ptr<datatable::RegionSpawnTable> SpawnTablePtr;
    typedef sne::core::HashMap<MapCode, SpawnTablePtr> SpawnTableMap;

    typedef std::shared_ptr<datatable::PositionSpawnTable> PositionSpawnTablePtr;
    typedef sne::core::HashMap<MapCode, PositionSpawnTablePtr> PositionSpawnTableMap;

    typedef std::shared_ptr<datatable::EntityPathTable> EntityPathTablePtr;
    typedef sne::core::HashMap<MapCode, EntityPathTablePtr> EntityPathTableMap;
    
public:
    GideonZoneService() :
        ioService_(nullptr),
        c2sSpec_(nullptr),
        z2lSpec_(nullptr),
        z2mSpec_(nullptr),
        z2aSpec_(nullptr) {}

    bool initialize(const sne::server::ServerSpec& c2sSpec,
        const sne::server::ServerSpec& z2lSpec, const sne::server::ServerSpec& z2mSpec,
        const sne::server::ServerSpec& z2aSpec, const sne::server::ServerSpec& s2sSpec,
        sne::base::IoContextTask& ioContextTask) {
        c2sSpec_ = &c2sSpec;
        z2lSpec_ = &z2lSpec;
        z2mSpec_ = &z2mSpec;
        z2aSpec_ = &z2aSpec;
        s2sSpec_ = &s2sSpec;
        ioService_ = &ioContextTask;

        return ZoneService::initialize(c2sSpec_->name_);
    }

private:
    virtual const sne::server::S2sCertificate& getServerCertificate() const {
        return s2sSpec_->certificate_;
    }

    virtual size_t getMaxUserCount() const {
        return c2sSpec_->maxUserCount_;
    }

    virtual std::unique_ptr<ZoneUserAllocator> createZoneUserAllocator() {
        return std::make_unique<ZoneUserAllocatorImpl>(getPlayerAllocator());
    }

    virtual std::unique_ptr<ZoneLoginServerProxy> createLoginServerProxy() const {
        auto proxy = sne::server::ClientSideProxyFactory<ZoneLoginServerProxy>::create(
            sne::server::toServerInfo(*z2lSpec_), *ioService_);
        if (! proxy->connect()) {
            proxy.reset();
        }
        return proxy;
    }

    virtual std::unique_ptr<ZoneCommunityServerProxy> createCommunityServerProxy() const {
        auto proxy = sne::server::ClientSideProxyFactory<ZoneCommunityServerProxy>::create(
            sne::server::toServerInfo(*z2mSpec_), *ioService_);
        if (! proxy->connect()) {
            proxy.reset();
        }
        return proxy;
    }

    virtual std::unique_ptr<ZoneArenaServerProxy> createArenaServerProxy() const {
        auto proxy = sne::server::ClientSideProxyFactory<ZoneArenaServerProxy>::create(
            sne::server::toServerInfo(*z2aSpec_), *ioService_);
        if (! proxy->connect()) {
            proxy.reset();
        }
        return proxy;
    }

    virtual go::EntityAllocator& getPlayerAllocator() {
        return playerAllocator_;
    }

    virtual go::EntityAllocator& getNpcAllocator() {
        return npcAllocator_;
    }

    virtual go::EntityAllocator& getHarvestAllocator() {
        return harvestAllocator_;
    }

    virtual go::EntityAllocator& getTreasureAllocator() {
        return treasureAllocator_;
    }

    virtual go::EntityAllocator& getRandomDungeonAllocator() {
        return randomDungeonAllocator_;
    }

    virtual go::EntityAllocator& getDeviceAllocator() {
        return deviceAllocator_;
    }

    virtual go::EntityAllocator& getObeliskAllocator() {
        return obeliskAllocator_;
    }

    virtual go::EntityAllocator& getAnchorAllocator() {
        return anchorAllocator_;
    }

    virtual WorldMapHelper& getWorldMapHelper() {
        return *this;
    }

private: // = WorldMapHelper overriding
    virtual ObjectId generateMapId() const {
        return ++mapIdGenerator_;
    }

    virtual const datatable::RegionTable* getRegionTable(MapCode mapCode) const {
        std::lock_guard<LockType> lock(getLock());

        if (! regionTableMap_[mapCode].get()) {
            regionTableMap_[mapCode].reset(
                serverbase::DataTableLoader::loadRegionTable(mapCode).release());
        }
        return regionTableMap_[mapCode].get();
    }

    virtual const datatable::RegionCoordinates* getRegionCoordinates(MapCode mapCode) const {
        std::lock_guard<LockType> lock(getLock());

        if (! regionCoordinatesMap_[mapCode].get()) {
            regionCoordinatesMap_[mapCode].reset(
                serverbase::DataTableLoader::loadRegionCoordinates(mapCode).release());
        }
        return regionCoordinatesMap_[mapCode].get();
    }

    virtual const datatable::RegionSpawnTable* getRegionSpawnTable(MapCode mapCode) const {
        std::lock_guard<LockType> lock(getLock());

        if (! spawnTableMap_[mapCode].get()) {
            spawnTableMap_[mapCode].reset(
                serverbase::DataTableLoader::loadRegionSpawnTable(mapCode).release());
        }
        return spawnTableMap_[mapCode].get();
    }

    virtual const datatable::PositionSpawnTable* getPositionSpawnTable(MapCode mapCode) const {
        std::lock_guard<LockType> lock(getLock());

        if (! positionSpawnTableMap_[mapCode].get()) {
            positionSpawnTableMap_[mapCode].reset(
                serverbase::DataTableLoader::loadPositionSpawnTable(mapCode).release());
        }
        return positionSpawnTableMap_[mapCode].get();
    }

    virtual const datatable::EntityPathTable* getEntityPathTable(MapCode mapCode) const {
        std::lock_guard<LockType> lock(getLock());

        if (! entityPathTableMap_[mapCode].get()) {
            entityPathTableMap_[mapCode].reset(
                serverbase::DataTableLoader::loadEntityPathTable(mapCode).release());
        }
        return entityPathTableMap_[mapCode].get();
    }

private:
    sne::base::IoContextTask* ioService_;
    const sne::server::ServerSpec* c2sSpec_;
    const sne::server::ServerSpec* z2lSpec_;
    const sne::server::ServerSpec* z2mSpec_;
    const sne::server::ServerSpec* z2aSpec_;
    const sne::server::ServerSpec* s2sSpec_;

    mutable RegionTableMap regionTableMap_;
    mutable RegionCoordinatesMap regionCoordinatesMap_;
    mutable SpawnTableMap spawnTableMap_;
    mutable PositionSpawnTableMap positionSpawnTableMap_;
    mutable EntityPathTableMap entityPathTableMap_;

    go::PlayerAllocator playerAllocator_;
    go::NpcAllocator npcAllocator_;
    go::RandomDungeonAllocator randomDungeonAllocator_;
    go::HarvestAllocator harvestAllocator_;
    go::TreasureAllocator treasureAllocator_;
    go::ObeliskAllocator obeliskAllocator_;
    go::DeviceAllocator deviceAllocator_;
    go::AnchorAllocator anchorAllocator_;

    mutable std::atomic<ObjectId> mapIdGenerator_;
};

} // namespace

// = ZoneServerApp

#pragma warning (disable: 4355)

ZoneServerApp::ZoneServerApp() : 
    zoneService_(std::make_unique<GideonZoneService>())
{
}


ZoneServerApp::~ZoneServerApp()
{
}


bool ZoneServerApp::initZoneService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    SNE_ASSERT(getServerSpec().isValid());

    GideonZoneService& theZoneService =
        static_cast<GideonZoneService&>(*zoneService_);

    if (! theZoneService.initialize(getServerSpec(), z2lSpec_, z2mSpec_,
        z2aSpec_, s2sSpec_, getIoContextTask(getS2sIoServiceName()))) {
        return false;
    }

    if (getServerSpec().name_ == getArenaServerName()) {
        if (! initSessionManagerForServers()) {
            SNE_LOG_ERROR("ZoneServerApp::initSessionManagerForServers() FAILED!");
            return false;
        }
    }

    return true;
}


bool ZoneServerApp::loadServerSpec()
{
    z2lSpec_ = getServerSpec(getLoginServerName(), getS2sIoServiceName());
    if (! z2lSpec_.isValid()) {
        return false;
    }

    z2mSpec_ = getServerSpec(getCommunityServerName(), getS2sIoServiceName());
    if (! z2mSpec_.isValid()) {
        return false;
    }

    if (getServerSpec().name_ == getArenaServerName()) {
        s2sSpec_ = getServerSpec(getServiceConfig().getServiceName(), getS2sIoServiceName());
        return s2sSpec_.isValid();
    }
    else {
        z2aSpec_ = getServerSpec(getArenaServerName(), getS2sIoServiceName());
        return z2aSpec_.isValid();
    }
}


bool ZoneServerApp::loadDataTables()
{
    if (!serverbase::DataTableLoader::loadPropertyTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadCharacterStatusTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadWorldMapTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadEquipTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadCharacterDefaultItemTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadCharacterDefaultSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadAchievementTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadAccessoryTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadReprocessTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadPlayerActiveSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadPlayerPassiveSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadItemActiveSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadSOActiveSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadGuildSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadGuildLevelTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadFunctionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadItemOptionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadItemSuffixTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadNpcActiveSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadSkillEffectTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadNpcTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadElementTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadFragmentTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadItemDropTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadWorldDropTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadWorldDropSuffixTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadGemTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadRecipeTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadExpTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadRandomDungeonTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestItemTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestKillMissionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestObtainMissionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestProbeMissionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestTransportMissionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestContentsMissionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadQuestActivationMissionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadHarvestTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadTreasureTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadNpcSellTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadNpcBuyTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadAnchorTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadBuildingTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadDeviceTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadGliderTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadVehicleTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadResourcesProductionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadArenaTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadHarnessTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadFactionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadNpcTalkingTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadBuildingGuardTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadBuildingGuardSellTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadWorldEventTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadWorldEventMissionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadWorldEventInvaderSpawnTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadWorldEventMissionSpawnTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadActionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadNpcFormationTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadEventTriggerTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadSelectProductionTable()) {
        return false;
    }
    return true;
}


bool ZoneServerApp::initMonitorService()
{
    registerRconQuerySource("user.count", *this);
    registerRconQuerySource("login.count", *this);
    return true;
}


void ZoneServerApp::initGameTimer()
{
    gameTimer_ = std::make_unique<GameTimer>();
    GameTimer::instance(gameTimer_.get());
}


bool ZoneServerApp::initTaskScheduler()
{
    // TODO: config에서 읽을 것
    const uint16_t threadCount = 2;
    auto* ioContextTask = addIoContextTask("task_scheduler", threadCount);
    if (!ioContextTask) {
        return false;
    }
    taskScheduler_ = std::make_unique<sne::base::TaskScheduler>(*ioContextTask);
    sne::base::TaskScheduler::instance(taskScheduler_.get());
    return true;
}


bool ZoneServerApp::initSessionManagerForServers()
{
    sne::server::Profiler profiler(__FUNCTION__);
    if (! s2sSpec_.isValid()) {
        return false;
    }
    static SessionFactoryHelperForServers sessionFactoryHelperForServers;

    sessionManagerForServers_ =
        std::make_unique<sne::server::ServerSideSessionManager>(
            "arena.s2s",
            s2sSpec_, getIoContextTask(getS2sIoServiceName()),
            sessionFactoryHelperForServers);
    if (! sessionManagerForServers_->initialize()) {
        return false;
    }

    if (! sessionManagerForServers_->startSessionAcceptor()) {
        return false;
    }

    return true;

}


void ZoneServerApp::finishZoneService()
{
    zoneService_->finalize();
}

// = serverbase::BaseSessionServerApp overriding

bool ZoneServerApp::onInit()
{
    if (! serverbase::BaseSessionServerApp::onInit()) {
        return false;
    }

    if (! loadServerSpec()) {
        SNE_LOG_ERROR("ZoneServerApp::loadLoginServerSpec() FAILED!");
        return false;
    }

    if (! initMonitorService()) {
        SNE_LOG_ERROR("ZoneServerApp::initMonitorService() FAILED!");
        return false;
    }

    initGameTimer();

    if (! initTaskScheduler()) {
        SNE_LOG_ERROR("ZoneServerApp::initTaskScheduler() FAILED!");
        return false;
    }

    if (! addIoContextTask(getS2sIoServiceName(),
        z2lSpec_.workerThreadCount_)) {
        SNE_LOG_ERROR("ZoneServerApp::addProactorTask() FAILED!");
        return false;
    }

    if (!loadDataTables()) {
        return false;
    }

    if (! initZoneService()) {
        SNE_LOG_ERROR("ZoneServerApp::initZoneService() FAILED!");
        return false;
    }

    return true;
}


bool ZoneServerApp::onStart()
{
    zoneService_->ready();

    if (! serverbase::BaseSessionServerApp::onStart()) {
        return false;
    }

    return true;
}


void ZoneServerApp::onFinish()
{
    serverbase::BaseSessionServerApp::onFinish();

    finishZoneService();
}

// = sne::server::SessionFactoryHelper overriding

std::unique_ptr<sne::base::SessionFactory> ZoneServerApp::createSessionFactory(
    const sne::server::ServerSpec& serverSpec,
    sne::base::IoContextTask& ioContextTask)
{
    return std::unique_ptr<sne::base::SessionFactory>(
        new ZoneClientSessionFactory(serverSpec, ioContextTask));
}

// = sne::server::rcon::CounterSource overriding

std::string ZoneServerApp::query(const std::string& name,
    const std::string& detail) const
{
    if ("user.count" == name) {
        return std::to_string(ZONEUSER_MANAGER->getUserCount());
    }

    if ("login.count" == name) {
        return std::to_string(ZONEUSER_MANAGER->getLoginCount());
    }

    return serverbase::BaseSessionServerApp::query(name, detail);
}

}} // namespace gideon { namespace zoneserver {


DEFINE_SNE_APP_FACTORY(gideon::zoneserver::ZoneServerApp)

