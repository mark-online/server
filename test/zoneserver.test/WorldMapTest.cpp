#include "ZoneServerTestPCH.h"
#include "MockTaskScheduler.h"
#include "ZoneServer/service/time/GameTimer.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/world/WorldMapHelper.h"
#include "ZoneServer/world/Sector.h"
#include "ZoneServer/model/gameobject/Creature.h"
#include "ZoneServer/model/gameobject/GraveStone.h"
#include "ZoneServer/model/gameobject/status/CreatureStatus.h"
#include "ZoneServer/model/gameobject/skilleffect/PassiveSkillManager.h"
#include "ZoneServer/controller/SkillController.h"
#include "ZoneServer/controller/CreatureController.h"
#include "ZoneServer/controller/CreatureEffectController.h"
#include "ZoneServer/controller/MoveController.h"
#include <gideon/servertest/datatable/MockRegionTable.h>
#include <gideon/servertest/datatable/MockRegionCoordinates.h>
#include <gideon/servertest/datatable/MockRegionSpawnTable.h>
#include <gideon/servertest/datatable/MockRandomDungeonTable.h>
#include <gideon/servertest/datatable/MockPropertyTable.h>
#include <gideon/servertest/datatable/MockFactionTable.h>
#include <gideon/servertest/datatable/MockEventTriggerTable.h>
#include <sne/test/CallCounter.h>
#include <sne/base/io_context/IoContextTask.h>

using namespace sne;
using namespace gideon;
using namespace gideon::zoneserver;

namespace {

const size_t horizontalSectors = 11;
const size_t verticalSectors = 11;
const uint16_t sectorLength = 5;


/**
 * @class MockCreatureController
 */
class MockCreatureController :
    public gc::CreatureController,
    public test::CallCounter
{
public:
    // = InterestAreaCallback overriding
    void entitiesAppeared(const go::EntityMap& entities) override {
        gc::CreatureController::entitiesAppeared(entities);
        addCallCount("entitiesAppeared");
    }

    void entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo) override {
        gc::CreatureController::entityAppeared(entity, entityInfo);
        addCallCount("entityAppeared");
    }

    void entitiesDisappeared(const go::EntityMap& entities) override {
        gc::CreatureController::entitiesDisappeared(entities);
        addCallCount("entitiesDisappeared");
    }

    void entityDisappeared(go::Entity& entity) override {
        gc::CreatureController::entityDisappeared(entity);
        addCallCount("entityDisappeared");
    }

    void entityDisappeared(const GameObjectInfo& info) override {
        gc::CreatureController::entityDisappeared(info);
        addCallCount("entityDisappeared");
    }

    // = CreatureController overriding
    void spawned(WorldMap& worldMap) override {
        worldMap;
    }

    void despawned(WorldMap& worldMap) override {
        worldMap;
    }

    // = SkillCastCallback overriding
    virtual void standGraveStone() {}
    virtual void stunned(const GameObjectInfo& /*creatureInfo*/, bool /*isActivate*/) {}
    virtual void stun(bool /*isActivate*/) {}

    virtual void castSkillTo(ErrorCode errorCode,
        const GameObjectInfo& targetInfo, SkillCode skillCode) {
        errorCode, targetInfo, skillCode;
    }

    virtual void castSkillStartedTo(const GameObjectInfo& targetInfo,
        const GameObjectInfo& sourceInfo, SkillCode skillCode) {
        targetInfo, sourceInfo, skillCode;
    }

    virtual void skillCastingCancelled(const GameObjectInfo& sourceInfo,
        SkillCode skillCode) {
        sourceInfo, skillCode;
    }

    virtual void targetSkillCastingCompleted(const GameObjectInfo& sourceInfo,
        const GameObjectInfo& targetInfo, SkillCode skillCode) {
        sourceInfo, targetInfo, skillCode;
    }

    virtual void areaSkillCastingCompleted(const GameObjectInfo& sourceInfo,
        const Position& targetPosition, SkillCode skillCode) {
            sourceInfo, targetPosition, skillCode;
    }

