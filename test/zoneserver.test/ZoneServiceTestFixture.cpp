#include "ZoneServerTestPCH.h"
#include "ZoneServiceTestFixture.h"
#include "ZoneServer/ZoneService.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "MockZoneLoginServerProxy.h"
#include "MockZoneCommunityServerProxy.h"
#include "MockZoneArenaServerProxy.h"
#include "MockZoneUser.h"
#include "MockPlayerController.h"
#include "MockPlayerAllocator.h"
#include "MockPlayerPartyController.h"
#include "MockPlayerTradeController.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerSkillController.h"
#include "MockPlayerGraveStoneController.h"
#include "MockPlayerQuestController.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerHarvestController.h"
#include "MockPlayerTreasureController.h"
#include "MockPlayerItemController.h"
#include "MockPlayerAnchorController.h"
#include "MockPlayerNpcController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerMailController.h"
#include "MockPlayerAuctionController.h"
#include "MockPlayerTeleportController.h"
#include "MockGameTimer.h"
#include "MockTaskScheduler.h"
#include "MockSummonController.h"
#include <gideon/servertest/DatabaseTestFixture.h>
#include <gideon/servertest/datatable/MockPropertyTable.h>
#include <gideon/servertest/datatable/MockWorldMapTable.h>
#include <gideon/servertest/datatable/MockWorldEventTable.h>
#include <gideon/servertest/datatable/MockSkillTable.h>
#include <gideon/servertest/datatable/MockEquipTable.h>
#include <gideon/servertest/datatable/MockNpcTable.h>
#include <gideon/servertest/datatable/MockNpcTalkingTable.h>
#include <gideon/servertest/datatable/MockExpTable.h>
#include <gideon/servertest/datatable/MockElementTable.h>
#include <gideon/servertest/datatable/MockRecipeTable.h>
#include <gideon/servertest/datatable/MockFragmentTable.h>
#include <gideon/servertest/datatable/MockRandomDungeonTable.h>
#include <gideon/servertest/datatable/MockQuestTable.h>
#include <gideon/servertest/datatable/MockQuestItemTable.h>
#include <gideon/servertest/datatable/MockQuestKillMissionTable.h>
#include <gideon/servertest/datatable/MockQuestObtainMissionTable.h>
#include <gideon/servertest/datatable/MockQuestTransportMissionTable.h>
#include <gideon/servertest/datatable/MockHarvestTable.h>
#include <gideon/servertest/datatable/MockTreasureTable.h>
#include <gideon/servertest/datatable/MockAnchorTable.h>
#include <gideon/servertest/datatable/MockBuildingTable.h>
#include <gideon/servertest/datatable/MockFactionTable.h>
#include <gideon/servertest/datatable/MockGliderTable.h>
#include <gideon/servertest/datatable/MockVehicleTable.h>
#include <gideon/servertest/datatable/MockEventTriggerTable.h>
#include <gideon/servertest/datatable/MockRegionTable.h>
#include <gideon/servertest/datatable/MockRegionCoordinates.h>
#include <gideon/servertest/datatable/MockRegionSpawnTable.h>
#include <gideon/servertest/datatable/MockFragmentTable.h>
#include <gideon/servertest/datatable/MockSkillEffectTable.h>
#include <gideon/servertest/datatable/MockPlayerActiveSkillTable.h>
#include <gideon/servertest/datatable/MockPlayerPassiveSkillTable.h>
#include <gideon/servertest/datatable/MockNpcActiveSkillTable.h>
#include <gideon/servertest/datatable/MockSOActiveSkillTable.h>
#include <gideon/servertest/datatable/MockCharacterStatusTable.h>
#include "ZoneServer/world/WorldMapHelper.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include "ZoneServer/user/ZoneUserAllocator.h"
#include "ZoneServer/model/gameobject/allocator/NpcAllocator.h"
#include "zoneserver/model/gameobject/allocator/HarvestAllocator.h"
#include "zoneserver/model/gameobject/allocator/TreasureAllocator.h"
#include "zoneserver/model/gameobject/allocator/ObeliskAllocator.h"
#include "zoneserver/model/gameobject/allocator/RandomDungeonAllocator.h"
#include "zoneserver/model/gameobject/allocator/DeviceAllocator.h"
#include "zoneserver/model/gameobject/allocator/AnchorAllocator.h"
#include "ZoneServer/model/gameobject/Npc.h"
#include "ZoneServer/model/gameobject/Harvest.h"
#include "ZoneServer/model/gameobject/Treasure.h"
#include "ZoneServer/controller/RandomDungeonController.h"
#include "ZoneServer/service/item/ItemIdGenerator.h"
#include "ZoneServer/service/anchor/AnchorService.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>
#include <sne/base/io_context/IoContextTask.h>

