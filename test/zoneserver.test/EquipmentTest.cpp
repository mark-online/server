#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerItemController.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/model/gameobject/status/CreatureStatus.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <sne/database/DatabaseManager.h>

using gideon::servertest::MockProxyGameDatabase;

/**
* @class EquipmentTest
*
* 장비 관련 테스트
*/
class EquipmentTest : public GameTestFixture
{ 
};


TEST_F(EquipmentTest, testCreatureStatusInfo)
{
    const go::Creature* player1 = static_cast<const go::Creature*>(player1_);
    const Points maxPoints = player1->getCreatureStatus().getMaxPoints();
    ASSERT_TRUE(maxPoints.hp_ > hpDefault);
    ASSERT_TRUE(maxPoints.mp_ > mpDefault);
}


TEST_F(EquipmentTest, testEquipItemNotOwned)
{
    const ObjectId notOwnedItemId = invalidObjectId;
    playerInventoryController1_->equipItem(notOwnedItemId);
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("onEquipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onEquipItem"));
    ASSERT_EQ(ecInventoryItemNotFound, playerInventoryController1_->lastErrorCode_);
}


TEST_F(EquipmentTest, testEquipItemAlreadyEquipped)
{
    const InventoryInfo inven = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* sword = getItemInfo(inven,
        servertest::defaultOneHandSwordEquipCode);
    ASSERT_TRUE(sword != nullptr);

    playerInventoryController1_->equipItem(sword->itemId_);
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("onEquipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onEquipItem"));
    ASSERT_EQ(ecEquipItemAlreadyEquipped, playerInventoryController1_->lastErrorCode_);
}


TEST_F(EquipmentTest, testEquipNotEquippedItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* shoes = getItemInfo(invenBefore,
        servertest::shoesEquipCode);
    ASSERT_TRUE(shoes != nullptr);

    playerInventoryController1_->equipItem(shoes->itemId_);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onEquipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onEquipItem"));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evEquipItemReplaced"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("evEquipItemReplaced"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evItemEquipped"));
    ASSERT_EQ(1, playerInventoryController2_->getCallCount("evItemEquipped"));
    ASSERT_EQ(shoes->itemCode_, playerInventoryController2_->lastEquipCode_);

    const MoreCharacterInfo& characterInfo =
        player1_->getUnionEntityInfo().asCharacterInfo();
    ASSERT_EQ(playerInventoryController2_->lastEquipCode_,
        characterInfo.equipments_[epShoes]);

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* shoesAfter = getItemInfo(invenAfter,
        servertest::shoesEquipCode);
    ASSERT_TRUE(shoesAfter != nullptr);
    ASSERT_TRUE(shoesAfter->isEquipped());

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("equipItem"));
}


TEST_F(EquipmentTest, testUnequipItemNotEquipped)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* shoes = getItemInfo(invenBefore,
        servertest::shoesEquipCode);
    ASSERT_TRUE(shoes != nullptr);

    playerInventoryController1_->unequipItem(shoes->itemId_, invalidSlotId);
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("onUnequipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onUnequipItem"));
    ASSERT_EQ(ecUnequipItemNotEquipped, playerInventoryController1_->lastErrorCode_);
}


TEST_F(EquipmentTest, testUnequipItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmet = getItemInfo(invenBefore,
        servertest::defaultHelmetEquipCode);
    ASSERT_TRUE(helmet != nullptr);
    const SlotId firstEmptySlotId = invenBefore.getFirstEmptySlotId();

    playerInventoryController1_->unequipItem(helmet->itemId_, firstEmptySlotId);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onUnequipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onUnequipItem"));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evUnequipItemReplaced"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("evUnequipItemReplaced"));


    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evItemUnequipped"));
    ASSERT_EQ(1, playerInventoryController2_->getCallCount("evItemUnequipped"));
    ASSERT_EQ(helmet->itemCode_, playerInventoryController2_->lastUnequipCode_);

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evUnequipItemReplaced"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evEquipItemReplaced"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryWithEquipItemReplaced"));


    const MoreCharacterInfo& characterInfo =
        player1_->getUnionEntityInfo().asCharacterInfo();
    ASSERT_EQ(playerInventoryController2_->lastEquipCode_,
        characterInfo.equipments_[epHelmet]);

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmetAfter = getItemInfo(invenAfter,
        servertest::defaultHelmetEquipCode);
    ASSERT_TRUE(helmetAfter != nullptr);
    ASSERT_TRUE(! helmetAfter->isEquipped());

    ASSERT_EQ(firstEmptySlotId,
        playerInventoryController1_->lastUnequippedSlotId_);

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("unequipItem"));
}