    virtual void playerGraveStoneStood(go::GraveStone& graveStone) {
        graveStone;
    }

    // = EffectCallback overriding
   
    virtual void effectApplied(go::Entity& to, go::Entity& from,
        const SkillEffectResult& skillResult) {
            to, from, skillResult;
    }
};


/**
 * @class MockSkillController
 */
class MockSkillController : public gc::SkillController
{
public:
    MockSkillController(go::Entity* owner) : SkillController(owner) {}
    virtual ~MockSkillController() {}
};



/**
 * @class MockSkillController
 */
class MockEffectController : public gc::CreatureEffectController
{
public:
    MockEffectController(go::Entity* owner) : CreatureEffectController(owner) {}
    virtual ~MockEffectController() {}

private:
    /// entity가 스킬의 영향을 받았다(to, from만 받음)
    virtual void effectApplied(go::Entity& /*to*/, go::Entity& /*from*/,
        const SkillEffectResult& /*skillEffectResult*/) override {}
    virtual void effectHit(go::Entity& /*to*/, go::Entity& /*from*/, DataCode /*dataCode*/) override {}

    /// 전체가 받음
    virtual void effectDefenced(go::Entity& /*to*/, go::Entity& /*from*/, 
        DataCode /*dataCode*/, EffectDefenceType /*type*/) {}
    virtual void standGraveStone() override {}

    virtual void mesmerizationEffected(const GameObjectInfo& /*creatureInfo*/, MesmerizationType /*mezt*/, bool /*isActivate*/) override {}
    virtual void mutated(const GameObjectInfo& /*creatureInfo*/, NpcCode /*npcCode*/,
        bool /*isActivate*/) override {}
    virtual void transformed(const GameObjectInfo& /*creatureInfo*/, NpcCode /*npcCode*/,
        bool /*isActivate*/) override {}
    virtual void releaseHidden(const UnionEntityInfo& /*creatureInfo*/) override {}
    virtual void playerGraveStoneStood(go::GraveStone& /*graveStone*/) override {}
    virtual void moveSpeedChanged(const GameObjectInfo& /*creatureInfo*/, float32_t /*currentSpeed*/) override {}
    virtual void frenzied(const GameObjectInfo& /*creatureInfo*/, float32_t /*currentScale*/, bool /*isActivate*/) override {}
    virtual void knockbacked(const GameObjectInfo& /*creatureInfo*/, const Position& /*position*/) override {}
    virtual void knockbackReleased(const GameObjectInfo& /*creatureInfo*/) override {}
    virtual void dashing(const GameObjectInfo& /*creatureInfo*/, const Position& /*position*/) override {}
    virtual void reviveEffected(const GameObjectInfo& /*creatureInfo*/, const ObjectPosition& /*position*/, HitPoint /*currentHp*/) override {}
    virtual void effectAdded(const GameObjectInfo& /*creatureInfo*/,
        const DebuffBuffEffectInfo& /*info*/) override {}
    virtual void effectRemoved(const GameObjectInfo& /*creatureInfo*/,
        DataCode /*dataCode*/, bool /*isCaster*/) override {}
};



/**
 * @class MockMoveController
 */
class MockMoveController : public gc::MoveController
{
public:
    MockMoveController(go::Entity* owner) :
        MoveController(owner) {}

    void entityGoneToward(const GameObjectInfo& /*entityInfo*/,
        const ObjectPosition& /*position*/) override {}

    void entityMoved(const GameObjectInfo& /*entityInfo*/,
        const ObjectPosition& /*position*/) override {}

    void entityStopped(const GameObjectInfo& /*entityInfo*/,
        const ObjectPosition& /*position*/) override {}

    void entityTeleported(const GameObjectInfo& /*entityInfo*/,
        const ObjectPosition& /*position*/) override {}

    void entityTurned(const GameObjectInfo& /*entityInfo*/,
        Heading /*heading*/) override {}