using namespace zoneserver;

namespace {

servertest::MockPropertyTable mockPropertyTable;
servertest::MockWorldMapTable mockWorldMapTable;
servertest::MockWorldEventTable mockWorldEventTable;
servertest::MockEquipTable mockEquipTable;
servertest::MockElementTable mockElmentTable;
servertest::MockExpTable mockExpTable;
servertest::MockRecipeTable mockRecipeTable;
servertest::MockNpcTable mockNpcTable;
servertest::MockNpcTalkingTable mockNpcTalkingTable;
servertest::MockFragmentTable mockFragmentTable;
servertest::MockRandomDungeonTable mockRandomDungeonTable;
servertest::MockQuestTable mockQuestTable;
servertest::MockQuestItemTable mockQuestItemTable;
servertest::MockQuestKillMissionTable mockQuestKillMissionTable;
servertest::MockQuestObtainMissionTable mockQuestObtainMissionTable;
servertest::MockQuestTransportMissionTable mockQuestTransportMissionTable;
servertest::MockHarvestTable mockHarvestTable;
servertest::MockTreasureTable mockTreasureTable;
servertest::MockAnchorTable mockAnchorTable;
servertest::MockBuildingTable mockBuildingTable;
servertest::MockFactionTable mockFactionTable;
servertest::MockEventTriggerTable mockEventTriggerTable;
servertest::MockVehicleTable mockVehicleTable;
servertest::MockGliderTable mockGliderTable;

servertest::MockSkillEffectTable mockSkillEffectTable;
servertest::MockPlayerActiveSkillTable mockPlayerActiveSkillTable;
servertest::MockPlayerPassiveSkillTable mockPlayerPassiveSkillTable;
servertest::MockNpcActiveSkillTable mockNpcActiveSkillTable;
servertest::MockSOActiveSkillTable mockSOActiveSkillTable;
servertest::MockCharacterStatusTable mockCharacterStatusTable;

void initializeDataTableSingletons()
{
    datatable::PropertyTable::instance(&mockPropertyTable);
    datatable::WorldMapTable::instance(&mockWorldMapTable);
    datatable::WorldEventTable::instance(&mockWorldEventTable);
    datatable::EquipTable::instance(&mockEquipTable);
    datatable::NpcTable::instance(&mockNpcTable);
    datatable::NpcTalkingTable::instance(&mockNpcTalkingTable);
    datatable::ElementTable::instance(&mockElmentTable);
    datatable::RecipeTable::instance(&mockRecipeTable);
    datatable::ExpTable::instance(&mockExpTable);
    datatable::FragmentTable::instance(&mockFragmentTable);
    datatable::RandomDungeonTable::instance(&mockRandomDungeonTable);
    datatable::QuestTable::instance(&mockQuestTable);
    datatable::QuestItemTable::instance(&mockQuestItemTable);
    datatable::QuestKillMissionTable::instance(&mockQuestKillMissionTable);
    datatable::QuestObtainMissionTable::instance(&mockQuestObtainMissionTable);
    datatable::QuestTransportMissionTable::instance(&mockQuestTransportMissionTable);
    datatable::FactionTable::instance(&mockFactionTable);
    datatable::EventTriggerTable::instance(&mockEventTriggerTable);

    servertest::MockHarvestTable::instance(&mockHarvestTable);
    servertest::MockTreasureTable::instance(&mockTreasureTable);
    servertest::MockAnchorTable::instance(&mockAnchorTable);
    servertest::MockBuildingTable::instance(&mockBuildingTable);
    servertest::MockVehicleTable::instance(&mockVehicleTable);
    servertest::MockGliderTable::instance(&mockGliderTable);

    servertest::MockSkillEffectTable::instance(&mockSkillEffectTable);
    servertest::MockPlayerActiveSkillTable::instance(&mockPlayerActiveSkillTable);
    servertest::MockPlayerPassiveSkillTable::instance(&mockPlayerPassiveSkillTable);
    servertest::MockNpcActiveSkillTable::instance(&mockNpcActiveSkillTable);
    servertest::MockSOActiveSkillTable::instance(&mockSOActiveSkillTable);

    servertest::MockCharacterStatusTable::instance(&mockCharacterStatusTable);
}

} // namespace

