#pragma once

#include "../../zoneserver_export.h"
#include "../callback/InventoryCallback.h"
#include "../callback/EquipInventoryCallback.h"
#include "../callback/AccessoryInventoryCallback.h"
#include "../callback/QuestInventoryCallback.h"
#include "../Controller.h"
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/InventoryRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerInventoryController
 * 아이템 담당
 */
class ZoneServer_Export PlayerInventoryController : public Controller,
    public rpc::InventoryRpc,
	public InventoryCallback,
    public EquipInventoryCallback,
    public AccessoryInventoryCallback,
    public QuestInventoryCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerInventoryController);
public:
    PlayerInventoryController(go::Entity* owner);

public:
    virtual void initialize();
    virtual void finalize();
public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = InventoryCallback overriding
    virtual void inventoryItemMoved(InvenType invenType, ObjectId itemId, SlotId slotId, bool dbSave = true);
    virtual void inventoryItemSwitched(InvenType invenType, ObjectId itemId1, ObjectId itemId2, bool dbSave = true);
    virtual void inventoryItemAdded(InvenType invenType, const ItemInfo& itemInfo, bool dbSave = true);
    virtual void inventoryItemRemoved(InvenType invenType, ObjectId itemId, bool dbSave = true);
    virtual void inventoryItemCountUpdated(InvenType invenType, ObjectId itemId, uint8_t itemCount, bool dbSave = true);
    virtual void inventoryInfoUpdated(InvenType invenType, bool isCashSlot, uint8_t count, bool dbSave = true);
    virtual void inventoryItemAllRemoved(InvenType invenType);

    // = EquipInventoryCallback overriding
    virtual void unequipItemReplaced(ObjectId itemId, SlotId slotId, EquipPart unequipPart);
    virtual void equipReplaced(ObjectId itemId, EquipPart equipPart);
    virtual void inventoryWithEquipItemReplaced(ObjectId unequipItemId,
        EquipPart unequipPart, ObjectId equipItemId, EquipPart equipPart);
    virtual void equipItemInfoChanged(ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount);
    virtual void equipItemEnchanted(ObjectId itemId, SocketSlotId id, EquipSocketInfo& socketInfo);
    virtual void equipItemUnenchanted(ObjectId itemId, SocketSlotId id);

    // = AccessoryInventoryCallback overriding
    virtual void unequipAccessoryItemReplaced(ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart);
    virtual void equipAccessoryReplaced(ObjectId itemId, AccessoryIndex equipPart);
    virtual void inventoryWithAccessoryItemReplaced(ObjectId unequipItemId,
        AccessoryIndex unequipPart, ObjectId equipItemId, AccessoryIndex equipPart);

private:
    // = QuestInventoryCallback overriding
    virtual void inventoryQuestItemAdded(const QuestItemInfo& itemInfo);
    virtual void inventoryQuestItemRemoved(ObjectId itemId);
    virtual void inventoryQuestItemUsableCountUpdated(ObjectId itemId, uint8_t usableCount);
	virtual void inventoryQuestItemCountUpdated(ObjectId itemId, uint8_t stackCount);

public:
    // = rpc::InventoryRpc overriding
    OVERRIDE_SRPC_METHOD_3(moveInventoryItem,
        InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_3(switchInventoryItem,
        InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_2(removeInventoryItem,
        InvenType, invenType, ObjectId, itemId1);

    OVERRIDE_SRPC_METHOD_1(equipItem,
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(unequipItem,
        ObjectId, itemId, SlotId, slotId);

    OVERRIDE_SRPC_METHOD_2(equipAccessoryItem,
        ObjectId, itemId, AccessoryIndex, index);
    OVERRIDE_SRPC_METHOD_2(unequipAccessoryItem,
        ObjectId, itemId, SlotId, slotId);

    OVERRIDE_SRPC_METHOD_4(divideItem,
        InvenType, invenType, ObjectId, itemId, uint8_t, count, SlotId, slotId);

    OVERRIDE_SRPC_METHOD_3(movePlayerInvenItemToBankInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, SlotId, bankInvenSlot);
    OVERRIDE_SRPC_METHOD_3(moveBankInvenItemToPlayerInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, bankItemId, SlotId, playerInvenSlot);
    OVERRIDE_SRPC_METHOD_3(switchBankInvenItemAndPlayerInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, ObjectId, bankItemId);
	
	OVERRIDE_SRPC_METHOD_2(moveInvenItemToVehicleInvenItem,
		ObjectId, playerItemId, SlotId, vehicleInvenSlot);
	OVERRIDE_SRPC_METHOD_2(moveVehicleInvenItemToPlayerInvenItem,
		ObjectId, vehicleItemId, SlotId, playerInvenSlot);
	OVERRIDE_SRPC_METHOD_2(switchVehicleInvenItemAndPlayerInvenItem,
		ObjectId, playerItemId, ObjectId, vehicleItemId);

    OVERRIDE_SRPC_METHOD_2(depositGameMoney,
        GameObjectInfo, targetInfo, GameMoney, invenGameMoney);
    OVERRIDE_SRPC_METHOD_2(withdrawGameMoney,
        GameObjectInfo, targetInfo, GameMoney, bankGameMoney);

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
    OVERRIDE_SRPC_METHOD_3(onEquipAccessoryItem,
        ErrorCode, errorCode, ObjectId, itemId, AccessoryIndex, index);
    OVERRIDE_SRPC_METHOD_3(onUnequipAccessoryItem,
        ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_1(onMovePlayerInvenItemToBankInvenItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onMoveBankInvenItemToPlayerInvenItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onSwitchBankInvenItemAndPlayerInvenItem,
        ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_1(onMoveInvenItemToVehicleInvenItem,
		ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_1(onMoveVehicleInvenItemToPlayerInvenItem,
		ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_1(onSwitchVehicleInvenItemAndPlayerInvenItem,
		ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_3(onDepositGameMoney,
        ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney);
    OVERRIDE_SRPC_METHOD_3(onWithdrawGameMoney,
        ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney);

    OVERRIDE_SRPC_METHOD_2(evItemEquipped,
        GameObjectInfo, creatureInfo, EquipCode, equipCode);
    OVERRIDE_SRPC_METHOD_2(evItemUnequipped,
        GameObjectInfo, creatureInfo, EquipCode, equipCode);

    OVERRIDE_SRPC_METHOD_3(evInventoryItemCountUpdated,
        InvenType, invenType, ObjectId, itemId, uint8_t, ItemCount);
    OVERRIDE_SRPC_METHOD_2(evInventoryItemAdded,
        InvenType, invenType, ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_3(evEquipItemEnchanted,
        ObjectId, itemId, SocketSlotId, id, EquipSocketInfo, socketInfo);
    OVERRIDE_SRPC_METHOD_2(evEquipItemUnenchanted,
        ObjectId, itemId, SocketSlotId, id);
    OVERRIDE_SRPC_METHOD_3(evInventoryEquipItemChanged,
        ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount);


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

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    ErrorCode checkUseBank(const GameObjectInfo& objectInfo);

private:
    AccountId accountId_;
    ObjectId characterId_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
