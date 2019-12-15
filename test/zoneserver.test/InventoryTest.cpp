#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <sne/database/DatabaseManager.h>

using gideon::servertest::MockProxyGameDatabase;

const uint8_t maxStackOfInventory = 255;
/**
* @class InventoryTest
*
* 인벤토리 관련 테스트
*/
class InventoryTest : public GameTestFixture
{
};


TEST_F(InventoryTest, testMoveInventoryItemIntoInvalidSlotId)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmet = getItemInfo(invenBefore,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(helmet != nullptr);

    playerInventoryController1_->moveInventoryItem(itPc, helmet->itemId_, defultInvenSlotId + 1);
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("onMoveInventoryItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onMoveInventoryItem"));
    ASSERT_EQ(ecInventorySlotNotValid, playerInventoryController1_->lastErrorCode_);

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));
}


TEST_F(InventoryTest, testMoveInventoryItemIntoOccupiedSlotId)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmet = getItemInfo(invenBefore,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(helmet != nullptr);

    playerInventoryController1_->moveInventoryItem(itPc, helmet->itemId_, firstSlotId);
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("onMoveInventoryItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onMoveInventoryItem"));
    ASSERT_EQ(ecInventorySlotOccupied, playerInventoryController1_->lastErrorCode_);

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));
}


TEST_F(InventoryTest, testCannotMoveEquippedItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmet = getItemInfo(invenBefore,
        servertest::defaultHelmetEquipCode);
    ASSERT_TRUE(helmet != nullptr);

    playerInventoryController1_->moveInventoryItem(itPc, helmet->itemId_, firstSlotId);
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("onMoveInventoryItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onMoveInventoryItem"));
    ASSERT_EQ(ecInventoryCannotChangeEquippedItemSlot, playerInventoryController1_->lastErrorCode_);

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));
}


TEST_F(InventoryTest, testMoveInventoryItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmet = getItemInfo(invenBefore,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(helmet != nullptr);
    const SlotId firstEmptySlotId = invenBefore.getFirstEmptySlotId();

    playerInventoryController1_->moveInventoryItem(itPc, helmet->itemId_, firstEmptySlotId);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onMoveInventoryItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onMoveInventoryItem"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmetAfter = invenAfter.getItemInfo(helmet->itemId_);
    ASSERT_TRUE(helmetAfter != nullptr);
    ASSERT_EQ(firstEmptySlotId, helmetAfter->slotId_);

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("moveInventoryItem"));
}


TEST_F(InventoryTest, testSwitchInventoryItem)
{
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmet = getItemInfo(invenBefore,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(helmet != nullptr);

    const ItemInfo* spear = getItemInfo(invenBefore,
        servertest::lanceEquipCode);
    ASSERT_TRUE(spear != nullptr);

    playerInventoryController1_->switchInventoryItem(itPc, helmet->itemId_, spear->itemId_);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onSwitchInventoryItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onSwitchInventoryItem"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmetAfter = invenAfter.getItemInfo(helmet->itemId_);
    ASSERT_TRUE(helmetAfter != nullptr);
    ASSERT_EQ(spear->slotId_, helmetAfter->slotId_);

    const ItemInfo* spearAfter = invenAfter.getItemInfo(spear->itemId_);
    ASSERT_TRUE(spearAfter != nullptr);
    ASSERT_EQ(helmet->slotId_, spearAfter->slotId_);

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("switchInventoryItem"));
}

TEST_F(InventoryTest, testMoveAndDeleteInventoryItem)
{
    const SlotId updateSlotId = 15;
    const SlotId deleteSlotId = 16;
    const uint8_t itemCount = 100;

    const BaseItemInfo stackUpdateItem(servertest::normalNpcElementCode1, itemCount);
    const BaseItemInfo deleteItem(servertest::normalNpcElementCode1, itemCount);

    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( stackUpdateItem, updateSlotId));
    const ObjectId updateItemId = playerInventoryController1_->lastAddItemId_;
    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( deleteItem, deleteSlotId));
    const ObjectId deleteItemId  = playerInventoryController1_->lastAddItemId_;
    ASSERT_EQ(2, playerInventoryController1_->getCallCount("evInventoryItemAdded"));


    playerInventoryController1_->switchInventoryItem(itPc, deleteItemId, updateItemId);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onSwitchInventoryItem"));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(2, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* element = invenAfter.getItemInfo(updateItemId);
    const ItemInfo* deleteElement = invenAfter.getItemInfo(deleteItemId);
    ASSERT_TRUE(element  != nullptr);
    ASSERT_TRUE(! deleteElement);
    ASSERT_EQ(itemCount + itemCount , element->count_);
}


TEST_F(InventoryTest, testMoveAndUpdateInventoryItem)
{
    const SlotId updateSlotId1 = 15;
    const SlotId updateSlotId2 = 16;
    
    const uint8_t updateStack = 250;


    const BaseItemInfo stackUpdateItem(servertest::normalNpcElementCode1, updateStack);
    const BaseItemInfo deleteItem(servertest::normalNpcElementCode1, updateStack);
    
    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( stackUpdateItem, updateSlotId1));
    const ObjectId updateItemId1 = playerInventoryController1_->lastAddItemId_;
    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( deleteItem, updateSlotId2));
    const ObjectId updateItemId2  = playerInventoryController1_->lastAddItemId_;
    ASSERT_EQ(2, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

    
    ASSERT_EQ(2, playerInventoryController1_->getCallCount("evInventoryItemAdded"));

    playerInventoryController1_->switchInventoryItem(itPc, updateItemId1, updateItemId2);

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onSwitchInventoryItem"));

    ASSERT_EQ(2, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(2, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));

    const InventoryInfo invenAfter = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* element1 = invenAfter.getItemInfo(updateItemId1);
    const ItemInfo* element2 = invenAfter.getItemInfo(updateItemId2);
    ASSERT_TRUE(element1  != nullptr);
    ASSERT_TRUE(element2  != nullptr);
    ASSERT_EQ(maxStackOfInventory, element2->count_);
    ASSERT_EQ(updateStack - (maxStackOfInventory - updateStack),
        element1->count_);
}



TEST_F(InventoryTest, testRemoveInventoryItem)
{
    const SlotId removeSlotId = 15;    
    const uint8_t stack = 250;

    const BaseItemInfo removeItem(servertest::normalNpcElementCode1, stack);
    
    ASSERT_TRUE(playerInventoryController1_->addInventoryItem( removeItem, removeSlotId));
    const ObjectId removeItemId = playerInventoryController1_->lastAddItemId_;


    playerInventoryController1_->removeInventoryItem(itPc, removeItemId);
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("onRemoveInventoryItem"));
    ASSERT_EQ(0, playerInventoryController2_->getCallCount("onRemoveInventoryItem"));

    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
    ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("removeInventoryItem"));
}


TEST_F(InventoryTest, testDivideInventoryItem)
{
	const SlotId removeSlotId = 15;    
	const uint8_t stack = 250;

	const BaseItemInfo removeItem(servertest::normalNpcElementCode1, stack);

	ASSERT_TRUE(playerInventoryController1_->addInventoryItem( removeItem, removeSlotId));
	const ObjectId addItemId = playerInventoryController1_->lastAddItemId_;
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
	ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));

	playerInventoryController1_->divideItem(itPc, addItemId, 5, invalidSlotId);
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("onDivideItem"));

	ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemCountUpdated"));
	ASSERT_EQ(1 + 1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemMoved"));
	ASSERT_EQ(0, playerInventoryController1_->getCallCount("evInventoryItemSwitched"));

}