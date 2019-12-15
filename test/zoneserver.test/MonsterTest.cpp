#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockNpcController.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerEffectController.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "zoneserver/world/region/SpawnMapRegion.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include "ZoneServer/model/gameobject/Npc.h"
#include "ZoneServer/model/gameobject/status/CreatureStatus.h"
#include "ZoneServer/model/state/CreatureState.h"
#include "ZoneServer/service/skill/Skill.h"
#include <gideon/servertest/datatable/MockSkillTable.h>


using namespace zoneserver;

namespace {

size_t getSpawnedNpcCount(const WorldMap& worldMap)
{
    const WorldMap::MapRegionMap& mapRegions = worldMap.getMapRegions();

    size_t npcCount = 0;
    for (const WorldMap::MapRegionMap::value_type& value : mapRegions) {
        const MapRegion* region = value.second;
        if (rtNpcSpawn != region->getRegionType()) {
            continue;
        }

        npcCount += region->getEntityCount(otNpc);
    }

    return npcCount;
}

size_t getSpawnedMonsterCount(const WorldMap& worldMap)
{
    const WorldMap::MapRegionMap& mapRegions = worldMap.getMapRegions();

    size_t npcCount = 0;
    for (const WorldMap::MapRegionMap::value_type& value : mapRegions) {
        const MapRegion* region = value.second;
        if (rtNpcSpawn != region->getRegionType()) {
            continue;
        }

        npcCount += region->getEntityCount(otMonster);
    }

    return npcCount;
}

size_t getSpawnedHarvestCount(const WorldMap& worldMap)
{
	const WorldMap::MapRegionMap& mapRegions = worldMap.getMapRegions();

	size_t harvestCount = 0;
	for (const WorldMap::MapRegionMap::value_type& value : mapRegions) {
		const MapRegion* region = value.second;
		if (rtHarvest != region->getRegionType()) {
			continue;
		}

		harvestCount += region->getEntityCount(otHarvest);
	}

	return harvestCount;
}

size_t getSpawnedTreasureCount(const WorldMap& worldMap)
{
	const WorldMap::MapRegionMap& mapRegions = worldMap.getMapRegions();

	size_t treasureCount = 0;
	for (const WorldMap::MapRegionMap::value_type& value : mapRegions) {
		const MapRegion* region = value.second;
		if (rtTreasure != region->getRegionType()) {
			continue;
		}

		treasureCount += region->getEntityCount(otTreasure);
	}

	return treasureCount;
}

} // namespace

/**
* @class MonsterTest
*
* Monster 관련 테스트
*/
class MonsterTest : public GameTestFixture
{
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();

        monsterInfo1_.objectType_ = otMonster;
        monsterInfo1_.objectId_ = 1;
        monster1_ =
            static_cast<go::Npc*>(WORLD->getGlobalWorldMap().getEntity(monsterInfo1_));
        ASSERT_TRUE(monster1_ != nullptr);
        monsterController1_ = &monster1_->getControllerAs<MockNpcController>();

        monsterInfo2_.objectType_ = otMonster;
        monsterInfo2_.objectId_ = 2;
        monster2_ =
            static_cast<go::Npc*>(WORLD->getGlobalWorldMap().getEntity(monsterInfo2_));
        ASSERT_TRUE(monster2_ != nullptr);
        monsterController2_ = &monster2_->getControllerAs<MockNpcController>();

        //skillCode_ = servertest::defaultMeleeAttackSkillCode;
        //skillTemplate_ = SKILL_TABLE->getSkill(skillCode_);
        //ASSERT_TRUE(skillTemplate_ != nullptr);
    }

protected:
    void killMonster1();

    MockNpcController& getMockNpcController(go::Npc& monster) {
        return monster.getControllerAs<MockNpcController>();
    }

protected:
    GameObjectInfo monsterInfo1_;
    go::Npc* monster1_;
    MockNpcController* monsterController1_;

    GameObjectInfo monsterInfo2_;
    go::Npc* monster2_;
    MockNpcController* monsterController2_;

    SkillCode skillCode_;
    //const datatable::SkillTemplate* skillTemplate_;
};


