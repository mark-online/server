#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "ZoneServer/world/WorldMap.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
//#include "MockPlayerSkillController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/MockSkillTable.h>
#undef NOMINMAX
#pragma warning (disable: 4267)
#include <gideon/3d/3d.h>
#include <gideon/servertest/datatable/DataCodes.h>


using namespace gideon::zoneserver;

/**
* @class SkillTest
*
* 스킬 테스트
*/
class SkillTest : public GameTestFixture
{
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();

        //validSkillCode_ = servertest::defaultMeleeAttackSkillCode;

        //skillTemplate_ = SKILL_TABLE->getSkill(validSkillCode_);
        //ASSERT_TRUE(skillTemplate_ != nullptr);
    }

protected:
    SkillCode validSkillCode_;
    //const datatable::SkillTemplate* skillTemplate_;
};


TEST_F(SkillTest, testCastSkillNotLearned)
{
    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), invalidSkillCode));
    ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(0, playerCastController2_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecSkillNotLearned, playerCastController1_->lastErrorCode_);
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    ASSERT_EQ(0, playerCastController2_->getCallCount("evCasted"));
}


TEST_F(SkillTest, testCastSkillToInvalidTarget)
{
    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, invalidObjectId), validSkillCode_));
    ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(0, playerCastController2_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecSkillTargetNotFound, playerCastController1_->lastErrorCode_);
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    ASSERT_EQ(0, playerCastController2_->getCallCount("evCasted"));
}


TEST_F(SkillTest, testCastSkillTo)
{
    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), validSkillCode_));
    ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    
    // 즉시 시전일 경우 안온다
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    ASSERT_EQ(0, playerCastController2_->getCallCount("evCasted"));
}


TEST_F(SkillTest, testDie)
{
    killPlayer2();

    ASSERT_EQ(1, playerController1_->getCallCount("evPlayerDied"));
    ASSERT_EQ(1, playerController2_->getCallCount("evPlayerDied"));

    ASSERT_EQ(0, playerController1_->getCallCount("evEntityDisappeared"));
    ASSERT_EQ(0, playerController2_->getCallCount("evEntityDisappeared"));

    go::Creature& creature2 = playerController2_->getOwnerAs<go::Creature>();
    ASSERT_TRUE(creature2.getCurrentWorldMap()->getEntity(
        creature2.getGameObjectInfo()) != nullptr);
}


TEST_F(SkillTest, testCooldownTime)
{
    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), validSkillCode_));
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));

    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), validSkillCode_));
    ASSERT_EQ(1 + 1, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecSkillTooShortInterval, playerCastController1_->lastErrorCode_);
    
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));

    //mockGameTimerSource_->set(GAME_TIMER->msec() +
    //    skillTemplate_->getSkillInfo()->cool_time());

    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), validSkillCode_));
    ASSERT_EQ(1 + 2, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
}


TEST_F(SkillTest, testGlobalCooldownTime)
{
    //const SkillCode longGlobalCooldownTimeSkillCode =
    //    servertest::defaultLongGlobalCooldownTimeSkillCode;
    //const datatable::SkillTemplate* longGlobalCooldownTimeSkillTemplate =
    //    SKILL_TABLE->getSkill(longGlobalCooldownTimeSkillCode);
    //ASSERT_TRUE(longGlobalCooldownTimeSkillTemplate != nullptr);

    //ASSERT_TRUE(longGlobalCooldownTimeSkillTemplate->getSkillInfo()->cool_time()
    //    < longGlobalCooldownTimeSkillTemplate->getSkillInfo()->global_cooldown_time());

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    longGlobalCooldownTimeSkillCode));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(0, playerCastController1_->getCallCount("evSkillCastedTo"));

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    longGlobalCooldownTimeSkillCode));
    //ASSERT_EQ(1 + 1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillTooShortInterval, playerCastController1_->lastErrorCode_);
    //ASSERT_EQ(0, playerCastController1_->getCallCount("evSkillCastedTo"));

    //mockGameTimerSource_->set(GAME_TIMER->msec() +
    //    longGlobalCooldownTimeSkillTemplate->getSkillInfo()->global_cooldown_time());

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    longGlobalCooldownTimeSkillCode));
    //ASSERT_EQ(1 + 2, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(0, playerCastController1_->getCallCount("evSkillCastedTo"));
}


TEST_F(SkillTest, testCastTime)
{
//    const SkillCode longCastTimeSkillCode =
//        servertest::defaultLongCastTimeSkillCode;
//    const datatable::SkillTemplate* longCastTimeSkillTemplate =
//        SKILL_TABLE->getSkill(longCastTimeSkillCode);
//    ASSERT_TRUE(longCastTimeSkillTemplate != nullptr);
//
//    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
//        longCastTimeSkillCode));
//    ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
//    // 캐스팅 시간이 있다.
//    ASSERT_EQ(1, playerCastController1_->getCallCount("evCasted"));

    // TODO: MockTaskScheduler를 개선해야 한다

    //mockGameTimerSource_->set(GAME_TIMER->msec() +
    //    longCastTimeSkillTemplate->getInfo().cast_time());

    //playerController1_->castSkillTo(GameObjectInfo(otPc, characterId2_),
    //    longCastTimeSkillCode);
    //ASSERT_EQ(0, playerController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(1, playerController1_->getCallCount("evSkillCastedTo"));
}


