#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockNpcController.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerQuestController.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/model/gameobject/Npc.h"
#include "ZoneServer/model/state/CreatureState.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/servertest/datatable/MockSkillTable.h>
#include <sne/database/DatabaseManager.h>

using gideon::servertest::MockProxyGameDatabase;

/**
* @class QuestTest
*
* 퀘스트 관련 테스트
*/
class QuestTest : public GameTestFixture
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


void QuestTest::killMonster1()
{
    ASSERT_EQ(monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
        hpDefault);
    monster1_->queryCreatureState()->invincible(false);

    //const int prevDiedCount = playerController1_->getCallCount("playerController1_");

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


TEST_F(QuestTest, testCancelQuest)
{
    playerQuestController1_->cancelQuest(servertest::killScorpionQuestCode);
    ASSERT_EQ(ecQuestNotFindQuest, playerQuestController1_->lastErrorCode_);

    playerQuestController1_->acceptQuest(servertest::killScorpionQuestCode, monsterInfo1_);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    playerQuestController1_->cancelQuest(servertest::killScorpionQuestCode);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    playerQuestController1_->acceptQuest(servertest::killScorpionQuestCode, monsterInfo1_);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);


    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(2, db->getCallCount("acceptQuest"));
    ASSERT_EQ(1, db->getCallCount("cancelQuest"));
}


TEST_F(QuestTest, testCompleteQuest)
{
    playerQuestController1_->acceptQuest(servertest::killScorpionQuestCode, monsterInfo1_);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    playerQuestController1_->completeQuest(servertest::killScorpionQuestCode, 1, 0);
    ASSERT_EQ(ecQuestNotCompleteQuest, playerQuestController1_->lastErrorCode_);

    killMonster1();

    playerQuestController1_->completeQuest(servertest::killScorpionQuestCode, 1, 0);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    ASSERT_EQ(1, playerController1_->getCallCount("evMoneyRewarded"));
}


TEST_F(QuestTest, testObtainQuest)
{
    const ObjectId stackCount = 1;
    const SlotId slotId = 15;

    BaseItemInfo elementItem(servertest::normalNpcElementCode1, stackCount);

    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( elementItem, slotId));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

    playerQuestController1_->acceptQuest(servertest::obtainScorpionQuestCode, monsterInfo1_);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evQuestItemAdded"));

    playerQuestController1_->completeQuest(servertest::obtainScorpionQuestCode, 4, 0);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);
    ASSERT_EQ(1, playerController1_->getCallCount("evMoneyRewarded"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evQuestItemRemoved"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));

}


TEST_F(QuestTest, testTransportQuest)
{
    playerQuestController1_->acceptQuest(servertest::transportQuestCode, monsterInfo1_);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);
    
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evQuestItemAdded"));

    playerQuestController1_->completeTransportMission(servertest::transportQuestCode,
        servertest::questItemTransportMissionCode);
    ASSERT_EQ(1, playerQuestController1_->getCallCount("onCompleteTransportMission"));
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    playerQuestController1_->completeQuest(servertest::transportQuestCode, 4, 0);
    ASSERT_EQ(1, playerController1_->getCallCount("evMoneyRewarded"));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evQuestItemRemoved"));
}



TEST_F(QuestTest, testAcceptRepeatQuest)
{
    playerQuestController1_->acceptQuest(servertest::repeatQuestCode, monsterInfo1_);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    playerQuestController1_->acceptQuest(servertest::repeatQuestCode, monsterInfo1_);
    ASSERT_EQ(ecQuestAlreayAcceptedQuest, playerQuestController1_->lastErrorCode_);

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("acceptRepeatQuest"));
}


TEST_F(QuestTest, testReAcceptRepeatQuest)
{
    playerQuestController1_->acceptQuest(servertest::repeatQuestCode, monsterInfo1_);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    playerQuestController1_->completeQuest(servertest::repeatQuestCode, 1, 0);
    ASSERT_EQ(ecQuestNotCompleteQuest, playerQuestController1_->lastErrorCode_);

    killMonster1();

    playerQuestController1_->completeQuest(servertest::repeatQuestCode, 1, 0);
    ASSERT_EQ(ecOk, playerQuestController1_->lastErrorCode_);

    ASSERT_EQ(1, playerController1_->getCallCount("evMoneyRewarded"));

    playerQuestController1_->acceptQuest(servertest::repeatQuestCode, monsterInfo1_);
    ASSERT_EQ(ecQuestNotEnoughAcceptTime, playerQuestController1_->lastErrorCode_);
}