void MonsterTest::killMonster1()
{
    ASSERT_EQ(monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
        hpDefault);

    //const int prevDiedCount = playerController1_->getCallCount("playerController1_");
    //monster1_->queryCreatureState()->invincible(false);

    //for (;;) {
    //    playerCastController1_->startCasting(createUnionSkillCastToInfo(monsterInfo1_, skillCode_));

    //    const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(skillCode_);
    //    ASSERT_TRUE(skillTemplate != nullptr);
    //    mockGameTimerSource_->set(GAME_TIMER->msec() +
    //        skillTemplate->getSkillInfo()->cool_time() + 1);

    //    if (playerController1_->getCallCount("evNpcDied") > prevDiedCount) {
    //        break;
    //    }
    //    // FYI: MockTaskScheduler에 의해 부활되어 버리므로 아래 코드를 사용하면 안됨!
    //    //if (monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.isDied()) {
    //    //    break;
    //    //}
    //}
}


TEST_F(MonsterTest, testNpcSpawned)
{
    const WorldMap& globalWorldMap = WORLD->getGlobalWorldMap();
    const size_t creatureCount =
        globalWorldMap.getEntityCount(otMonster) + globalWorldMap.getEntityCount(otNpc) +
        globalWorldMap.getEntityCount(otPc) + globalWorldMap.getEntityCount(otHarvest) +
		globalWorldMap.getEntityCount(otTreasure);

    ASSERT_EQ(getSpawnedNpcCount(globalWorldMap) + getSpawnedHarvestCount(globalWorldMap) +
		getSpawnedTreasureCount(globalWorldMap)  + getSpawnedMonsterCount(globalWorldMap) +
        ZONEUSER_MANAGER->getUserCount(), creatureCount);

    ASSERT_EQ(1, playerController1_->getCallCount("evEntitiesAppeared"));
    ASSERT_EQ(creatureCount - 1, playerController1_->lastEntityInfos_.size());
    ASSERT_EQ(creatureCount, playerController2_->lastEntityInfos_.size());
}


TEST_F(MonsterTest, testMeleeAttackToNpc)
{
    ASSERT_EQ(monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
        hpDefault);
    monster1_->queryCreatureState()->invincible(false);

    playerCastController1_->startCasting(createUnionSkillCastToInfo(monsterInfo1_, skillCode_));
    ASSERT_EQ(1, playerEffectController1_->getCallCount("evEffectApplied"));

    //ASSERT_EQ(monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(hpDefault - skillTemplate_->getSkillInfo()->castee_effect_value_1()));
}


TEST_F(MonsterTest, testDie)
{
    killMonster1();

    ASSERT_EQ(1, playerController1_->getCallCount("evNpcDied"));
    ASSERT_EQ(1, playerController2_->getCallCount("evNpcDied"));

    ASSERT_TRUE(
        monster1_->getCurrentWorldMap()->getEntity(monsterInfo1_) != nullptr);
}


TEST_F(MonsterTest, testRespawn)
{
    ASSERT_EQ(0, playerController1_->getCallCount("evEntityDisappeared"));
    ASSERT_EQ(1, playerController1_->getCallCount("evEntityAppeared"));
    ASSERT_EQ(7 + 1, monsterController2_->getCallCount("entityAppeared"));

    killMonster1();

    ASSERT_EQ(1, playerController1_->getCallCount("evEntityDisappeared"));
    ASSERT_EQ(1, playerController2_->getCallCount("evEntityDisappeared"));
    ASSERT_EQ(1, monsterController1_->getCallCount("entityDisappeared"));

    ASSERT_EQ(2, playerController1_->getCallCount("evEntityAppeared"));
    ASSERT_EQ(7 + 1 + 1, monsterController2_->getCallCount("entityAppeared"));

    const CreatureInfo& creatureInfo = monster1_->getUnionEntityInfo().asCreatureInfo();
    const go::Creature* monster = static_cast<const go::Creature*>(monster1_);
    const Points maxPoints = monster->getCreatureStatus().getMaxPoints();
    ASSERT_EQ(creatureInfo.currentPoints_.hp_, maxPoints.hp_);
    ASSERT_EQ(creatureInfo.currentPoints_.mp_, maxPoints.mp_);
}