// 시전 중인 스킬을 취소한다
TEST_F(SkillTest, testCancelCastingSkill)
{
    // TODO: MockTaskScheduler를 개선해야 한다
}


// 스킬 시전 중에 (다른) 스킬 시전 요청이 오면 기존 스킬을 취소한다
TEST_F(SkillTest, testCancelPreviousCastingSkillIfNewCastingIsRequested)
{
    // TODO: MockTaskScheduler를 개선해야 한다
}


TEST_F(SkillTest, testConsumeHpMpCp)
{
    //const Points prevPoints = player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_;

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), validSkillCode_));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(0, playerCastController1_->getCallCount("evSkillCastedTo"));
    //ASSERT_EQ(0, playerCastController2_->getCallCount("evSkillCastedTo"));
    ////ASSERT_EQ(1, playerController1_->getCallCount("evTargetSkillCastingCompleted"));
    ////ASSERT_EQ(1, playerController2_->getCallCount("evTargetSkillCastingCompleted"));

    //ASSERT_EQ(player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(prevPoints.hp_ - skillTemplate_->getSkillInfo()->consumed_hp()));
    //ASSERT_EQ(player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.mp_,
    //    toHitPoint(prevPoints.mp_ - skillTemplate_->getSkillInfo()->consumed_mp()));
    // MockTaskScheduler 때문에 CP가 가득 차버림
    //ASSERT_TRUE(player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.cp_ <
    //    prevPoints.cp_);
}


TEST_F(SkillTest, testCpRecovery)
{
    // TODO: MockTaskScheduler를 개선해야 한다

    const Points prevPoints = player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_;

    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), validSkillCode_));
    ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(0, playerCastController1_->getCallCount("evSkillCastedTo"));

    // MockTaskScheduler 때문에 CP가 가득 차버림
    //ASSERT_EQ(player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.cp_,
    //    toHitPoint(prevPoints.cp_ - skillTemplate_->getInfo().consumed_cp() +
    //        GIDEON_PROPERTY_TABLE->getPropertyValue<uint16_t>(L"cp_recovery_value")));
}


TEST_F(SkillTest, testChangeCharacterState)
{
    //playerMoveController1_->move(ObjectPosition(1.0, 1.0, 0.0, toHeading(190)));
    //playerMoveController2_->move(ObjectPosition(-1.0, -1.0, 0.0, toHeading(40)));
    //// 평화 상태
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    servertest::combatUseTypeSkillCode));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillNotUseableState, playerCastController1_->lastErrorCode_);

    //// 평화 -> 전투 상태로 바뀜
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    servertest::peaceUseTypeSkillCode));
    //ASSERT_EQ(1 +1, playerCastController1_->getCallCount("onStartCasting"));


    //// 전투 상태일때 만 쓸수 있는 스킬

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    servertest::combatUseTypeSkillCode));

    //ASSERT_EQ(1 + 2 , playerCastController1_->getCallCount("onStartCasting"));

    //// 평화로 바뀜
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    servertest::changePeaceSkillCode));

    //ASSERT_EQ(1 + 3, playerCastController1_->getCallCount("onStartCasting"));

    //// 사용 불가
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    //    servertest::combatUseTypeSkillCode));

    //ASSERT_EQ(1 + 4, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillNotUseableState, playerCastController1_->lastErrorCode_);

}


TEST_F(SkillTest, testAllowedEquipType)
{
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_),
    //    servertest::absolutenessDefence));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));

    //const InventoryInfo inven = player1_->queryInventoryable()->getInventoryInfo();
    //const ItemInfo* shield = getItemInfo(inven,
    //    servertest::defaultShieldEquipCode);
    //ASSERT_TRUE(shield != nullptr);

    //playerInventoryController1_->unequipItem(shield->itemId_, invalidSlotId);
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_),
    //    servertest::absolutenessDefence));

    //ASSERT_EQ(1 + 1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillNotAllowedEquipment, playerCastController1_->lastErrorCode_);
}


TEST_F(SkillTest, testDirection)
{
    /* playerMoveController1_->move(ObjectPosition(1.0, 1.0, 0.0, toHeading(40)));
    playerMoveController2_->move(ObjectPosition(-1.0, -1.0, 0.0, toHeading(40)));
    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    servertest::peaceUseTypeSkillCode));


    const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(validSkillCode_);
    mockGameTimerSource_->set(GAME_TIMER->msec() +
    skillTemplate ->getSkillInfo()->global_cooldown_time());

    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    servertest::combatUseTypeSkillCode));
    ASSERT_EQ(1 + 1, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecSkillInvalidTargetDirection, playerCastController1_->lastErrorCode_);

    playerMoveController1_->move(ObjectPosition(1.0, 1.0, 0.0, toHeading(190)));
    playerMoveController2_->move(ObjectPosition(-1.0, -1.0, 0.0, toHeading(190)));
    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    servertest::combatUseTypeSkillCode));
    ASSERT_EQ(1 + 2, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecSkillInvalidTargetOrientation, playerCastController1_->lastErrorCode_);

    playerMoveController1_->move(ObjectPosition(1.0, 1.0, 0.0, toHeading(190)));
    playerMoveController2_->move(ObjectPosition(-1.0, -1.0, 0.0, toHeading(40)));
    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_),
    servertest::combatUseTypeSkillCode));
    ASSERT_EQ(1 + 3, playerCastController1_->getCallCount("onStartCasting"));*/


}