    void entityJumped(const GameObjectInfo& /*entityInfo*/,
        const ObjectPosition& /*position*/) override {}

    void entityRun(const GameObjectInfo& /*entityInfo*/) override {}

    void entityWalked(const GameObjectInfo& /*entityInfo*/) override {}
};


/**
 * @class MockCreature
 */
class MockCreature : public go::Creature
{
public:
    MockCreature() :
        go::Creature(std::make_unique<MockCreatureController>()) {}

    bool initialize(ObjectId objectId) {
        return go::Creature::initialize(otPc, objectId, 1);
    }

    void frenze(int32_t /*size*/, bool /*isRelease*/) override {}
    bool reviveByEffect(HitPoint& /*refillPoint*/, permil_t /*perRefillHp*/) override { return true;}
    CreatureLevel getCreatureLevel() const override {
        return clMinLevel;
    }

    int getCallCount(const std::string& funcName) const {
        return getControllerAs<MockCreatureController>().getCallCount(funcName);
    }

private:
    std::unique_ptr<gc::MoveController> createMoveController() override {
        return std::make_unique<MockMoveController>(this);
    }

    std::unique_ptr<go::CreatureStatus> createCreatureStatus() override {
        return nullptr;
    }

    go::PassiveSkillManager& getPassiveSkillManager() override {        
        go::PassiveSkillManager* temp = new go::PassiveSkillManager(*this);
        return *(temp);
    } 

    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const GameObjectInfo& targetInfo) const {
        skillCode, targetInfo;
        return ecOk;
    }

    void notifyChangeCondition(PassiveCheckCondition /*condition*/) override {}

    bool checkPassiveCastablePoints(const Points& /*points*/) const {
        return true;
    }

    virtual bool initSkills() {
        return true;
    }

    ErrorCode revive(bool /*isSkipTime*/) override { return ecServerInternalError;}

    void initState(CreatureStateInfo& /*stateInfo*/) override {}

    float32_t getCurrentDefaultSpeed() const override {
        return 5.0f;
    }

    std::unique_ptr<gc::SkillController> createSkillController() override {
        return std::make_unique<MockSkillController>(this);
    }

    std::unique_ptr<gc::CreatureEffectController> createEffectController() override {
        return std::make_unique<MockEffectController>(this);
    }

    virtual void reviveEffected(const GameObjectInfo& /*creatureInfo*/, const ObjectPosition& /*position*/, HitPoint /*currentHp*/) {}

    void tick(GameTime) override {}

    void reserveReleaseDownSpeed() override {}
    void reserveReleaseUpSpeed() override {}

private:
    Points mockMaxPoints_;
};


std::unique_ptr<MockCreature> createMockCreature(ObjectId objectId)
{
    auto creature = std::make_unique<MockCreature>();
    EXPECT_TRUE(creature->initialize(objectId));
    return creature;
}

} // namespace {

