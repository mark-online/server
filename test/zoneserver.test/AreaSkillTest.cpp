#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockZoneUser.h"
#include "MockPlayerController.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerCastController.h"
#include "ZoneServer\user\ZoneUserManager.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/MockSkillTable.h>

using namespace gideon::zoneserver;

/**
* @class AreaSkillTest
*
* 에어리어(광역) 스킬 테스트
*/
class AreaSkillTest : public GameTestFixture
{
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();
/*
        areaSkillCode_ = servertest::defaultAreaAndCastee2SkillCode;
        areaSkillTemplate_ = SKILL_TABLE->getSkill(areaSkillCode_);
        ASSERT_TRUE(areaSkillTemplate_ != nullptr);

        targetAreaSkillCode_ = servertest::defaultTargetAreaSkillCode;
        targetAreaSkillTemplate_ = SKILL_TABLE->getSkill(targetAreaSkillCode_);
        ASSERT_TRUE(targetAreaSkillTemplate_ != nullptr);

        selfAreaSkillCode_ = servertest::defaultSelfAreaSkillCode;
        selfAreaSkillTemplate_ = SKILL_TABLE->getSkill(selfAreaSkillCode_);
        ASSERT_TRUE(selfAreaSkillTemplate_ != nullptr);

        accountId3_ = accountId2_ + 1;
        setValidAccount(accountId3_);
        characterId3_ = createCharacter(accountId3_);
        loginUser(accountId3_);

        user3_ = getMockZoneUser(accountId3_);
        user3_->selectCharacter(characterId3_);
        user3_->enterWorld();
        ASSERT_TRUE(user3_->isCharacterSelected());
        playerController3_ = &getMockPlayerController(*user3_);
        playerMoveController3_ = &getMockPlayerMoveController(*user3_);
        playerSkillController3_ = &getMockPlayerSkillController(*user3_);
        playerCastController3_ = &getMockPlayerCastController(*user3_);
        playerEffectController3_ = &getMockPlayerEffectController(*user3_);*/
        player3_ = &user3_->getActivePlayer();
        playerController3_->readyToPlay();
        ASSERT_EQ(1, playerController3_->getCallCount("onReadyToPlay"));

        ASSERT_EQ(3, ZONEUSER_MANAGER->getUserCount());
    }

protected:
    //SkillCode areaSkillCode_;
    //const datatable::SkillTemplate* areaSkillTemplate_;

    //SkillCode targetAreaSkillCode_;
    //const datatable::SkillTemplate* targetAreaSkillTemplate_;

    //SkillCode selfAreaSkillCode_;
    //const datatable::SkillTemplate* selfAreaSkillTemplate_;

    AccountId accountId3_;
    ObjectId characterId3_;
    MockZoneUser* user3_;
    zoneserver::go::Player* player3_;
    MockPlayerController* playerController3_;
    MockPlayerMoveController* playerMoveController3_;
    MockPlayerSkillController* playerSkillController3_;
    MockPlayerCastController* playerCastController3_;
    MockPlayerEffectController* playerEffectController3_;
};


TEST_F(AreaSkillTest, testAreaSkill)
{
    //go::Entity& creature2 = playerController2_->getOwner();
    //ObjectPosition position2 = creature2.getPosition();
    //position2.x_ += areaSkillTemplate_->getMinDistance();
    //playerMoveController2_->move(position2);

    //go::Entity& creature3 = playerController3_->getOwner();
    //ObjectPosition position3 = creature3.getPosition();
    //position3.x_ += areaSkillTemplate_->getMaxDistance() +
    //    areaSkillTemplate_->getEffectRangeValue() + 1;
    //playerMoveController3_->move(position3);

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);
    //ASSERT_EQ(player3_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);

    //playerCastController1_->startCasting(createUnionSkillCastAtInfo(player1_->getPosition(), areaSkillCode_));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //
    //// 즉시 시전이기때문에 패킷이 안온다.
    //ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    //ASSERT_EQ(0, playerCastController2_->getCallCount("evCasted"));
    //ASSERT_EQ(0, playerCastController3_->getCallCount("evCasted"));
    //
    //ASSERT_EQ(4, playerEffectController1_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(1, playerEffectController2_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(0, playerEffectController3_->getCallCount("evEffectApplied"));

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(playerHp_ -
    //        areaSkillTemplate_->getSkillInfo()->castee_effect_value_1() -
    //        areaSkillTemplate_->getSkillInfo()->castee_effect_value_2()));
    //ASSERT_EQ(player3_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //  playerHp_);
}


TEST_F(AreaSkillTest, testTargetAreaSkill)
{
    //const ObjectPosition position1 = playerController1_->getOwner().getPosition();

    //playerMoveController2_->move(position1);

    //ObjectPosition position3 = position1;
    //position3.x_ += targetAreaSkillTemplate_->getEffectRangeValue() + 1;
    //playerMoveController3_->move(position3);

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);
    //ASSERT_EQ(player3_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);

    //playerCastController1_->startCasting(
    //    createUnionSkillCastToInfo(player2_->getGameObjectInfo(), targetAreaSkillCode_));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //
    //// 즉시 시전이기때문에 패킷이 안온다. 
    //ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    //ASSERT_EQ(0, playerCastController2_->getCallCount("evCasted"));
    //ASSERT_EQ(0, playerCastController3_->getCallCount("evCasted"));

    //ASSERT_EQ(4, playerEffectController1_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(1, playerEffectController2_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(0, playerEffectController3_->getCallCount("evEffectApplied"));

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(playerHp_ - targetAreaSkillTemplate_->getSkillInfo()->castee_effect_value_1()));
    //ASSERT_EQ(player3_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);
}


TEST_F(AreaSkillTest, testSelfAreaSkill)
{
    //const ObjectPosition position1 = playerController1_->getOwner().getPosition();

    //playerMoveController2_->move(position1);

    //ObjectPosition position3 = position1;
    //position3.x_ += selfAreaSkillTemplate_->getEffectRangeValue() + 1;
    //playerMoveController3_->move(position3);

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);
    //ASSERT_EQ(player3_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);

    //playerCastController1_->startCasting(
    //    createUnionSkillCastToInfo(player1_->getGameObjectInfo(), selfAreaSkillCode_));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));

    //// 즉시 시전이기때문에 패킷이 안온다.
    //ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    //ASSERT_EQ(0, playerCastController2_->getCallCount("evCasted"));
    //ASSERT_EQ(0, playerCastController3_->getCallCount("evCasted"));

    //ASSERT_EQ(4, playerEffectController1_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(1, playerEffectController2_->getCallCount("evEffectApplied"));
    //ASSERT_EQ(0, playerEffectController3_->getCallCount("evEffectApplied"));

    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(playerHp_ - selfAreaSkillTemplate_->getSkillInfo()->castee_effect_value_1()));
    //ASSERT_EQ(player3_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    playerHp_);
}