namespace
{

/**
 * @class MockZoneUserAllocator
 */
class MockZoneUserAllocator : public ZoneUserAllocator
{
public:
    MockZoneUserAllocator(go::EntityAllocator& playerAllocator) :
        ZoneUserAllocator(playerAllocator) {}
    virtual ~MockZoneUserAllocator() {}

    ZoneUser* malloc() override {
        return new MockZoneUser(getPlayerAllocator());
    }

    void free(ZoneUser* user) override {
        boost::checked_delete(user);
    }
};


/**
 * @class MockNpcAllocator
 */
class MockNpcAllocator : public go::NpcAllocator
{
public:
    go::Entity* malloc() override {
        return new go::Npc(std::make_unique<MockNpcController>());
    }
};


/**
 * @class MockZoneService
 */
class MockZoneService :
    public ZoneService,
    private WorldMapHelper
{
public:
    MockZoneService() :
        mockLoginServerProxy_(nullptr) {}

    MockZoneLoginServerProxy& getLoginServerProxy() {
        return *mockLoginServerProxy_;
    }

    MockZoneCommunityServerProxy& getCommunityServerProxy() {
        return *mockCommunityServerProxy_;
    }

private:
    virtual const sne::server::S2sCertificate& getServerCertificate() const override {
        static const sne::server::S2sCertificate cert("gee");
        return cert;
    }

    virtual size_t getMaxUserCount() const override {
        return ZoneServiceTestFixture::maxZoneServerUserCount;
    }

    virtual std::unique_ptr<ZoneUserAllocator> createZoneUserAllocator() override {
        return std::make_unique<MockZoneUserAllocator>(getPlayerAllocator());
    }

    virtual std::unique_ptr<ZoneLoginServerProxy> createLoginServerProxy() const override {
        auto proxy = std::make_unique<MockZoneLoginServerProxy>();
        (void)proxy->connect();
        mockLoginServerProxy_ = proxy.get();
        return proxy;
    }

    virtual std::unique_ptr<ZoneCommunityServerProxy> createCommunityServerProxy() const override {
        auto proxy = std::make_unique<MockZoneCommunityServerProxy>();
        (void)proxy->connect();
        mockCommunityServerProxy_ = proxy.get();
        return proxy;
    }

    virtual std::unique_ptr<ZoneArenaServerProxy> createArenaServerProxy() const override {
        auto proxy = std::make_unique<MockZoneArenaServerProxy>();
        (void)proxy->connect();
        mockArenaServerProxy_ = proxy.get();
        return proxy;
    }

    virtual go::EntityAllocator& getPlayerAllocator() override {
        return playerAllocator_;
    }

    virtual go::EntityAllocator& getNpcAllocator() override {
        return npcAllocator_;
    }

    virtual go::EntityAllocator& getHarvestAllocator() override {
        return harvestAllocator_;
    }

    virtual go::EntityAllocator& getTreasureAllocator() override {
        return treasureAllocator_;
    }

    virtual go::EntityAllocator& getRandomDungeonAllocator() override {
        return randomDungeonAllocator_;
    }

    virtual go::EntityAllocator& getObeliskAllocator() override {
        return obeliskAllocator_;
    }

    virtual go::EntityAllocator& getDeviceAllocator() override {
        return deviceAllocator_;
    }

    virtual go::EntityAllocator& getAnchorAllocator() override {
        return anchorAllocator_;
    }

    WorldMapHelper& getWorldMapHelper() override {
        return *this;
    }

private:
    // = WorldMapHelper overriding
    ObjectId generateMapId() const override {
        static ObjectId mapId = invalidObjectId;
        return ++mapId;
    }
    const datatable::RegionTable* getRegionTable(MapCode mapCode) const override {
        static servertest::MockRegionTable regionTable(mapCode);
        return &regionTable;
    }

    const datatable::RegionCoordinates* getRegionCoordinates(MapCode mapCode) const override {
        static servertest::MockRegionCoordinates regionCoordinates(mapCode);
        return &regionCoordinates;
    }

    const datatable::RegionSpawnTable* getRegionSpawnTable(MapCode mapCode) const override {
        static servertest::MockRegionSpawnTable RegionSpawnTable(mapCode);
        return &RegionSpawnTable;
    }

    const datatable::PositionSpawnTable* getPositionSpawnTable(MapCode /*mapCode*/) const override {
        //static servertest::MockRegionSpawnTable RegionSpawnTable(mapCode);
        return nullptr;
    }

    const datatable::EntityPathTable* getEntityPathTable(MapCode /*mapCode*/) const override {
        return nullptr;
    }


    // = for test
    bool timerResultSkip() const override {
        return true;
    }

private:
    MockPlayerAllocator playerAllocator_;
    MockNpcAllocator npcAllocator_;
    go::RandomDungeonAllocator randomDungeonAllocator_;
    go::HarvestAllocator harvestAllocator_;
    go::TreasureAllocator treasureAllocator_;
    go::ObeliskAllocator obeliskAllocator_;
    go::DeviceAllocator deviceAllocator_;
    go::AnchorAllocator anchorAllocator_;

    mutable MockZoneLoginServerProxy* mockLoginServerProxy_;
    mutable MockZoneCommunityServerProxy* mockCommunityServerProxy_;
    mutable MockZoneArenaServerProxy* mockArenaServerProxy_;
};

} // namespace

