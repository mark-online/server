#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerAnchorController.h"
#include "MockPlayerController.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/service/anchor/AnchorService.h"
#include "ZoneServer/service/skill/Skill.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/servertest/datatable/DataCodes.h>

const GameTime defaultLootingDelay = 5000;

/**
* @class AnchorTest
*
* 설치물 관련 테스트
*/
class AnchorTest : public GameTestFixture
{
    virtual void SetUp() {
        GameTestFixture::SetUp();

        const uint8_t stackCount1 = 1;
        const SlotId slotId1 = 17;
        BaseItemInfo anchorItem(servertest::shabbyAncorCode, stackCount1);

        const uint8_t stackCount2 = 100;
        const SlotId slotId2 = 18;
        BaseItemInfo buildMaterialItem(servertest::normalNpcElementCode1, stackCount2);


        ASSERT_TRUE(playerInventoryController1_->addInventoryItem(anchorItem, slotId1));
        anchorId_ = playerInventoryController1_->lastAddItemId_;
        ASSERT_TRUE(playerInventoryController1_->addInventoryItem(buildMaterialItem, slotId2));
        buildMeterialId_ = playerInventoryController1_->lastAddItemId_;
    }
protected:
    ObjectId anchorId_;
    ObjectId buildMeterialId_;
};



TEST_F(AnchorTest, testCreateAnchor)
{
    ASSERT_EQ(1, getMockPlayerController(*user1_).getCallCount("evEntityAppeared"));
    ASSERT_EQ(0, getMockPlayerController(*user2_).getCallCount("evEntityAppeared"));

    const ObjectPosition position1 = playerController1_->getOwner().getPosition();
    
    playerAnchorController1_->createAnchor(anchorId_, position1);
    ASSERT_EQ(0, playerAnchorController1_->getCallCount("onCreateAnchor"));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(1 + 1, getMockPlayerController(*user1_).getCallCount("evEntityAppeared"));
    ASSERT_EQ(1, getMockPlayerController(*user2_).getCallCount("evEntityAppeared"));
}


TEST_F(AnchorTest, testBuildAnchor)
{
    //getCommunityServerProxy().z2m_evPartyMemberJoined(1, player1_->getObjectId());
    //ASSERT_TRUE(nullptr != player1_->queryPartyable()->getParty());
    //getCommunityServerProxy().z2m_evPartyMemberJoined(1, player2_->getObjectId());
    //ASSERT_TRUE(nullptr != player1_->queryPartyable()->getParty());
    //const ObjectPosition position1 = playerController1_->getOwner().getPosition();

    //playerAnchorController1_->createAnchor(anchorId_, position1);
    //ASSERT_EQ(ecOk, playerAnchorController1_->lastErrorCode_);
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    //playerAnchorController1_->putfuelItem(GameObjectInfo(otAnchor, 1), buildMeterialId_, 1);
    //ASSERT_EQ(ecOk, playerAnchorController1_->lastErrorCode_);
    //ASSERT_EQ(1 + 1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));

    //const UnionEntityInfo& entity = playerController1_->lastAppearedEntity_;
    //const MoreAnchorInfo& info = entity.asAnchorInfo();
    //ASSERT_EQ(otAnchor, entity.objectType_);
    //playerAnchorController1_->buildAnchor(GameObjectInfo(otAnchor, info.objectId_));
    //ASSERT_EQ(ecOk, playerAnchorController1_->lastErrorCode_);
    //ASSERT_EQ(1, playerAnchorController1_->getCallCount("onBuildAnchor"));
    //ASSERT_EQ(1, playerAnchorController1_->getCallCount("evAnchorStartBuilt"));
    //ASSERT_EQ(1, playerAnchorController1_->getCallCount("evAnchorStartBuilt"));
    //playerAnchorController2_->buildAnchor(GameObjectInfo(otAnchor, playerAnchorController1_->lastAnchorId_));
    //ASSERT_EQ(playerAnchorController1_->lastOwnerId_, playerAnchorController2_->lastOwnerId_);
}


TEST_F(AnchorTest, testAutoSkillAnchor)
{
    //playerAnchorController1_->createAnchor(anchorId_, position1);
    //ASSERT_EQ(0, playerAnchorController1_->getCallCount("onCreateAnchor"));
}


TEST_F(AnchorTest, testBreakAnchor)
{
    //const ObjectPosition position1 = playerController1_->getOwner().getPosition();
    //playerAnchorController1_->createAnchor(anchorId_, position1);
    //const UnionEntityInfo& entity = playerController1_->lastAppearedEntity_;
    //const MoreAnchorInfo& info = entity.asAnchorInfo();
    //GameObjectInfo objectInfo(otAnchor, info.objectId_);
    //playerAnchorController1_->putfuelItem(objectInfo, buildMeterialId_, 1);

    //SkillCode skillCode = servertest::defaultMeleeAttackSkillCode;
    //for (;;) {
    //    playerCastController1_->startCasting(objectInfo, skillCode);

    //    const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(skillCode);
    //    ASSERT_TRUE(skillTemplate != nullptr);
    //    mockGameTimerSource_->set(GAME_TIMER->msec() +
    //        skillTemplate->getSkillInfo()->cool_time() + 1);

    //    if (playerAnchorController1_->getCallCount("evAnchorBroken") > 0) {
    //        break;
    //    }
    //    // FYI: MockTaskScheduler에 의해 부활되어 버리므로 아래 코드를 사용하면 안됨!
    //    //if (monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.isDied()) {
    //    //    break;
    //    //}
    //}
}

TEST_F(AnchorTest, testBreakAnchorAfterBuild)
{
    //const ObjectPosition position1 = playerController1_->getOwner().getPosition();
    //playerAnchorController1_->createAnchor(anchorId_, position1);
    //const UnionEntityInfo& entity = playerController1_->lastAppearedEntity_;
    //const MoreAnchorInfo& info = entity.asAnchorInfo();
    //    playerAnchorController1_->putfuelItem(GameObjectInfo(otAnchor, info.objectId_), buildMeterialId_, 1);
    //playerAnchorController1_->buildAnchor(GameObjectInfo(otAnchor, info.objectId_));

    //GameObjectInfo objectInfo(otAnchor, info.objectId_);
    //
    //SkillCode skillCode = servertest::defaultMeleeAttackSkillCode;
    //for (;;) {
    //    playerCastController1_->startCasting(objectInfo, skillCode);

    //    const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(skillCode);
    //    ASSERT_TRUE(skillTemplate != nullptr);
    //    mockGameTimerSource_->set(GAME_TIMER->msec() +
    //        skillTemplate->getSkillInfo()->cool_time() + 1);

    //    if (playerAnchorController1_->getCallCount("evAnchorBroken") > 0) {
    //        break;
    //    }
    //    // FYI: MockTaskScheduler에 의해 부활되어 버리므로 아래 코드를 사용하면 안됨!
    //    //if (monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.isDied()) {
    //    //    break;
    //    //}
    //}

}