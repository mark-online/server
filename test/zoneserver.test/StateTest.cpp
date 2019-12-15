#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerMoveController.h"
#include "ZoneServer/model/gameobject/skilleffect/CreatureEffectScriptApplier.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/MockSkillTable.h>

/**
* @class StateTest
*
* 상태 테스트
*/
class StateTest : public GameTestFixture
{ 
};


TEST_F(StateTest, testKillState)
{
    killPlayer2();

    playerMoveController2_->move(ObjectPosition(1.0, 1.0, 0.0, toHeading(40)));

    ASSERT_EQ(0, playerMoveController1_->getCallCount("evEntityMoved"));
    
    //playerCastController2_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_),
    //    servertest::defaultMeleeAttackSkillCode));
    ASSERT_EQ(1, playerCastController2_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecStateCannotSkillCastState, playerCastController2_->lastErrorCode_);
}


TEST_F(StateTest, testMoveState)
{
    //playerMoveController2_->goToward(ObjectPosition(1.0, 1.0, 0.0, toHeading(40)));

    //playerCastController2_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_),
    //    servertest::defaultMeleeAttackSkillCode));
    //ASSERT_EQ(1, playerCastController2_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillCannotMoveSkill, playerCastController2_->lastErrorCode_);

    //playerMoveController2_->stop(ObjectPosition(1.0, 1.0, 0.0, toHeading(40)));

    //playerCastController2_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_),
    //    servertest::defaultMeleeAttackSkillCode));
    //ASSERT_EQ(1 + 1, playerCastController2_->getCallCount("onStartCasting"));

}


TEST_F(StateTest, testStunState)
{
 //   playerCastController2_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_),
 //       servertest::stunSkillCode));
 //   ASSERT_EQ(1, playerCastController2_->getCallCount("onStartCasting"));

 //   // 스턴 상태
 //   playerMoveController1_->move(ObjectPosition(1.0, 1.0, 0.0, toHeading(40)));
 //   ASSERT_EQ(0, playerMoveController2_->getCallCount("evEntityMoved"));
	//ASSERT_EQ(1, playerEffectController1_->getCallCount("evMesmerizationEffected"));
	//ASSERT_EQ(1, playerEffectController2_->getCallCount("evMesmerizationEffected"));

 //   const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(servertest::stunSkillCode);
 //   const datatable::BaseEffectInfo& casteeInfo = skillTemplate->getCasteeEffectInfo(eanEffect_1);
 //   go::EffectScriptApplier& applier = player1_->getEffectScriptApplier();
 //   mockGameTimerSource_->set(GAME_TIMER->msec() + casteeInfo.activateTime_);
 //   applier.tick();

 //   // 해제
 //   playerMoveController1_->move(ObjectPosition(1.0, 1.0, 0.0, toHeading(40)));
 //   ASSERT_EQ(1, playerMoveController2_->getCallCount("evEntityMoved"));
	//ASSERT_EQ(1 + 1, playerEffectController1_->getCallCount("evMesmerizationEffected"));
	//ASSERT_EQ(1 + 1, playerEffectController2_->getCallCount("evMesmerizationEffected"));
}