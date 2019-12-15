#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerMoveController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/service/skill/Skill.h"
#include <gideon/servertest/datatable/MockSkillTable.h>

using namespace gideon::zoneserver;

/**
* @class MagicAttackTest
*
* 원거리 마법 공격 테스트
*/
class MagicAttackTest : public GameTestFixture
{
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();

        //skillCode_ = servertest::defaultMagicAttackSkillCode;
        //skillTemplate_ = SKILL_TABLE->getSkill(skillCode_);
        //ASSERT_TRUE(skillTemplate_ != nullptr);

        //go::Entity& creature2 = playerController2_->getOwner();
        //ObjectPosition position = creature2.getPosition();
        //position.x_ += skillTemplate_->getMinDistance();
        //playerMoveController2_->move(position);
    }

protected:
    SkillCode skillCode_;
//    const datatable::SkillTemplate* skillTemplate_;
};


TEST_F(MagicAttackTest, testEffectInfoed)
{
    //ObjectPosition position = player2_->getPosition();
    //position.y_ = skillTemplate_->getSkillCommonInfo().minDistance_ + 1;
    //playerMoveController1_->move(position);
    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), skillCode_));
    //ASSERT_EQ(1, playerEffectController1_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(1, playerEffectController2_->getCallCount("evEffectApplied"));

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(playerHp_ - skillTemplate_->getSkillInfo()->castee_effect_value_1()));
}


TEST_F(MagicAttackTest, testTargetIsTooNearIn2D)
{
    //const ObjectPosition position = player2_->getPosition();
    //playerMoveController1_->move(position);

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), skillCode_));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillTargetTooNear, playerCastController1_->lastErrorCode_);
}


TEST_F(MagicAttackTest, testTargetIsTooFarIn2D)
{
    //ObjectPosition position = player2_->getPosition();
    //position.x_ += skillTemplate_->getMaxDistance() + 1.0f;
    //playerMoveController1_->move(position);

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), skillCode_));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillTargetTooFar, playerCastController1_->lastErrorCode_);
}

// TODO: 나중에 해결할 것!
//TEST_F(MagicAttackTest, testTargetIsTooFarIn3D)
//{
//    ObjectPosition position = player2_->getPosition();
//    position.z_ += Skill::rangeAttackAllowedMaxHeight + 1;
//    playerController1_->move(position);
//
//    playerController1_->castSkillTo(GameObjectInfo(otPc, characterId2_), skillCode_);
//    ASSERT_EQ(1, playerController1_->getCallCount("onStartCasting"));
//    ASSERT_EQ(ecSkillTargetTooFar, playerController1_->lastErrorCode_);
//}