/**
* @class WorldMapTest
*
* Map WorldMap 테스트
*/
class WorldMapTest :
    public testing::Test,
    private WorldMapHelper
{
protected:
    using MockCreaturePtr = std::unique_ptr<MockCreature>;

public:
    WorldMapTest() {
        datatable::PropertyTable::instance(&mockPropertyTable_);
    }

private:
    virtual void SetUp() {
        gameTimer_ = std::make_unique<GameTimer>();
        GameTimer::instance(gameTimer_.get());

        ioContextTask_ = std::make_unique<sne::base::IoContextTask>("WorldMapTest");
        ioContextTask_->createForTicker();
        taskScheduler_ = std::make_unique<MockTaskScheduler>(*ioContextTask_);
        sne::base::TaskScheduler::instance(taskScheduler_.get());

        datatable::RandomDungeonTable::instance(&mockRandomDungeonTable_);
        datatable::FactionTable::instance(&mockFactionTable_);
        datatable::EventTriggerTable::instance(&mockEventTriggerTable_);

        MapInfo mapInfo;
        mapInfo.mapCode_ = makeMapCode(mtGlobalWorld, 1);
        mapInfo.originX_ = 0;
        mapInfo.originY_ = 0;
        mapInfo.originZ_ = 0;
        mapInfo.xSectorCount_ = horizontalSectors;
        mapInfo.ySectorCount_ = verticalSectors;
        mapInfo.zSectorCount_ = 3;
        mapInfo.sectorLength_ = sectorLength;

        WorldMapHelper& worldMapHelper = *this;
        worldMap_ = std::make_unique<WorldMap>(worldMapHelper, invalidObjectId + 1);
        ASSERT_EQ(true,
            worldMap_->initialize(mapInfo)) <<
                "initialize world map";

        creature1_ = createMockCreature(1);
        creature1_->setWorldMap(*worldMap_);
        creature1_->setMapRegion(worldMap_->getGlobalMapRegion());

        creature2_ = createMockCreature(2);
        creature2_->setWorldMap(*worldMap_);
        creature2_->setMapRegion(worldMap_->getGlobalMapRegion());

        creature3_ = createMockCreature(3);
        creature3_->setWorldMap(*worldMap_);
        creature3_->setMapRegion(worldMap_->getGlobalMapRegion());
    }

private:
    // = WorldMapHelper overriding
    ObjectId generateMapId() const override {
        assert(false);
        return invalidObjectId;
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
        return nullptr;
    }

    const datatable::EntityPathTable* getEntityPathTable(MapCode /*mapCode*/) const override {
        return nullptr;
    }

protected:
    std::unique_ptr<sne::base::IoContextTask> ioContextTask_;
    std::unique_ptr<sne::base::TaskScheduler> taskScheduler_;

    servertest::MockPropertyTable mockPropertyTable_;
    servertest::MockRandomDungeonTable mockRandomDungeonTable_;
    servertest::MockFactionTable mockFactionTable_;
    servertest::MockEventTriggerTable mockEventTriggerTable_;

    std::unique_ptr<zoneserver::GameTimer> gameTimer_;
    std::unique_ptr<WorldMap> worldMap_;
    MockCreaturePtr creature1_;
    MockCreaturePtr creature2_;
    MockCreaturePtr creature3_;
};


TEST_F(WorldMapTest, testInitialized)
{
    ASSERT_EQ(horizontalSectors * verticalSectors,
        static_cast<int>(worldMap_->getSectorCount())) <<
        "total sector count";
}


TEST_F(WorldMapTest, testSectorAddress)
{
    const Sector* sector00 = worldMap_->getSector(SectorAddress(0, 0));
    ASSERT_TRUE(sector00 != nullptr);
    ASSERT_EQ(0, sector00->getAddress().getColumn());
    ASSERT_EQ(0, sector00->getAddress().getRow());
    ASSERT_EQ(0.0f, sector00->getX());
    ASSERT_EQ(0.0f, sector00->getY());

    const Sector* sector12 = worldMap_->getSector(SectorAddress(1, 2));
    ASSERT_TRUE(sector12 != nullptr);
    ASSERT_EQ(1, sector12->getAddress().getColumn());
    ASSERT_EQ(2, sector12->getAddress().getRow());
    ASSERT_EQ(1 * sectorLength, sector12->getX());
    ASSERT_EQ(2 * sectorLength, sector12->getY());
}


TEST_F(WorldMapTest, testSectorAddressNeighbor)
{
    const SectorAddress address(0, 0);

    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirEast)) != nullptr);
    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirNorthEast)) != nullptr);
    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirNorth)) != nullptr);
    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirNorthWest)) != nullptr);
    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirWest)) != nullptr);
    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirSouthWest)) != nullptr);
    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirSouth)) != nullptr);
    ASSERT_TRUE(worldMap_->getSector(address.getNeighbor(dirSouthEast)) != nullptr);

    const Sector* sector00 = worldMap_->getSector(address.getNeighbor(dirEast).
        getNeighbor(dirNorth).getNeighbor(dirWest).getNeighbor(dirSouth));
    ASSERT_TRUE(sector00 != nullptr);
    ASSERT_EQ(0, sector00->getAddress().getColumn());
    ASSERT_EQ(0, sector00->getAddress().getRow());
    ASSERT_EQ(0.0f, sector00->getX());
    ASSERT_EQ(0.0f, sector00->getY());
}