// = ZoneServiceTestFixture

ZoneServiceTestFixture::ZoneServiceTestFixture() :
    zoneService_(nullptr)
{
    initializeDataTableSingletons();
}


ZoneServiceTestFixture::~ZoneServiceTestFixture()
{
}


void ZoneServiceTestFixture::SetUp()
{
    DatabaseTestFixture::SetUp();

    ASSERT_TRUE(getProperties());

    go::Harvest::setNotUseTimer();
    go::Treasure::setNotUseTimer();

    auto timerSource = std::make_unique<MockGameTimerSource>();
    mockGameTimerSource_ = static_cast<MockGameTimerSource*>(timerSource.get());
    gameTimer_= std::make_unique<GameTimer>(std::move(timerSource));
    GameTimer::instance(gameTimer_.get());

    ioContextTask_ = std::make_unique<sne::base::IoContextTask>("ZoneServiceTestFixture");
    ioContextTask_->createForTicker();
    taskScheduler_= std::make_unique<MockTaskScheduler>(*ioContextTask_);
    sne::base::TaskScheduler::instance(taskScheduler_.get());

    itemIdGenerator_= std::make_unique<ItemIdGenerator>(12345678);
    ItemIdGenerator::instance(itemIdGenerator_.get());

    zoneService_ = new MockZoneService;
    ASSERT_TRUE(zoneService_->initialize("zone_server"));
    ZoneService::instance(zoneService_);

    accountId1_ = 1;
    setValidAccount(accountId1_);

    mockGameTimerSource_->set(10000);
}


void ZoneServiceTestFixture::TearDown()
{
    DatabaseTestFixture::TearDown();

    zoneService_->finalize();
    delete zoneService_;
}


void ZoneServiceTestFixture::setValidAccount(AccountId accountId)
{
    sne::database::Guard<servertest::MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    db->setValidAccount(accountId, makeId(accountId), makePassword(accountId));
}


ObjectId ZoneServiceTestFixture::createCharacter(AccountId accountId)
{
    const ZoneId zoneId = 1;

    CharacterAppearance appearance(1, 1, 1);

    CreateCharacterEquipments equipments;
    equipments[epRightHand].equipCode_ = servertest::defaultOneHandSwordEquipCode;
    equipments[epLeftHand].equipCode_ = servertest::defaultShieldEquipCode;
    equipments[epHelmet].equipCode_ = servertest::defaultHelmetEquipCode;    
    // = 테스트용이라 의미 없음 디비에서 갱신해버림
    equipments[epRightHand].equipId_ = 1;
    equipments[epLeftHand].equipId_ = 2;
    equipments[epHelmet].equipId_ = 3;

    CreateCharacterInfo charInfo(accountId, ccAny, appearance, servertest::defaultOneHandSwordEquipCode,
        makeId(accountId), stFemale);

    sne::database::Guard<servertest::MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    sne::base::Future::Ref future =
        db->asyncCreateCharacter(charInfo, equipments, zoneId, WorldPosition());

    if (! future) { // future->waitForDone()
        EXPECT_TRUE(false);
        return invalidObjectId;
    }

    const auto& requestFuture =
        static_cast<const serverbase::CreateCharacterRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        EXPECT_TRUE(false);
        return invalidObjectId;
    }

    return requestFuture.characterInfo_.objectId_;
}


void ZoneServiceTestFixture::loginUser(AccountId accountId)
{
    const Certificate certificate = Certificate::issue(stZoneServer, accountId, 1);
    ZONE_SERVICE->login(certificate, invalidObjectId, invalidRegionCode, invalidMapCode, Position(), sessionCallback_);
}


MockZoneLoginServerProxy& ZoneServiceTestFixture::getLoginServerProxy()
{
    return static_cast<MockZoneService*>(zoneService_)->getLoginServerProxy();
}


