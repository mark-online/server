#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "MockPlayerTradeController.h"
#include "MockPlayerItemController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerCastController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/DataCodes.h>

/**
* @class ItemTest
*
* 아이템 관련 테스트
*/
class ItemTest : public GameTestFixture
{
};


TEST_F(ItemTest, testMakeComponet)
{
    //const ObjectId stackCount = 9;
    //const SlotId slotId = 15;

    //BaseItemInfo elementItem(servertest::normalNpcElementCode1, stackCount);

    //ASSERT_TRUE(playerInventoryController1_->addInventoryItem( elementItem, slotId));
    //const ObjectId itemId = playerInventoryController1_->lastAddItemId_;
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

    //playerItemController1_->makeComponent(GameObjectInfo(), itemId);
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    //ASSERT_EQ(1 + 1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    //ASSERT_EQ(1, playerItemController1_->getCallCount("onMakeComponent"));
    //ASSERT_EQ(ecOk, playerItemController1_->lastErrorCode_);

    //playerItemController1_->makeComponent(GameObjectInfo(), itemId);
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    //ASSERT_EQ(1 + 1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    //ASSERT_EQ(1 + 1, playerItemController1_->getCallCount("onMakeComponent"));
    //ASSERT_EQ(ecItemNotEnoughElementItem, playerItemController1_->lastErrorCode_);

}

//TEST_F(ItemTest, testUseRecipe)
//{
//    //const uint8_t stackCount1 = 4;
//    //const SlotId slotId1 = 15;
//    //BaseItemInfo componentItem1(servertest::refinedShellComponent, stackCount1);
//
//    //const uint8_t stackCount2 = 6;
//    //const SlotId slotId2 = 16;
//    //BaseItemInfo componentItem2(servertest::refinedVenomComponent, stackCount2);
//    
//    const uint8_t stackCount3 = 1;
//    const SlotId slotId3 = 17;
//    BaseItemInfo recipeITem(servertest::shabbyHelmetRecipeCode, stackCount3);
//
//
//    //ASSERT_TRUE(playerInventoryController1_->addInventoryItem( componentItem1, slotId1));
//    //ASSERT_TRUE(playerInventoryController1_->addInventoryItem( componentItem2, slotId2));
//    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( recipeITem, slotId3));
//    const ObjectId recipeId = playerInventoryController1_->lastAddItemId_;
//    ASSERT_EQ(3, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
//
//    playerItemController1_->useRecipeItem(recipeId);
//    ASSERT_EQ(ecItemRecipeItemNotRegistered, playerItemController1_->lastErrorCode_);
//
//    playerItemController1_->startUseRecipeItem(GameObjectInfo(), recipeId);
//    ASSERT_EQ(ecOk, playerItemController1_->lastErrorCode_);
//
//    mockGameTimerSource_->set(GAME_TIMER->msec() + 2000);
//    playerItemController1_->useRecipeItem(recipeId);
//    ASSERT_EQ(2, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
//    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
//    ASSERT_EQ(ecOk, playerItemController1_->lastErrorCode_);
//}


TEST_F(ItemTest, testSellItem)
{
	const uint8_t stackCount = 4;
	const SlotId slotId = 15;
	BaseItemInfo elementItem(servertest::normalNpcElementCode1, stackCount);

	ASSERT_TRUE(playerInventoryController1_->addInventoryItem( elementItem, slotId));
	const ObjectId itemId = playerInventoryController1_->lastAddItemId_;
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

	GameMoney preGameMoney = player1_->queryMoneyable()->getGameMoney();
	playerTradeController1_->sellItem(3, itemId, 4);
	ASSERT_EQ(1, playerTradeController1_->getCallCount("onSellItem"));
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(preGameMoney + stackCount, player1_->queryMoneyable()->getGameMoney());
}


TEST_F(ItemTest, testUseElementItem)
{
    const uint8_t stackCount = 4;
    const SlotId slotId = 15;
    BaseItemInfo elementItem(servertest::normalNpcElementCode1, stackCount);

    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( elementItem, slotId));
    const ObjectId itemId = playerInventoryController1_->lastAddItemId_;

    ASSERT_EQ(0, playerController1_->getCallCount("evPointChanged"));

    const GameObjectInfo targetInfo(otPc, characterId1_);
    playerCastController1_->startCasting(createUnionItemCastToInfo(targetInfo, itemId));

    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    ASSERT_EQ(1, playerEffectController1_->getCallCount("evItemEffected"));
    ASSERT_EQ(0, playerEffectController2_->getCallCount("evItemEffected"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));

    //ASSERT_EQ(2, playerController1_->getCallCount("evPointChanged"));
}


TEST_F(ItemTest, testEnchatEquip)
{
    //const ObjectId stackCount = 2;
    //const SlotId slotId = 15;
    //BaseItemInfo skillPackageItem(servertest::skillPackage_1, stackCount);
    //playerController1_->rewardExp(toExpPoint(120));
    //
    //const SkillCode skill1 = makeSkillCode(4, 1);
    //const SkillCode skill2 = makeSkillCode(4, 2);
    //const GameObjectInfo targetInfo(otPc, characterId2_);


    //ASSERT_TRUE(playerInventoryController1_->addInventoryItem( skillPackageItem, slotId));
    //const ObjectId itemId = playerInventoryController1_->lastAddItemId_;
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(targetInfo, skill1));
    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    //ASSERT_EQ(ecSkillNotLearned, playerCastController1_->lastErrorCode_);

    //const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    //const ItemInfo* spear = getItemInfo(invenBefore,
    //    servertest::lanceEquipCode);
    //ASSERT_TRUE(spear != nullptr);
    //playerItemController1_->useSkillPackageItem(itemId, 4);
    //playerInventoryController1_->equipItem(spear->itemId_);
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(targetInfo, skill1));
    //ASSERT_EQ(1 + 1, playerCastController1_->getCallCount("onStartCasting"));

    //playerItemController1_->enchantEquip(GameObjectInfo(), spear->itemId_, itemId, 4);
    //ASSERT_EQ(ecOk, playerItemController1_->lastErrorCode_);
    //ASSERT_EQ(1, playerItemController1_->getCallCount("onEnchantEquip"));
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(targetInfo, skill2));
    //ASSERT_EQ(1 + 2, playerCastController1_->getCallCount("onStartCasting"));

    //playerInventoryController1_->equipItem(spear->itemId_);
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(targetInfo, skill2));
    //ASSERT_EQ(1 + 3, playerCastController1_->getCallCount("onStartCasting"));
    //playerCastController1_->startCasting(createUnionSkillCastToInfo(targetInfo, skill1));
    //ASSERT_EQ(1 + 4, playerCastController1_->getCallCount("onStartCasting"));

}


TEST_F(ItemTest, testExtendInventory)
{
    const uint8_t stackCount = 4;
    const SlotId slotId = 15;
    BaseItemInfo elementItem(servertest::extendInventoryElementCode, stackCount);

    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( elementItem, slotId));
    const ObjectId itemId = playerInventoryController1_->lastAddItemId_;
    const GameObjectInfo targetInfo(otPc, characterId1_);
    playerCastController1_->startCasting(createUnionItemCastToInfo(targetInfo, itemId));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryInfoUpdated"));

}