TEST_F(WorldMapTest, testGetSectorByPosition)
{
    const Sector* sector = worldMap_->getSector(
        -(2 * sectorLength + 1.0f), (3 * sectorLength + 1.0f));
    ASSERT_TRUE(sector != nullptr);
    ASSERT_EQ(-2, sector->getAddress().getColumn());
    ASSERT_EQ(3, sector->getAddress().getRow());
    ASSERT_EQ(-(2 * sectorLength), sector->getX());
    ASSERT_EQ((3 * sectorLength), sector->getY());

    const float x = -(1000 * sectorLength);
    const float y = (1000 * sectorLength);
    ASSERT_TRUE(! worldMap_->getSector(x, y));
}


TEST_F(WorldMapTest, testSectorNeighbor)
{
    const Sector* sector00 = worldMap_->getSector(SectorAddress(0, 0));
    ASSERT_TRUE(sector00->getNeighbor(dirWest) != nullptr);
    ASSERT_TRUE(sector00->getNeighbor(dirEast) != nullptr);
    ASSERT_EQ(sector00,
        sector00->getNeighbor(dirEast)->getNeighbor(dirNorth)->
            getNeighbor(dirWest)->getNeighbor(dirSouth));
}


TEST_F(WorldMapTest, testSpawn)
{
    const ObjectPosition position(1.0f, 2.0f, 3.0f, 4);
    const size_t userCount = 2;

    ASSERT_EQ(ecOk, worldMap_->spawn(*creature1_, position));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature2_, position));

    const Sector* sector00 = worldMap_->getSector(SectorAddress(0, 0));
    ASSERT_EQ(userCount, sector00->getEntityCount());

    ASSERT_FLOAT_EQ(position.x_, creature1_->getPosition().x_);
    ASSERT_FLOAT_EQ(position.y_, creature2_->getPosition().y_);

    ASSERT_EQ(2, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature2_->queryKnowable()->getKnownEntities().size());

    ASSERT_EQ(1, creature1_->getCallCount("entitiesAppeared"));
    ASSERT_EQ(1, creature2_->getCallCount("entitiesAppeared"));

    ASSERT_EQ(1, creature1_->getCallCount("entityAppeared"));
    ASSERT_EQ(0, creature2_->getCallCount("entityAppeared"));
}


TEST_F(WorldMapTest, testSpawnWithInvalidPosition)
{
    const float invalidX = ((horizontalSectors / 2) + 1 ) * sectorLength;
    const ObjectPosition position(invalidX, 0.0f, 0.0f, 0);

    ASSERT_TRUE(! worldMap_->getSector(position));

    ASSERT_EQ(ecZoneInvalidSpawnPosition, worldMap_->spawn(*creature1_, position));
}


