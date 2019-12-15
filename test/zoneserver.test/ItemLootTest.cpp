#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockNpcController.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerGraveStoneController.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/model/gameobject/Npc.h"
#include "ZoneServer/model/state/CreatureState.h"
#include "ZoneServer/service/skill/Skill.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/servertest/datatable/MockSkillTable.h>

using namespace zoneserver;


const GameTime defaultLootingDelay = 5000;

/**
* @class ItemLootTest
*
* 비석 관련 테스트
*/
class ItemLootTest : public GameTestFixture
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


        // 비석이 호출될때 evEntityAppeared 호출이 되면 안된다. 
        //TODO: 비석생성시 evEntityAppeared 호출 스킵~
        //ASSERT_EQ(2, playerController1_->getCallCount("evEntityAppeared"));
        //ASSERT_EQ(2, playerController1_->getCallCount("evEntityAppeared"));
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
    ObjectId lastGraveStoneId_;
};

void ItemLootTest::killMonster1()
{
    ASSERT_EQ(monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
        hpDefault);

    //const int prevDiedCount = playerController1_->getCallCount("evNpcDied");
    //monster1_->queryCreatureState()->invincible(false);

    //for (;;) {
    //    playerCastController1_->startCasting(createUnionSkillCastToInfo(monsterInfo1_, skillCode_));

    //    const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(skillCode_);
    //    ASSERT_TRUE(skillTemplate != nullptr);
    //    mockGameTimerSource_->set(GAME_TIMER->msec() +
    //        skillTemplate->getSkillInfo()->cool_time() + 1);

    //    if (playerController1_->getCallCount("evNpcDied") > prevDiedCount) {
    //        lastGraveStoneId_ = playerController1_->lastGraveStoneInfo_.objectId_;
    //        break;
    //    }
    //    // FYI: MockTaskScheduler에 의해 부활되어 버리므로 아래 코드를 사용하면 안됨!
    //    //if (monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.isDied()) {
    //    //    break;
    //    //}
    //}
}

TEST_F(ItemLootTest, testRequestStoneInsideInfo)
{
    //killMonster1();
    //ASSERT_EQ(1, playerController1_->getCallCount("evNpcDied"));
    //ASSERT_TRUE(playerController1_->lastGraveStoneInfo_.isValid());

    //playerGraveStoneController1_->queryGraveStoneInsideInfo(lastGraveStoneId_);
    //ASSERT_EQ(1, playerGraveStoneController1_->getCallCount("onGraveStoneInsideInfo"));
    //ASSERT_EQ(ecOk, playerGraveStoneController1_->lastErrorCode_);
    //ASSERT_EQ(0, playerGraveStoneController2_->getCallCount("onGraveStoneInsideInfo"));
    //LootInvenItemInfoMap& info = playerGraveStoneController1_->lastGraveStoneInsideinfo_;
    //ASSERT_TRUE(! info.empty());
}


TEST_F(ItemLootTest, testAquireItemInGraveStone)
{
    //killMonster1();

    //playerGraveStoneController1_->queryGraveStoneInsideInfo(lastGraveStoneId_);
    //LootInvenItemInfoMap& info = playerGraveStoneController1_->lastGraveStoneInsideinfo_;
    //ASSERT_TRUE(! info.graveStoneItemMap_.empty());
    //LootInvenId graveStoneItemId = firstGraveStoneItemId;

    //playerGraveStoneController1_->startLooting(lastGraveStoneId_, graveStoneItemId);
    //mockGameTimerSource_->set(GAME_TIMER->msec() + defaultLootingDelay);

    //playerGraveStoneController1_->acquireItemInGraveStone(lastGraveStoneId_, graveStoneItemId);

    //ASSERT_EQ(1, playerGraveStoneController1_->getCallCount("onAcquireItemInGraveStone"));
    //ASSERT_EQ(0, playerGraveStoneController2_->getCallCount("onAcquireItemInGraveStone"));
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

    //ASSERT_EQ(ecOk, playerGraveStoneController1_->lastErrorCode_);
}


TEST_F(ItemLootTest, testDuplicateAquireItemInGraveStone)
{
	/*  killMonster1();

	playerGraveStoneController1_->queryGraveStoneInsideInfo(lastGraveStoneId_);
	LootInvenItemInfoMap& info = playerGraveStoneController1_->lastGraveStoneInsideinfo_;

	ASSERT_TRUE(! info.graveStoneItemMap_.empty());

	LootInvenId graveStoneItemId = firstGraveStoneItemId;
	playerGraveStoneController1_->startLooting(lastGraveStoneId_, graveStoneItemId);
	mockGameTimerSource_->set(GAME_TIMER->msec() + defaultLootingDelay);

	playerGraveStoneController1_->acquireItemInGraveStone(lastGraveStoneId_, graveStoneItemId);

	ASSERT_EQ(1, playerGraveStoneController1_->getCallCount("onAcquireItemInGraveStone"));
	ASSERT_EQ(0, playerGraveStoneController2_->getCallCount("onAcquireItemInGraveStone"));
	ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));

	ASSERT_EQ(ecOk, playerGraveStoneController1_->lastErrorCode_);

	playerGraveStoneController1_->acquireItemInGraveStone(lastGraveStoneId_, graveStoneItemId);

	ASSERT_EQ(1 + 1, playerGraveStoneController1_->getCallCount("onAcquireItemInGraveStone"));
	ASSERT_EQ(0, playerGraveStoneController2_->getCallCount("onAcquireItemInGraveStone"));
	ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));

	ASSERT_EQ(ecItemNotExist, playerGraveStoneController1_->lastErrorCode_);
	}*/
}

