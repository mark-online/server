#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerMoveController.h"
#include "ZoneServer/service/skill/Skill.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/MockSkillTable.h>

using namespace gideon::zoneserver;

/**
* @class RecoveryTest
*
* 회복 테스트
*/
class RecoveryTest : public GameTestFixture
{
protected:
    void damagePlayer2();
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();

        //hpSkillCode_ = servertest::hpRecoverySkillCode;
        //hpSkillTemplate_ = SKILL_TABLE->getSkill(hpSkillCode_);
        //ASSERT_TRUE(hpSkillTemplate_ != nullptr);

        //mpSkillCode_ = servertest::mpRecoverySkillCode;
        //mpSkillTemplate_ = SKILL_TABLE->getSkill(mpSkillCode_);
        //ASSERT_TRUE(mpSkillTemplate_ != nullptr);

        //attackSkillCode_ = servertest::defaultMagicAttackSkillCode;
        //attackSkillTemplate_ = SKILL_TABLE->getSkill(attackSkillCode_);
        //ASSERT_TRUE(attackSkillTemplate_ != nullptr);

        //go::Entity& creature2 = playerController2_->getOwner();
        //ObjectPosition position = creature2.getPosition();
        //position.x_ += attackSkillTemplate_->getMinDistance();
        //playerMoveController1_->move(position);
    }

protected:
    SkillCode hpSkillCode_;
    SkillCode mpSkillCode_;
    SkillCode attackSkillCode_;
    //const datatable::SkillTemplate* hpSkillTemplate_;
    //const datatable::SkillTemplate* mpSkillTemplate_;
    //const datatable::SkillTemplate* attackSkillTemplate_;
};

void RecoveryTest::damagePlayer2()
{
    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), attackSkillCode_));
    //ASSERT_EQ(1, playerEffectController1_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(1, playerEffectController2_->getCallCount("evEffectApplied"));

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(playerHp_ - attackSkillTemplate_->getSkillInfo()->castee_effect_value_1()));
}



TEST_F(RecoveryTest, testHpRecoverySkill)
{
    damagePlayer2();
    //HitPoint myHp = toHitPoint(playerHp_ - attackSkillTemplate_->getSkillInfo()->castee_effect_value_1());

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), hpSkillCode_));

    //ASSERT_EQ(2, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(0, playerCastController2_->getCallCount("onStartCasting"));

    //ASSERT_EQ(1 + 1, playerEffectController1_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(1 + 1, playerEffectController2_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(myHp + hpSkillTemplate_->getSkillInfo()->castee_effect_value_1()));
}


TEST_F(RecoveryTest, testMpRecoverySkill)
{
    damagePlayer2();
    //ASSERT_EQ(1, playerEffectController1_->getCallCount("evEffectApplied"));
    //ManaPoint myMp = toManaPoint(playerMp_ - attackSkillTemplate_->getSkillInfo()->consumed_mp());

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId1_), mpSkillCode_));

    //ASSERT_EQ(2, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(0, playerCastController2_->getCallCount("onStartCasting"));

    //ASSERT_EQ(1 + 1, playerEffectController1_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(1, playerEffectController2_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.mp_,
    //    toManaPoint(myMp + mpSkillTemplate_->getSkillInfo()->castee_effect_value_1()));
}