TEST_F(WorldMapTest, testSpawnWithAdjacentSectors)
{
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature1_, ObjectPosition(0, 0, 0, 0)));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature2_, ObjectPosition(sectorLength, 0, 0, 0)));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature3_, ObjectPosition(0, sectorLength, 0, 0)));

    const Sector* sector00 = worldMap_->getSector(SectorAddress(0, 0));
    ASSERT_EQ(1, sector00->getEntityCount());
    ASSERT_EQ(3, sector00->getInterestingEntityCount());

    const Sector* sector10 = worldMap_->getSector(SectorAddress(1, 0));
    ASSERT_EQ(1, sector10->getEntityCount());
    ASSERT_EQ(3, sector10->getInterestingEntityCount());

    const Sector* sector01 = worldMap_->getSector(SectorAddress(0, 1));
    ASSERT_EQ(1, sector01->getEntityCount());
    ASSERT_EQ(3, sector01->getInterestingEntityCount());

    ASSERT_EQ(3, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(3, creature2_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(3, creature3_->queryKnowable()->getKnownEntities().size());
}


TEST_F(WorldMapTest, testDespawn)
{
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature1_, ObjectPosition(0, 0, 0, 0)));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature2_, ObjectPosition(0, 0, 0, 0)));

    ASSERT_EQ(ecOk, worldMap_->despawn(*creature1_));
    ASSERT_EQ(ecOk, worldMap_->despawn(*creature2_));

    const Sector* sector00 = worldMap_->getSector(SectorAddress(0, 0));
    ASSERT_EQ(0, sector00->getEntityCount());
    ASSERT_EQ(0, sector00->getInterestingEntityCount());

    ASSERT_EQ(0, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(0, creature2_->queryKnowable()->getKnownEntities().size());

    ASSERT_EQ(1, creature1_->getCallCount("entityDisappeared"));
    ASSERT_EQ(2, creature2_->getCallCount("entityDisappeared"));
}
 

TEST_F(WorldMapTest, testMoveInSameSector)
{
    const ObjectPosition origin(0, 0, 0, 0);
    const ObjectPosition destin(sectorLength - 0.1f, 0, 0, 0);

    ASSERT_TRUE(! worldMap_->canMove(origin,
        ObjectPosition((horizontalSectors * sectorLength) + 1, 0, 0, 0)));

    ASSERT_EQ(ecOk, worldMap_->spawn(*creature1_, origin));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature2_, origin));

    ASSERT_TRUE(worldMap_->canMove(origin, destin));

    //ASSERT_TRUE(! worldMap_->migrate(*creature1_, origin, destin));
}


TEST_F(WorldMapTest, testMoveToEast)
{
    const ObjectPosition origin(0, 0, 0, 0);
    const ObjectPosition destin(sectorLength, 0, 0, 0);

    ASSERT_EQ(worldMap_->getSector(origin)->getNeighbor(dirEast),
        worldMap_->getSector(destin));

    ASSERT_EQ(ecOk, worldMap_->spawn(*creature1_, origin));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature2_, origin));

    ASSERT_EQ(2, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature2_->queryKnowable()->getKnownEntities().size());

    ASSERT_TRUE(worldMap_->canMove(origin, destin));
    ASSERT_TRUE(worldMap_->migrate(*creature1_, origin, destin) != nullptr);

    const Sector* sector1 = worldMap_->getSector(creature1_->getSectorAddress());
    ASSERT_EQ(1, sector1->getEntityCount());
    ASSERT_EQ(2, sector1->getInterestingEntityCount());

    const Sector* sector2 = worldMap_->getSector(creature2_->getSectorAddress());
    ASSERT_EQ(1, sector2->getEntityCount());
    ASSERT_EQ(2, sector2->getInterestingEntityCount());

    ASSERT_EQ(2, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature2_->queryKnowable()->getKnownEntities().size());

    ASSERT_EQ(0, creature1_->getCallCount("entityDisappeared"));
    ASSERT_EQ(0, creature2_->getCallCount("entityDisappeared"));
}


