#pragma once

#include "ZoneServer/controller/player/PlayerInventoryController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerInventoryController
 *
 * 테스트 용 mock MockPlayerInventoryController
 */
class MockPlayerInventoryController :
    public zoneserver::gc::PlayerInventoryController,
    public sne::test::CallCounter
{
public:
    MockPlayerInventoryController(zoneserver::go::Entity* owner);

    bool addInventoryItem(const BaseItemInfo& itemInfo, SlotId slotId);

public: // = rpc::InventoryRpc overriding
    OVERRIDE_SRPC_METHOD_4(onMoveInventoryItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_4(onSwitchInventoryItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_3(onRemoveInventoryItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1);
    OVERRIDE_SRPC_METHOD_2(onEquipItem,
        ErrorCode, errorCode, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_3(onUnequipItem,
        ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_4(onDivideItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, uint8_t, count);

    OVERRIDE_SRPC_METHOD_2(evItemEquipped,
        GameObjectInfo, creatureInfo, EquipCode, equipCode);
    OVERRIDE_SRPC_METHOD_2(evItemUnequipped,
        GameObjectInfo, creatureInfo, EquipCode, equipCode);

    OVERRIDE_SRPC_METHOD_3(evInventoryItemCountUpdated,
        InvenType, invenType, ObjectId, itemId, uint8_t, ItemCount);
    OVERRIDE_SRPC_METHOD_2(evInventoryItemAdded,
        InvenType, invenType, ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_3(evInventoryEquipItemChanged,
        ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount);
    OVERRIDE_SRPC_METHOD_3(evEquipItemEnchanted,
        ObjectId, itemId, SocketSlotId, id, EquipSocketInfo, socketInfo);
    OVERRIDE_SRPC_METHOD_2(evEquipItemUnenchanted,
        ObjectId, itemId, SocketSlotId, id);


    OVERRIDE_SRPC_METHOD_2(evInventoryItemRemoved,
        InvenType, invenType, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_3(evInventoryItemMoved,
        InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_3(evInventoryItemSwitched,
        InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_3(evUnequipItemReplaced,
        ObjectId, itemId, SlotId, slotId, EquipPart, unequipPrt);
    OVERRIDE_SRPC_METHOD_2(evEquipItemReplaced,
        ObjectId, itemId, EquipPart, equipPrt);
    OVERRIDE_SRPC_METHOD_2(evInventoryWithEquipItemReplaced,
        ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_3(evUnequipAccessoryItemReplaced, 
        ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPrt);
    OVERRIDE_SRPC_METHOD_2(evEquipAccessoryItemReplaced,
        ObjectId, itemId, AccessoryIndex, equipPrt);
    OVERRIDE_SRPC_METHOD_4(evInventoryWithAccessoryItemReplaced,
        ObjectId, unequipItemId, AccessoryIndex, unequipIndex,
        ObjectId, equipItemId, AccessoryIndex, equipIndex);
    OVERRIDE_SRPC_METHOD_3(evInventoryInfoUpdated, 
        InvenType, invenTpe, bool, isCashSlot, uint8_t, currentExtendSlot);

    OVERRIDE_SRPC_METHOD_1(evQuestItemAdded, 
        QuestItemInfo, questItemInfo);
    OVERRIDE_SRPC_METHOD_1(evQuestItemRemoved,
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(evQuestItemUsableCountUpdated,
        ObjectId, itemId, uint8_t, usableCount);
	OVERRIDE_SRPC_METHOD_2(evQuestItemCountUpdated,
		ObjectId, itemId, uint8_t, stackCount);

public:
    ErrorCode lastErrorCode_;
    SlotId lastUnequippedSlotId_;
    ObjectId lastAddItemId_;
    EquipCode lastEquipCode_;
    EquipCode lastUnequipCode_;
};