TEST_F(EquipmentTest, testEquipSamePartItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* otherHelmet = getItemInfo(invenBefore,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(otherHelmet != nullptr);

    playerInventoryController1_->equipItem(otherHelmet->itemId_);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onEquipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onEquipItem"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evItemEquipped"));
    ASSERT_EQ(1, playerInventoryController2_->getCallCount("evItemEquipped"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evUnequipItemReplaced"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evEquipItemReplaced"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryWithEquipItemReplaced"));

    ASSERT_EQ(otherHelmet->itemCode_, playerInventoryController2_->lastEquipCode_);

    const MoreCharacterInfo& characterInfo =
        player1_->getUnionEntityInfo().asCharacterInfo();
    ASSERT_EQ(playerInventoryController2_->lastEquipCode_,
        characterInfo.equipments_[epHelmet]);

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmetAfter = getItemInfo(invenAfter,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(helmetAfter != nullptr);
    ASSERT_TRUE(helmetAfter->isEquipped());

    const ItemInfo* helmetBefore = getItemInfo(invenAfter,
        servertest::defaultHelmetEquipCode);
    ASSERT_TRUE(helmetBefore != nullptr);
    ASSERT_TRUE(! helmetBefore->isEquipped());

    // TODO: switch 호출로 바꿀것
    //sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    //ASSERT_EQ(1, db->getCallCount("unequipItem"));
    //ASSERT_EQ(1, db->getCallCount("equipItem"));
}


TEST_F(EquipmentTest, testEquipTwoHandItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* spear = getItemInfo(invenBefore,
        servertest::lanceEquipCode);
    ASSERT_TRUE(spear != nullptr);

    playerInventoryController1_->equipItem(spear->itemId_);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onEquipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onEquipItem"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evItemEquipped"));
    ASSERT_EQ(1, playerInventoryController2_->getCallCount("evItemEquipped"));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evUnequipItemReplaced"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evEquipItemReplaced"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryWithEquipItemReplaced"));

    ASSERT_EQ(spear->itemCode_, playerInventoryController2_->lastEquipCode_);

    const MoreCharacterInfo& characterInfo =
        player1_->getUnionEntityInfo().asCharacterInfo();
    ASSERT_EQ(playerInventoryController2_->lastEquipCode_,
        characterInfo.equipments_[epTwoHands]);

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* spearAfter = getItemInfo(invenAfter,
        servertest::lanceEquipCode);
    ASSERT_TRUE(spearAfter != nullptr);
    ASSERT_TRUE(spearAfter->isEquipped());

    const ItemInfo* leftHandBefore = getItemInfo(invenAfter,
        servertest::defaultShieldEquipCode);
    ASSERT_TRUE(leftHandBefore != nullptr);
    ASSERT_TRUE(! leftHandBefore->isEquipped());

    const ItemInfo* rightHandBefore = getItemInfo(invenAfter,
        servertest::defaultOneHandSwordEquipCode);
    ASSERT_TRUE(rightHandBefore != nullptr);
    ASSERT_TRUE(! rightHandBefore->isEquipped());

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("unequipItem"));
    ASSERT_EQ(1, db->getCallCount("replaceInventoryWithEquipItem"));
}


