#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerController.h"
#include "MockZoneCommunityServerProxy.h"
#include "MockPlayerPartyController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "zoneserver/world/region/SpawnMapRegion.h"
#include "ZoneServer/service/spawn/SpawnService.h"
#include "ZoneServer/model/gameobject/RandomDungeon.h"

using namespace zoneserver;

namespace {

size_t getSpawnedRandomDungeonCount(const WorldMap& worldMap)
{
    const WorldMap::MapRegionMap& mapRegions = worldMap.getMapRegions();

    size_t dungeonCount = 0;
    for (const WorldMap::MapRegionMap::value_type& value : mapRegions) {
        const MapRegion* region = value.second;
        if (rtRandomDungeon != region->getRegionType()) {
            continue;
        }

        dungeonCount += region->getEntityCount(otDungeon);
    }

    return dungeonCount;
}

} // namespace

/**
* @class RandomDungeonTest
*
* 랜덤 던전 관련 테스트
*/
class RandomDungeonTest : public GameTestFixture
{
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();

        dungeonInfo_ = playerController1_->getAnyEntity(otDungeon);
        ASSERT_TRUE(dungeonInfo_.isValid());
    }

protected:
    GameObjectInfo dungeonInfo_;
};


TEST_F(RandomDungeonTest, testRandomDungeonSpawned)
{
    const WorldMap& globalWorldMap = WORLD->getGlobalWorldMap();
    const size_t dungeonCount = getSpawnedRandomDungeonCount(globalWorldMap);

    playerController1_->enterDungeon(dungeonInfo_.objectId_);
    ASSERT_EQ(dungeonCount,
        globalWorldMap.getEntityCount(otDungeon));

    ASSERT_EQ(1 + dungeonCount,
        getCommunityServerProxy().getCallCount("z2m_worldMapOpened"));
}


TEST_F(RandomDungeonTest, testEnterRandomDungeon)
{
    WorldMap* worldMap = player1_->getCurrentWorldMap();

    playerController1_->enterDungeon(dungeonInfo_.objectId_);
    ASSERT_EQ(1, playerController1_->getCallCount("onEnterDungeon"));
    ASSERT_EQ(ecOk, playerController1_->lastErrorCode_);

    playerController1_->readyToPlay();
    ASSERT_EQ(2, playerController1_->getCallCount("onReadyToPlay"));

    ASSERT_TRUE(! worldMap->getEntity(player1_->getGameObjectInfo()));
}


TEST_F(RandomDungeonTest, testReenterRandomDungeon)
{
    playerController1_->enterDungeon(dungeonInfo_.objectId_);
    playerController1_->readyToPlay();
    ASSERT_EQ(2, playerController1_->getCallCount("onReadyToPlay"));

    playerController1_->enterDungeon(dungeonInfo_.objectId_);
    ASSERT_EQ(2, playerController1_->getCallCount("onEnterDungeon"));
    ASSERT_EQ(ecDungeonNotFound, playerController1_->lastErrorCode_);
}


TEST_F(RandomDungeonTest, testReadyToPlayWithoutEnterRandomDungeon)
{
    playerController1_->readyToPlay();
    ASSERT_EQ(2, playerController1_->getCallCount("onReadyToPlay"));
    ASSERT_EQ(ecWorldMapAlreadyEntered, playerController1_->lastErrorCode_);
}


TEST_F(RandomDungeonTest, testLeaveRandomDungeonWithoutEntrance)
{
    playerController1_->leaveDungeon();
    ASSERT_EQ(1, playerController1_->getCallCount("onLeaveDungeon"));
    ASSERT_EQ(ecDungeonNotEntered, playerController1_->lastErrorCode_);
}


TEST_F(RandomDungeonTest, testLeaveRandomDungeon)
{
    playerController1_->enterDungeon(dungeonInfo_.objectId_);
    playerController1_->readyToPlay();
    ASSERT_EQ(2, playerController1_->getCallCount("onReadyToPlay"));

    playerController1_->leaveDungeon();
    ASSERT_EQ(1, playerController1_->getCallCount("onLeaveDungeon"));
    ASSERT_EQ(ecOk, playerController1_->lastErrorCode_);

    playerController1_->readyToPlay();
    ASSERT_EQ(3, playerController1_->getCallCount("onReadyToPlay"));
}


TEST_F(RandomDungeonTest, testNotifyRandomDungeonClosed)
{
    ASSERT_EQ(0, getCommunityServerProxy().getCallCount("z2m_worldMapClosed"));
    go::RandomDungeon* dungeon =
        static_cast<go::RandomDungeon*>(player1_->queryKnowable()->getEntity(dungeonInfo_));
    ASSERT_TRUE(dungeon != nullptr);
    playerController1_->enterDungeon(dungeonInfo_.objectId_);
    playerController1_->leaveDungeon();
    SPAWN_SERVICE->scheduleDespawn(*dungeon, 0);

    ASSERT_EQ(1, getCommunityServerProxy().getCallCount("z2m_worldMapClosed"));
}
