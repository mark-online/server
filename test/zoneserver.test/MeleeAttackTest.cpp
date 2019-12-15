#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerMoveController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/service/skill/Skill.h"
#include <gideon/servertest/datatable/MockPlayerActiveSkillTable.h>

using namespace gideon::zoneserver;

/**
* @class MeleeAttackTest
*
* 근접 물리 공격 테스트
*/
class MeleeAttackTest : public GameTestFixture
{
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();

        skillCode_ = servertest::defaultMeleeAttackSkillCode;
        skillTemplate_ = PLAYER_ACTIVE_SKILL_TABLE->getPlayerSkill(skillCode_);
        ASSERT_TRUE(skillTemplate_ != nullptr);
    }

protected:
    SkillCode skillCode_;
    const datatable::PlayerActiveSkillTemplate* skillTemplate_;
};


TEST_F(MeleeAttackTest, testEffectInfoed)
{
    ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
        playerHp_);

    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), skillCode_));
    ASSERT_EQ(1, playerEffectController1_->getCallCount("evEffectApplied"));
    ASSERT_EQ(1, playerEffectController2_->getCallCount("evEffectApplied"));

    ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
        toHitPoint(playerHp_ - skillTemplate_->casteeEffectValues_[0]));
}


TEST_F(MeleeAttackTest, testTargetIsTooFarIn2D)
{
    ObjectPosition position = player1_->getPosition();
    position.x_ += skillTemplate_->skillCastCheckDistanceInfo_.maxDistance_ + 1.0f;
    playerMoveController1_->move(position);

    playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), skillCode_));
    ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecSkillTargetTooFar, playerCastController1_->lastErrorCode_);
}


// TODO: 나중에 해결할 것!
//TEST_F(MeleeAttackTest, testTargetIsTooFarIn3D)
//{
//    ObjectPosition position = player1_->getPosition();
//    position.z_ += Skill::meleeAttackAllowedMaxHeight + 1;
//    playerController1_->move(position);
//
//    playerController1_->castSkillTo(GameObjectInfo(otPc, characterId2_), skillCode_);
//    ASSERT_EQ(1, playerController1_->getCallCount("onStartCasting"));
//    ASSERT_EQ(ecSkillTargetTooFar, playerController1_->lastErrorCode_);
//}
