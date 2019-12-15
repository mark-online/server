#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockNpcController.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/world/region/MapRegion.h"
#include "ZoneServer/model/gameobject/Npc.h"
#include "ZoneServer/model/state/CreatureState.h"
#include "ZoneServer/service/skill/Skill.h"
#include <gideon/servertest/datatable/MockSkillTable.h>

using namespace zoneserver;

/**
* @class GrowthTest
*
* Growth 관련 테스트
*/
class GrowthTest : public GameTestFixture
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

/*        skillCode_ = servertest::defaultMeleeAttackSkillCode;
        skillTemplate_ = SKILL_TABLE->getSkill(skillCode_);
        ASSERT_TRUE(skillTemplate_ != nullptr);  */      
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

void GrowthTest::killMonster1()
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

TEST_F(GrowthTest, testNotLevelUp)
{
    killMonster1();

    ASSERT_EQ(0, playerController1_->getCallCount("evPlayerLeveledUp"));
    ASSERT_EQ(0, playerController2_->getCallCount("evPlayerLeveledUp"));
    ASSERT_EQ(1, playerController1_->getCallCount("evExpPointUpdated"));
    ASSERT_TRUE(! playerController2_->lastIsMajorLevelUp_);
}


TEST_F(GrowthTest, testMinorLevelUp)
{
    playerController1_->rewardExp(toExpPoint(110));
    killMonster1();

    ASSERT_EQ(1, playerController1_->getCallCount("evPlayerLeveledUp"));
    ASSERT_EQ(1, playerController2_->getCallCount("evPlayerLeveledUp"));
    ASSERT_TRUE(! playerController2_->lastIsMajorLevelUp_);
}


TEST_F(GrowthTest, testMajorLevelUp)
{
    playerController1_->rewardExp(toExpPoint(1640));
    killMonster1();

    ASSERT_EQ(1 + 1, playerController1_->getCallCount("evPlayerLeveledUp"));
    ASSERT_EQ(1 + 1, playerController2_->getCallCount("evPlayerLeveledUp"));
    ASSERT_EQ(1 +1, playerController1_->getCallCount("evPlayerLeveledUpInfo"));
    ASSERT_EQ(0, playerController2_->getCallCount("evPlayerLeveledUpInfo"));
    ASSERT_TRUE(playerController2_->lastIsMajorLevelUp_);
}


TEST_F(GrowthTest, testMaxLevelUp)
{
    const int oneMaxStackExp = 65535;
    const int maxLevelStackExp = 841890;
    int popStackExp = maxLevelStackExp;
    int count = 0;
    while (popStackExp != 0) {
        ExpPoint addExp = static_cast<ExpPoint>(oneMaxStackExp < popStackExp ? oneMaxStackExp : popStackExp);        
        playerController1_->rewardExp(toExpPoint(addExp));
        popStackExp -= addExp;
        ++count;
    }

    killMonster1();

    ASSERT_EQ(count, playerController1_->getCallCount("evPlayerLeveledUp"));
    ASSERT_EQ(count, playerController2_->getCallCount("evPlayerLeveledUp"));
    ASSERT_EQ(count, playerController1_->getCallCount("evPlayerLeveledUpInfo"));
    ASSERT_EQ(0, playerController2_->getCallCount("evPlayerLeveledUpInfo"));
    ASSERT_TRUE(playerController2_->lastIsMajorLevelUp_);
}