MockZoneCommunityServerProxy& ZoneServiceTestFixture::getCommunityServerProxy()
{
    return static_cast<MockZoneService*>(zoneService_)->getCommunityServerProxy();
}


MockZoneUser* ZoneServiceTestFixture::getMockZoneUser(AccountId accountId)
{
    return static_cast<MockZoneUser*>(ZONEUSER_MANAGER->getUser(accountId));
}


MockPlayerController& ZoneServiceTestFixture::getMockPlayerController(
    MockZoneUser& user)
{
    return user.getActivePlayer().getControllerAs<MockPlayerController>();
}


MockPlayerMoveController& ZoneServiceTestFixture::getMockPlayerMoveController(MockZoneUser& user)
{
    return static_cast<MockPlayerMoveController&>(user.getActivePlayer().getMoveController());
}


MockPlayerSkillController& ZoneServiceTestFixture::getMockPlayerSkillController(MockZoneUser& user)
{
    return static_cast<MockPlayerSkillController&>(user.getActivePlayer().getSkillController());
}


MockPlayerInventoryController& ZoneServiceTestFixture::getMockPlayerInventoryController(MockZoneUser& user)
{
    return static_cast<MockPlayerInventoryController&>(
        user.getActivePlayer().getPlayerInventoryController());
}


MockPlayerQuestController& ZoneServiceTestFixture::getMockPlayerQuestController(MockZoneUser& user)
{
    return static_cast<MockPlayerQuestController&>(
        user.getActivePlayer().getPlayerQuestController());
}


MockPlayerTradeController& ZoneServiceTestFixture::getMockPlayerTradeController(MockZoneUser& user)
{
    return static_cast<MockPlayerTradeController&>(user.getActivePlayer().getPlayerTradeController());
}


MockPlayerHarvestController& ZoneServiceTestFixture::getMockPlayerHarvestController(MockZoneUser& user)
{
    return static_cast<MockPlayerHarvestController&>(user.getActivePlayer().getPlayerHarvestController());
}


MockPlayerTreasureController& ZoneServiceTestFixture::getMockPlayerTreasureController(MockZoneUser& user)
{
    return static_cast<MockPlayerTreasureController&>(user.getActivePlayer().getPlayerTreasureController());
}


MockPlayerGraveStoneController& ZoneServiceTestFixture::getMockPlayerGraveStoneController(MockZoneUser& user)
{
    return static_cast<MockPlayerGraveStoneController&>(
        user.getActivePlayer().queryGraveStoneable()->getPlayerGraveStoneController());
}


MockPlayerItemController& ZoneServiceTestFixture::getMockPlayerItemController(MockZoneUser& user)
{
    return static_cast<MockPlayerItemController&>(user.getActivePlayer().getPlayerItemController());
}


MockPlayerPartyController& ZoneServiceTestFixture::getMockPlayerPartyController(MockZoneUser& user)
{
    return static_cast<MockPlayerPartyController&>(
        user.getActivePlayer().getPlayerPartyController());
}


MockPlayerAnchorController& ZoneServiceTestFixture::getMockPlayerAnchorController(MockZoneUser& user)
{
    return static_cast<MockPlayerAnchorController&>(user.getActivePlayer().getPlayerAnchorController());
}


MockPlayerNpcController& ZoneServiceTestFixture::getMockPlayerNpcController(MockZoneUser& user)
{
    return static_cast<MockPlayerNpcController&>(user.getActivePlayer().getPlayerNpcController());
}


MockPlayerCastController& ZoneServiceTestFixture::getMockPlayerCastController(MockZoneUser& user)
{
    return static_cast<MockPlayerCastController&>(user.getActivePlayer().getPlayerCastController());
}


MockPlayerEffectController& ZoneServiceTestFixture::getMockPlayerEffectController(MockZoneUser& user)
{
    return static_cast<MockPlayerEffectController&>(user.getActivePlayer().getEffectController());
}


MockPlayerMailController& ZoneServiceTestFixture::getMockPlayerMailController(MockZoneUser& user)
{
    return static_cast<MockPlayerMailController&>(user.getActivePlayer().getPlayerMailController());
}


MockPlayerAuctionController& ZoneServiceTestFixture::getMockPlayerAuctionController(MockZoneUser& user)
{
    return static_cast<MockPlayerAuctionController&>(user.getActivePlayer().getPlayerAuctionController());
}


MockPlayerTeleportController& ZoneServiceTestFixture::getMockPlayerTeleportController(MockZoneUser& user)
{
    return static_cast<MockPlayerTeleportController&>(user.getActivePlayer().getPlayerTeleportController());

}
