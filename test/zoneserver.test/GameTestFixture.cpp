#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockZoneUser.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "MockGameTimer.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include <gideon/cs/shared/data/CastInfo.h>
#include <gideon/servertest/datatable/MockSkillTable.h>
#include <gideon/cs/datatable/SkillTemplate.h>

using namespace gideon::zoneserver;

void GameTestFixture::SetUp()
{
    ZoneServiceTestFixture::SetUp();

    player2KillCount_ = 0;
    const int pointTodefaultHelmetEquip = 10;

    playerMp_ = toManaPoint(mpDefault + pointTodefaultHelmetEquip);
    characterId1_ = createCharacter(accountId1_);
    loginUser(accountId1_);

    user1_ = getMockZoneUser(accountId1_);
    user1_->selectCharacter(characterId1_);
    user1_->enterWorld();
    ASSERT_TRUE(user1_->isCharacterSelected());
    playerController1_ = &getMockPlayerController(*user1_);
    playerMoveController1_ = &getMockPlayerMoveController(*user1_);
    playerSkillController1_ = &getMockPlayerSkillController(*user1_);
    playerInventoryController1_ = &getMockPlayerInventoryController(*user1_);
    playerQuestController1_ = &getMockPlayerQuestController(*user1_);
    playerTradeController1_ = &getMockPlayerTradeController(*user1_);
    playerHarvestController1_ = &getMockPlayerHarvestController(*user1_);
    playerTreasureController1_ = &getMockPlayerTreasureController(*user1_);
    playerGraveStoneController1_ = &getMockPlayerGraveStoneController(*user1_);
    playerItemController1_ = &getMockPlayerItemController(*user1_);
    playerPartyController1_ = &getMockPlayerPartyController(*user1_);
    playerAnchorController1_ = &getMockPlayerAnchorController(*user1_);
    playerNpcController1_ = &getMockPlayerNpcController(*user1_);
    playerCastController1_ = &getMockPlayerCastController(*user1_);
    playerEffectController1_ = &getMockPlayerEffectController(*user1_);
    playerMailController1_ = &getMockPlayerMailController(*user1_);
    playerAuctionController1_ = &getMockPlayerAuctionController(*user1_);
    playerTeleportController1_ = &getMockPlayerTeleportController(*user1_);


    player1_ = &user1_->getActivePlayer();
    playerController1_->readyToPlay();
    ASSERT_EQ(1, playerController1_->getCallCount("onReadyToPlay"));
    playerHp_ = player1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_;

    accountId2_ = accountId1_ + 1;
    setValidAccount(accountId2_);
    characterId2_ = createCharacter(accountId2_);
    loginUser(accountId2_);

    user2_ = getMockZoneUser(accountId2_);
    user2_->selectCharacter(characterId2_);
    user2_->enterWorld();
    ASSERT_TRUE(user2_->isCharacterSelected());
    playerController2_ = &getMockPlayerController(*user2_);
    playerMoveController2_ = &getMockPlayerMoveController(*user2_);
    playerSkillController2_ = &getMockPlayerSkillController(*user2_);
    playerInventoryController2_ = &getMockPlayerInventoryController(*user2_);
    playerQuestController2_ = &getMockPlayerQuestController(*user2_);
    playerTradeController2_ = &getMockPlayerTradeController(*user2_);
    playerHarvestController2_ = &getMockPlayerHarvestController(*user2_);
    playerTreasureController2_ = &getMockPlayerTreasureController(*user2_);
    playerGraveStoneController2_ = &getMockPlayerGraveStoneController(*user2_);
    playerItemController2_ = &getMockPlayerItemController(*user2_);
    playerPartyController2_ = &getMockPlayerPartyController(*user2_);
    playerAnchorController2_ = &getMockPlayerAnchorController(*user2_);
    playerNpcController2_ = &getMockPlayerNpcController(*user2_);
    playerCastController2_ = &getMockPlayerCastController(*user2_);
    playerEffectController2_ = &getMockPlayerEffectController(*user2_);
    playerMailController2_ = &getMockPlayerMailController(*user2_);
    playerAuctionController2_ = &getMockPlayerAuctionController(*user2_);
    playerTeleportController2_ = &getMockPlayerTeleportController(*user2_);

    player2_ = &user2_->getActivePlayer();
    playerController2_->readyToPlay();
    ASSERT_EQ(1, playerController2_->getCallCount("onReadyToPlay"));

    ASSERT_EQ(2, ZONEUSER_MANAGER->getUserCount());
}


void GameTestFixture::killPlayer2()
{
    //go::Creature& creature2 = playerController2_->getOwnerAs<go::Creature>();

    //ASSERT_EQ(creature2.getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_, playerHp_);
    //ASSERT_EQ(creature2.getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_, playerMp_);

    //for (;;) {
    //    const SkillCode validSkillCode = servertest::defaultMeleeAttackSkillCode;
    //    StartCastInfo unionCastInfo;
    //    unionCastInfo.set(GameObjectInfo(otPc, characterId2_), validSkillCode);
    //    playerCastController1_->startCasting(unionCastInfo);

    //    const gideon::datatable::SkillTemplate* skillTemplate =
    //        SKILL_TABLE->getSkill(validSkillCode);
    //    ASSERT_TRUE(skillTemplate != nullptr);
    //    mockGameTimerSource_->set(GAME_TIMER->msec() +
    //        skillTemplate->getSkillCommonInfo().coolTime_ + 1);
    //    ++player2KillCount_;
    //    if (creature2.getUnionEntityInfo().asCreatureInfo().currentPoints_.isDied()) {
    //        break;
    //    }
    //}
}


StartCastInfo GameTestFixture::createUnionSkillCastToInfo(const GameObjectInfo& info, SkillCode skillCode)
{
    StartCastInfo castInfo;
    castInfo.set(uctSkillTo, info, skillCode);
    return castInfo;
}


StartCastInfo GameTestFixture::createUnionSkillCastAtInfo(const Position& info, SkillCode skillCode)
{
    StartCastInfo castInfo;
    castInfo.set(info, skillCode);
    return castInfo;
}



StartCastInfo GameTestFixture::createUnionItemCastToInfo(const GameObjectInfo& info, ObjectId objectId)
{
    StartCastInfo castInfo;
    castInfo.set(uctSkillTo, objectId, info);
    return castInfo;
}


StartCastInfo GameTestFixture::createUnionItemCastAtInfo(const Position& info, ObjectId objectId)
{
    StartCastInfo castInfo;
    castInfo.set(objectId, info);
    return castInfo;
}
