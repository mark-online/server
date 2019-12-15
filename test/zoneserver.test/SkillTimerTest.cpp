#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "ZoneServer/model/gameobject/skilleffect/CreatureEffectScriptApplier.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/MockSkillTable.h>
#undef NOMINMAX
#pragma warning (disable: 4267)
#include <gideon/3d/3d.h>

using namespace gideon::zoneserver;

/**
* @class SkillTimerTest
*
* 스킬 타이머 테스트
*/
class SkillTimerTest : public GameTestFixture
{
};


TEST_F(SkillTimerTest, testAbsolutenessDefence)
{
    //ASSERT_EQ(0, playerController1_->getCallCount("evAttributeRateChanged"));
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_),
    //    servertest::absolutenessDefence));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(1, playerController1_->getCallCount("evAttributeRateChanged"));

    //const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(servertest::absolutenessDefence);
    //const datatable::BaseEffectInfo& casteeInfo = skillTemplate->getCasteeEffectInfo(eanEffect_1);
    //go::EffectScriptApplier& applier = player1_->getEffectScriptApplier();
    //mockGameTimerSource_->set(GAME_TIMER->msec() + casteeInfo.activateTime_);
    //applier.tick();
    //ASSERT_EQ(1 + 1, playerController1_->getCallCount("evAttributeRateChanged"));
    //mockGameTimerSource_->set(GAME_TIMER->msec() + casteeInfo.activateTime_);
    //applier.tick();
    //ASSERT_EQ(1 + 1, playerController1_->getCallCount("evAttributeRateChanged"));
}
