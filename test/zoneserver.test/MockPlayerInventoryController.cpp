#include "ZoneServerTestPCH.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Entity.h"
#include "ZoneServer/model/gameobject/ability/Inventoryable.h"
#include "ZoneServer/model/item/Inventory.h"

using namespace gideon::zoneserver;

MockPlayerInventoryController::MockPlayerInventoryController(zoneserver::go::Entity* owner) :
    zoneserver::gc::PlayerInventoryController(owner),
    lastErrorCode_(ecWhatDidYouTest),
    lastUnequippedSlotId_(invalidSlotId),
    lastAddItemId_(invalidObjectId),
    lastEquipCode_(invalidEquipCode),
    lastUnequipCode_(invalidEquipCode)
{
}


bool MockPlayerInventoryController::addInventoryItem(const BaseItemInfo& itemInfo, SlotId slotId)
{
    itemInfo, slotId;
    return true;
    //go::Entity& owner = getOwner();
    //return isSucceeded(owner.queryInventoryable()->getInventory().addItem(itemInfo, slotId));
}


// = rpc::InventoryRpc overriding

DEFINE_SRPC_METHOD_4(MockPlayerInventoryController, onMoveInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, SlotId, slotId)
{
    addCallCount("onMoveInventoryItem");

    lastErrorCode_ = errorCode;
    invenType, itemId, slotId;
}


DEFINE_SRPC_METHOD_4(MockPlayerInventoryController, onSwitchInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2)
{
    addCallCount("onSwitchInventoryItem");

    lastErrorCode_ = errorCode;
    invenType, itemId1, itemId2;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, onRemoveInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId)
{
    addCallCount("onRemoveInventoryItem");

    lastErrorCode_ = errorCode;
    invenType, itemId;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, onEquipItem,
    ErrorCode, errorCode, ObjectId, itemId)
{
    addCallCount("onEquipItem");

    lastErrorCode_ = errorCode;
    itemId;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, onUnequipItem,
    ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId)
{
    addCallCount("onUnequipItem");

    lastErrorCode_ = errorCode;
    itemId;
    lastUnequippedSlotId_ = slotId;
}


DEFINE_SRPC_METHOD_4(MockPlayerInventoryController, onDivideItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, uint8_t, count)
{
    addCallCount("onDivideItem");

    lastErrorCode_ = errorCode;
    invenType, itemId, count;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evItemEquipped,
    GameObjectInfo, creatureInfo, EquipCode, equipCode)
{
    addCallCount("evItemEquipped");

    creatureInfo;
    lastEquipCode_ = equipCode;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evItemUnequipped,
    GameObjectInfo, creatureInfo, EquipCode, equipCode)
{
    addCallCount("evItemUnequipped");

    creatureInfo;
    lastUnequipCode_ = equipCode;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evInventoryItemCountUpdated,
    InvenType, invenType, ObjectId, itemId, uint8_t, ItemCount)
{
    addCallCount("evInventoryItemCountUpdated");

    invenType, itemId, ItemCount;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evInventoryItemAdded,
    InvenType, invenType, ItemInfo, itemInfo)
{
    addCallCount("evInventoryItemAdded");
    invenType;
    lastAddItemId_ = itemInfo.itemId_;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evInventoryEquipItemChanged,
    ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount)
{
    addCallCount("evInventoryEquipItemChanged");

    itemId, newEquipCode, socketCount;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evEquipItemEnchanted,
    ObjectId, itemId, SocketSlotId, id, EquipSocketInfo, socketInfo)
{
    addCallCount("evEquipItemEnchanted");

    itemId, id, socketInfo;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evEquipItemUnenchanted,
    ObjectId, itemId, SocketSlotId, id)
{
    addCallCount("evEquipItemUnenchanted");

    itemId, id;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evInventoryItemRemoved,
    InvenType, invenType, ObjectId, itemId)
{
    addCallCount("evInventoryItemRemoved");

    invenType, itemId;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evInventoryItemMoved,
    InvenType, invenType, ObjectId, itemId, SlotId, slotId)
{
    addCallCount("evInventoryItemMoved");
    
    invenType, itemId, slotId;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evInventoryItemSwitched,
    InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2)
{
    addCallCount("evInventoryItemSwitched");

    invenType, itemId1, itemId2;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evUnequipItemReplaced,
    ObjectId, equipItemId, SlotId, slotId, EquipPart, unequipPrt)
{
    addCallCount("evUnequipItemReplaced");
    lastUnequippedSlotId_ = slotId;
    equipItemId, unequipPrt;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evEquipItemReplaced,
    ObjectId, equipItemId, EquipPart, equipPrt)
{
    addCallCount("evEquipItemReplaced");

    equipItemId, equipPrt;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evInventoryWithEquipItemReplaced,
    ObjectId, equipItemId1, ObjectId, equipItemId2)
{
    addCallCount("evInventoryWithEquipItemReplaced");

    equipItemId1, equipItemId2;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evUnequipAccessoryItemReplaced, 
    ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPrt)
{
    addCallCount("evUnequipAccessoryItemReplaced");

    itemId, slotId, unequipPrt;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evEquipAccessoryItemReplaced,
    ObjectId, itemId, AccessoryIndex, equipPrt)
{
    addCallCount("evEquipAccessoryItemReplaced");
    itemId, equipPrt;
}


DEFINE_SRPC_METHOD_4(MockPlayerInventoryController, evInventoryWithAccessoryItemReplaced,
    ObjectId, unequipItemId, AccessoryIndex, unequipIndex,
    ObjectId, equipItemId, AccessoryIndex, equipIndex)
{
    addCallCount("evInventoryWithAccessoryItemReplaced");

    unequipItemId, unequipIndex, equipItemId, equipIndex;
}


DEFINE_SRPC_METHOD_3(MockPlayerInventoryController, evInventoryInfoUpdated, 
    InvenType, invenTpe, bool, isCashSlot, uint8_t, currentExtendSlot)
{
	addCallCount("evInventoryInfoUpdated");
    invenTpe, isCashSlot, currentExtendSlot;
}


DEFINE_SRPC_METHOD_1(MockPlayerInventoryController, evQuestItemAdded, 
    QuestItemInfo, questItemInfo)
{
    addCallCount("evQuestItemAdded");

    questItemInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerInventoryController, evQuestItemRemoved,
    ObjectId, itemId)
{
    addCallCount("evQuestItemRemoved");

    itemId;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evQuestItemUsableCountUpdated,
    ObjectId, itemId, uint8_t, usableCount)
{
    addCallCount("evQuestItemUsableCountUpdated");

    itemId, usableCount;
}


DEFINE_SRPC_METHOD_2(MockPlayerInventoryController, evQuestItemCountUpdated,
	ObjectId, itemId, uint8_t, stackCount)
{
	addCallCount("evQuestItemCountUpdated");

	itemId, stackCount;
}