TEST_F(WorldMapTest, testMoveToFarEast)
{
    const ObjectPosition origin(0, 0, 0, 0);
    const ObjectPosition destin(sectorLength * 3, 0, 0, 0);

    ASSERT_EQ(ecOk, worldMap_->spawn(*creature1_, origin));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature2_, origin));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature3_, destin));

    ASSERT_EQ(0, creature1_->getCallCount("entityDisappeared"));
    ASSERT_EQ(0, creature1_->getCallCount("entitiesDisappeared"));
    ASSERT_EQ(1, creature1_->getCallCount("entityAppeared"));
    ASSERT_EQ(1, creature1_->getCallCount("entitiesAppeared"));
    ASSERT_EQ(0, creature2_->getCallCount("entityDisappeared"));
    ASSERT_EQ(0, creature2_->getCallCount("entityAppeared"));
    ASSERT_EQ(0, creature3_->getCallCount("entityDisappeared"));
    ASSERT_EQ(0, creature3_->getCallCount("entityAppeared"));

    ASSERT_EQ(2, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature2_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(1, creature3_->queryKnowable()->getKnownEntities().size());

    ASSERT_TRUE(worldMap_->canMove(origin, destin));
    ASSERT_TRUE(worldMap_->migrate(*creature1_, origin, destin) != nullptr);

    ASSERT_EQ(2, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(1, creature2_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature3_->queryKnowable()->getKnownEntities().size());

    const Sector* sector1 = worldMap_->getSector(creature1_->getSectorAddress());
    ASSERT_EQ(2, sector1->getEntityCount());
    ASSERT_EQ(2, sector1->getInterestingEntityCount());

    const Sector* sector2 = worldMap_->getSector(creature2_->getSectorAddress());
    ASSERT_EQ(1, sector2->getEntityCount());
    ASSERT_EQ(1, sector2->getInterestingEntityCount());

    const Sector* sector3 = worldMap_->getSector(creature3_->getSectorAddress());
    ASSERT_EQ(2, sector3->getEntityCount());
    ASSERT_EQ(2, sector3->getInterestingEntityCount());

    ASSERT_EQ(0, creature1_->getCallCount("entityDisappeared"));
    ASSERT_EQ(1, creature1_->getCallCount("entitiesDisappeared"));
    ASSERT_EQ(1, creature1_->getCallCount("entityAppeared"));
    ASSERT_EQ(2, creature1_->getCallCount("entitiesAppeared"));
    ASSERT_EQ(1, creature2_->getCallCount("entityDisappeared"));
    ASSERT_EQ(0, creature2_->getCallCount("entityAppeared"));
    ASSERT_EQ(0, creature3_->getCallCount("entityDisappeared"));
    ASSERT_EQ(1, creature3_->getCallCount("entityAppeared"));
}


TEST_F(WorldMapTest, testMoveToOverlappedSector)
{
    const ObjectPosition origin(0, 0, 0, 0);
    const ObjectPosition destin(sectorLength * 1, 0, 0, 0);

    ASSERT_EQ(ecOk, worldMap_->spawn(*creature1_, origin));
    ASSERT_EQ(ecOk, worldMap_->spawn(*creature2_, origin));
    ASSERT_EQ(ecOk,
        worldMap_->spawn(*creature3_, ObjectPosition(sectorLength * 2, 0, 0, 0)));

    ASSERT_EQ(2, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature2_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(1, creature3_->queryKnowable()->getKnownEntities().size());

    {
        const Sector* sector2 = worldMap_->getSector(creature2_->getSectorAddress());
        ASSERT_EQ(2, sector2->getEntityCount());
        ASSERT_EQ(2, sector2->getInterestingEntityCount());
    }

    ASSERT_TRUE(worldMap_->canMove(origin, destin));
    ASSERT_TRUE(worldMap_->migrate(*creature1_, origin, destin) != nullptr);

    ASSERT_EQ(3, creature1_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature2_->queryKnowable()->getKnownEntities().size());
    ASSERT_EQ(2, creature3_->queryKnowable()->getKnownEntities().size());

    const Sector* sector1 = worldMap_->getSector(creature1_->getSectorAddress());
    ASSERT_EQ(1, sector1->getEntityCount());
    ASSERT_EQ(3, sector1->getInterestingEntityCount());

    const Sector* sector2 = worldMap_->getSector(creature2_->getSectorAddress());
    ASSERT_EQ(1, sector2->getEntityCount());
    ASSERT_EQ(2, sector2->getInterestingEntityCount());

    const Sector* sector3 = worldMap_->getSector(creature3_->getSectorAddress());
    ASSERT_EQ(1, sector3->getEntityCount());
    ASSERT_EQ(2, sector3->getInterestingEntityCount());
}