TEST_F(EquipmentTest, testUnequipTwoHandItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* spear = getItemInfo(invenBefore,
        servertest::lanceEquipCode);
    ASSERT_TRUE(spear != nullptr);

    playerInventoryController1_->equipItem(spear->itemId_);
    playerInventoryController1_->unequipItem(spear->itemId_, invalidSlotId);

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onUnequipItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onUnequipItem"));

    ASSERT_EQ(1 + 1, playerInventoryController1_->getCallCount("evUnequipItemReplaced"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evEquipItemReplaced"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryWithEquipItemReplaced"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evItemUnequipped"));
    ASSERT_EQ(1, playerInventoryController2_->getCallCount("evItemUnequipped"));
    ASSERT_EQ(spear->itemCode_, playerInventoryController2_->lastUnequipCode_);

    const MoreCharacterInfo& characterInfo =
        player1_->getUnionEntityInfo().asCharacterInfo();
    ASSERT_EQ(invalidEquipCode, characterInfo.equipments_[epTwoHands]);
    ASSERT_EQ(invalidEquipCode, characterInfo.equipments_[epLeftHand]);
    ASSERT_EQ(invalidEquipCode, characterInfo.equipments_[epRightHand]);

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* spearAfter = getItemInfo(invenAfter,
        servertest::lanceEquipCode);
    ASSERT_TRUE(spearAfter != nullptr);
    ASSERT_TRUE(! spearAfter->isEquipped());

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(2, db->getCallCount("unequipItem"));
    ASSERT_EQ(0, db->getCallCount("equipItem"));
    ASSERT_EQ(1, db->getCallCount("replaceInventoryWithEquipItem"));
    // TODO: db switch 추가
}



TEST_F(EquipmentTest, testMakeEquipItemTest)
{
    //const uint8_t oneHandSwordFragmentCount = 46;
    //const SlotId slotId = 17;
    //BaseItemInfo fragmentItem(servertest::oneHandSwordFragmentCode, oneHandSwordFragmentCount);
    //playerItemController1_->makeEquipItem(servertest::upgradeOneHandSwordEquipCode);
    //ASSERT_EQ(ecItemNotMakeEquipItem, playerItemController1_->lastErrorCode_);
    //ASSERT_EQ(1, playerItemController1_->getCallCount("onMakeEquipItem"));

    //playerItemController1_->makeEquipItem(servertest::defaultOneHandSwordEquipCode);
    //ASSERT_EQ(ecItemNotEnoughFragmentItem, playerItemController1_->lastErrorCode_);
    //ASSERT_EQ(2, playerItemController1_->getCallCount("onMakeEquipItem"));

    //ASSERT_TRUE(playerInventoryController1_->addInventoryItem(fragmentItem, slotId));
    //const InventoryInfo beforeInven = player1_->queryInventoryable()->getInventoryInfo();


    //playerItemController1_->makeEquipItem(servertest::defaultOneHandSwordEquipCode);
    //ASSERT_EQ(ecOk, playerItemController1_->lastErrorCode_);
    //ASSERT_EQ(3, playerItemController1_->getCallCount("onMakeEquipItem"));
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    //ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryEquipItemAdded"));
    //const InventoryInfo afterInven = player1_->queryInventoryable()->getInventoryInfo();

    //ASSERT_EQ(beforeInven.items_.size() + 1, afterInven.items_.size());
}


TEST_F(EquipmentTest, testUpdateEquipItem)
{
    // TODO: 테스트 복구 (2019-03-26)
    //const uint8_t oneHandSwordFragmentCount = 46;
    //const SlotId slotId = 17;
    //BaseItemInfo fragmentItem(servertest::oneHandSwordFragmentCode, oneHandSwordFragmentCount);
    //ASSERT_TRUE(playerInventoryController1_->addInventoryItem(fragmentItem, slotId));

    //const InventoryInfo beforeInven = player1_->queryInventoryable()->getInventoryInfo();
    //const ItemInfo* defaultSword = getItemInfo(beforeInven,
    //    servertest::defaultOneHandSwordEquipCode);
    //ASSERT_TRUE(defaultSword != nullptr);
    //const ItemInfo* fragment = getItemInfo(beforeInven,
    //    servertest::oneHandSwordFragmentCode);
    //ASSERT_TRUE(fragment != nullptr);

    //for (int i = 0; i < 5; ++i) {
    //    playerItemController1_->upgradeEquipItem(GameObjectInfo(), defaultSword->itemId_, fragment->itemId_);
    //    ASSERT_EQ(ecOk, playerItemController1_->lastErrorCode_);
    //}

    //playerItemController1_->upgradeEquipItem(GameObjectInfo(), defaultSword->itemId_, fragment->itemId_);
    //ASSERT_EQ(ecItemMaxUpgradeEquipItem, playerItemController1_->lastErrorCode_);

    //const InventoryInfo afterInven = player1_->queryInventoryable()->getInventoryInfo();
    //const ItemInfo* upgradeSword = getItemInfo(afterInven,
    //    servertest::upgradeOneHandSwordEquipCode);
    //ASSERT_TRUE(upgradeSword != nullptr);
}