TEST_F(ItemLootTest, testRemoveGraveStone)
{
    /*killMonster1();

    ASSERT_EQ(1, playerController1_->getCallCount("evEntityDisappeared"));

    playerGraveStoneController1_->queryGraveStoneInsideInfo(lastGraveStoneId_);
    LootInvenItemInfoMap& info = playerGraveStoneController1_->lastGraveStoneInsideinfo_;
    ASSERT_EQ(2, info.graveStoneItemMap_.size());

    const LootInvenId graveStoneItemId1 = firstGraveStoneItemId;
    const LootInvenId graveStoneItemId2 = toLootInvenId(firstGraveStoneItemId + 1);
    
    playerGraveStoneController1_->startLooting(lastGraveStoneId_, graveStoneItemId1);
    mockGameTimerSource_->set(GAME_TIMER->msec() + defaultLootingDelay);
    playerGraveStoneController1_->acquireItemInGraveStone(lastGraveStoneId_, graveStoneItemId1);

    playerGraveStoneController1_->startLooting(lastGraveStoneId_, graveStoneItemId2);
    mockGameTimerSource_->set(GAME_TIMER->msec() + defaultLootingDelay);
    playerGraveStoneController1_->acquireItemInGraveStone(lastGraveStoneId_, graveStoneItemId2);

    ASSERT_EQ(1 + 1, playerController1_->getCallCount("evEntityDisappeared"));
    ASSERT_EQ(1 + 1, playerController2_->getCallCount("evEntityDisappeared"));

    mockGameTimerSource_->set(GAME_TIMER->msec() + defaultLootingDelay);
    playerGraveStoneController1_->acquireItemInGraveStone(lastGraveStoneId_, graveStoneItemId2);

    ASSERT_EQ(ecGraveStoneNotFound, playerGraveStoneController1_->lastErrorCode_);*/
}


TEST_F(ItemLootTest, testSuccessPlayerGraveStone)
{
    //const ObjectId stackCount = 2;
    //const SlotId slotId = 15;
    //BaseItemInfo skillPackageItem(servertest::shabbyHelmatFragment, stackCount);

    //ASSERT_TRUE(playerInventoryController2_->addInventoryItem( skillPackageItem, slotId));
    //ASSERT_EQ(1, playerInventoryController2_->getCallCount("evInventoryItemAdded"));
    //ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

    //killPlayer2();
    //ASSERT_EQ(0, playerController1_->getCallCount("evEntityDisappeared"));
    //ASSERT_EQ(0, playerController2_->getCallCount("evEntityDisappeared"));

    //ASSERT_EQ(player2KillCount_, playerEffectController1_->getCallCount("evEffectApplied"));
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), 
    //    servertest::graveStoneSkillCode));

    //lastGraveStoneId_ = playerEffectController1_->lastGraveStoneInfo_.objectId_;

    //ASSERT_EQ(1, playerEffectController1_->getCallCount("evPlayerGraveStoneStood"));
    //ASSERT_EQ(1, playerEffectController2_->getCallCount("evPlayerGraveStoneStood"));
    //ASSERT_EQ(4, playerInventoryController2_->getCallCount("evInventoryItemRemoved"));


    ///// 기본아이템이라 드랍 안됨
    //const int forceUnequippedItemCount = 0;
    //ASSERT_EQ(forceUnequippedItemCount, playerInventoryController2_->getCallCount("evUnequipItemReplaced"));


    //playerGraveStoneController1_->startLooting(lastGraveStoneId_, invalidGraveStoneItemId);
    //ASSERT_EQ(ecGraveStoneNotMinHp, playerGraveStoneController1_->lastErrorCode_);
    //
    //while (true) {
    //    playerController1_->cheat(L"/fullpoints");
    //    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otGraveStone, lastGraveStoneId_), 
    //        servertest::testObserverSkillCode));
    //    playerController1_->selectTarget(GameObjectInfo(otGraveStone, lastGraveStoneId_));
    //    if (hpMin == playerController1_->lastTargetInfo_.getHitPoints().hp_) {
    //        break;
    //    }
    //}

    //playerGraveStoneController1_->queryGraveStoneInsideInfo(lastGraveStoneId_);
    //LootInvenItemInfoMap& info = playerGraveStoneController1_->lastGraveStoneInsideinfo_;
    //ASSERT_TRUE(0 < info.graveStoneItemMap_.size());

    //LootInvenId graveStoneItemId = invalidGraveStoneItemId;
    //LootInvenItemInfoMap& graveStoneItemMap = info.graveStoneItemMap_;
    //LootInvenItemInfoMap::const_iterator pos = graveStoneItemMap.begin();
    //for (; pos != graveStoneItemMap.end(); ++pos) {
    //    const BaseItemInfo testItem = (*pos).second;
    //    if (testItem.isEquipment()) {
    //        graveStoneItemId = (*pos).first;
    //        break;
    //    }
    //}
    //ASSERT_TRUE( graveStoneItemId != invalidGraveStoneItemId);

    //playerGraveStoneController1_->startLooting(lastGraveStoneId_, graveStoneItemId);
    //ASSERT_EQ(ecOk, playerGraveStoneController1_->lastErrorCode_);
    //mockGameTimerSource_->set(GAME_TIMER->msec() + defaultLootingDelay);
    //std::cout<<graveStoneItemId<<std::endl;

    //playerGraveStoneController1_->acquireItemInGraveStone(lastGraveStoneId_, graveStoneItemId);

    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryEquipItemAdded"));

    //const InventoryInfo emptyInven = player2_->getInventoryInfo();
    //for (SlotId i = forceUnequippedItemCount + 1; i <= defultInvenSlotId; ++i) {       
    //    ASSERT_TRUE(! emptyInven.isOccupiedSlot(i));
    //}

}