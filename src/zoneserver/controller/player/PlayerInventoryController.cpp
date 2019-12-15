#include "ZoneServerPCH.h"
#include "PlayerInventoryController.h"
#include "../../model/item/Inventory.h"
#include "../../model/item/QuestInventory.h"
#include "../../model/item/EquipInventory.h"
#include "../../model/item/AccessoryInventory.h"
#include "../../model/bank/BankAccount.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/Player.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Bankable.h"
#include "../../model/state/ItemManageState.h"
#include "../../service/inven/OrderInvenItemMoveService.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class EquipItemEvent
 */
class EquipItemEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<EquipItemEvent>
{
public:
    EquipItemEvent(const GameObjectInfo& creatureInfo, EquipCode equipCode) :
        creatureInfo_(creatureInfo),
        equipCode_(equipCode) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        go::Player& player = static_cast<go::Player&>(entity);
        player.getPlayerInventoryController().evItemEquipped(creatureInfo_, equipCode_);
    }

private:
    const GameObjectInfo creatureInfo_;
    const EquipCode equipCode_;
};


/**
 * @class UnequipItemEvent
 */
class UnequipItemEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<UnequipItemEvent>
{
public:
    UnequipItemEvent(const GameObjectInfo& creatureInfo, EquipCode equipCode) :
        creatureInfo_(creatureInfo),
        equipCode_(equipCode) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        go::Player& player = static_cast<go::Player&>(entity);
        player.getPlayerInventoryController().evItemUnequipped(creatureInfo_, equipCode_);
    }

private:
    const GameObjectInfo creatureInfo_;
    const EquipCode equipCode_;
};


} // namespace {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerInventoryController);

PlayerInventoryController::PlayerInventoryController(go::Entity* owner) :
    Controller(owner),
	accountId_(invalidAccountId),
	characterId_(invalidObjectId)
{
}


void PlayerInventoryController::initialize()
{
	accountId_ = getOwner().getAccountId();
	characterId_ = getOwner().getObjectId();
}


void PlayerInventoryController::finalize()
{
	accountId_ = invalidAccountId;
	characterId_ = invalidObjectId;
}


void PlayerInventoryController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerInventoryController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerInventoryController::inventoryItemMoved(InvenType invenType, ObjectId itemId, SlotId slotId, bool dbSave)
{
    evInventoryItemMoved(invenType, itemId, slotId);
    
    if (dbSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncMoveInventoryItem(accountId_, characterId_, invenType, itemId, slotId);
    }
}


void PlayerInventoryController::inventoryItemSwitched(InvenType invenType, ObjectId itemId1, ObjectId itemId2, bool dbSave)
{
    evInventoryItemSwitched(invenType, itemId1, itemId2);
    
    if (dbSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncSwitchInventoryItem(accountId_, characterId_, invenType, itemId1, itemId2);    
    }
}


void PlayerInventoryController::inventoryItemAdded(InvenType invenType, const ItemInfo& itemInfo, bool dbSave)
{
    evInventoryItemAdded(invenType, itemInfo);
    
    if (dbSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddInventoryItem(accountId_, characterId_, invenType, itemInfo);
    }
}


void PlayerInventoryController::inventoryItemRemoved(InvenType invenType, ObjectId itemId, bool dbSave)
{
    evInventoryItemRemoved(invenType, itemId);

    if (dbSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveInventoryItem(accountId_, characterId_, invenType, itemId);
    }
}


void PlayerInventoryController::inventoryItemCountUpdated(InvenType invenType, ObjectId itemId, uint8_t itemCount, bool dbSave)
{
    evInventoryItemCountUpdated(invenType, itemId, itemCount);
    
    if (dbSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateInventoryItemCount(accountId_, characterId_, invenType, itemId, itemCount);
    }
}


void PlayerInventoryController::inventoryInfoUpdated(InvenType invenType, bool isCashSlot, uint8_t count, bool dbSave) 
{    
    evInventoryInfoUpdated(invenType, isCashSlot, count);

    if (dbSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateCharacterInventoryInfo(accountId_, characterId_, invenType, isCashSlot, count);
    }
}


void PlayerInventoryController::inventoryItemAllRemoved(InvenType /*invenType*/)
{
    assert(false);
}


void PlayerInventoryController::unequipItemReplaced(ObjectId itemId, SlotId slotId, EquipPart unequipPart)
{
    evUnequipItemReplaced(itemId, slotId, unequipPart);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUnequipItem(accountId_, characterId_, itemId, slotId, unequipPart); 
}


void PlayerInventoryController::equipReplaced(ObjectId itemId, EquipPart equipPart)
{
    evEquipItemReplaced(itemId, equipPart);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncEquipItem(accountId_, characterId_, itemId, equipPart);   
}


void PlayerInventoryController::inventoryWithEquipItemReplaced(ObjectId unequipItemId,
    EquipPart unequipPart, ObjectId equipItemId, EquipPart equipPart)
{
    evInventoryWithEquipItemReplaced(unequipItemId, equipItemId);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncReplaceInventoryWithEquipItem(accountId_, characterId_, unequipItemId,
        unequipPart, equipItemId, equipPart);
}


void PlayerInventoryController::equipItemInfoChanged(ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount)
{
    evInventoryEquipItemChanged(itemId, newEquipCode, socketCount);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncChangeEquipItemInfo(accountId_, characterId_, itemId,
        newEquipCode, socketCount);
}


void PlayerInventoryController::equipItemEnchanted(ObjectId itemId, SocketSlotId id, EquipSocketInfo& socketInfo)
{
    evEquipItemEnchanted(itemId, id, socketInfo);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncEnchantEquipItem(accountId_, characterId_, itemId, id, socketInfo);
}


void PlayerInventoryController::equipItemUnenchanted(ObjectId itemId, SocketSlotId id)
{
    evEquipItemUnenchanted(itemId, id);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUnenchantEquipItem(accountId_, characterId_, itemId, id);
}


void PlayerInventoryController::unequipAccessoryItemReplaced(ObjectId itemId, SlotId slotId,
    AccessoryIndex unequipPart)
{
    evUnequipAccessoryItemReplaced(itemId, slotId, unequipPart);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUnequipAccessoryItem(accountId_, characterId_, itemId, slotId, unequipPart); 
}


void PlayerInventoryController::equipAccessoryReplaced(ObjectId itemId, AccessoryIndex equipPart)
{
    evEquipAccessoryItemReplaced(itemId, equipPart);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncEquipAccessoryItem(accountId_, characterId_, itemId, equipPart);   
}


void PlayerInventoryController::inventoryWithAccessoryItemReplaced(ObjectId unequipItemId,
    AccessoryIndex  unequipPart, ObjectId equipItemId, AccessoryIndex equipPart)
{
    evInventoryWithAccessoryItemReplaced(unequipItemId, unequipPart, equipItemId, equipPart);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncReplaceInventoryWithAccessoryItem(accountId_, characterId_, unequipItemId,
        unequipPart, equipItemId, equipPart);
}


void PlayerInventoryController::inventoryQuestItemAdded(const QuestItemInfo& itemInfo)
{
    evQuestItemAdded(itemInfo);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAddQuestItem(accountId_, characterId_, itemInfo);
}


void PlayerInventoryController::inventoryQuestItemRemoved(ObjectId itemId)
{
    evQuestItemRemoved(itemId);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncRemoveQuestItem(accountId_, characterId_, itemId);
}


void PlayerInventoryController::inventoryQuestItemUsableCountUpdated(ObjectId itemId, uint8_t usableCount)
{
    evQuestItemUsableCountUpdated(itemId, usableCount);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUpdateQuestItemUsableCount(accountId_, characterId_, itemId,
        usableCount);
}


void PlayerInventoryController::inventoryQuestItemCountUpdated(ObjectId itemId, uint8_t stackCount)
{
	evQuestItemCountUpdated(itemId, stackCount);

	DatabaseGuard db(SNE_DATABASE_MANAGER);
	db->asyncUpdateQuestItemCount(accountId_, characterId_, itemId, stackCount);
}


RECEIVE_SRPC_METHOD_3(PlayerInventoryController, moveInventoryItem,
    InvenType, invenType, ObjectId, itemId, SlotId, slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canInventoryChangeState()) {
        errorCode =  ecItemCannotChangeInventory;
    }
    
    if (isSucceeded(errorCode)) {
        if (invenType == itPc) {
            errorCode = owner.queryInventoryable()->getInventory().moveItem(itemId, slotId);
        }
        else if (invenType == itBank) {
            errorCode = owner.queryInventoryable()->getBankAccount().moveItem(itemId, slotId);
        }
		else if (invenType == itVehicle) {
			errorCode = owner.queryInventoryable()->getVehicleInventory().moveItem(itemId, slotId);
		}
        else {
            errorCode = ecInventoryInvalidInvenType;
        }
    }
    
    if (isFailed(errorCode)) {
        onMoveInventoryItem(errorCode, invenType, itemId, slotId);
    }
}


RECEIVE_SRPC_METHOD_3(PlayerInventoryController, switchInventoryItem,
    InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canInventoryChangeState()) {
        errorCode =  ecItemCannotChangeInventory;
    }

    if (isSucceeded(errorCode)) {
        if (invenType == itPc) {
            errorCode = owner.queryInventoryable()->getInventory().switchItem(itemId1, itemId2);
        }
        else if (invenType == itBank) {
            errorCode = owner.queryInventoryable()->getBankAccount().switchItem(itemId1, itemId2);
        }
		else if (invenType == itVehicle) {
			errorCode = owner.queryInventoryable()->getVehicleInventory().switchItem(itemId1, itemId2);
		}
        else {
            errorCode = ecInventoryInvalidInvenType;
        }
    }

    if (isFailed(errorCode)) {
        onSwitchInventoryItem(errorCode, invenType, itemId1, itemId2);
    }
}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, removeInventoryItem,
    InvenType, invenType, ObjectId, itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canInventoryChangeState()) {
        errorCode =  ecItemCannotChangeInventory;
    }

    if (isSucceeded(errorCode)) {
        if (invenType == itPc) {
            errorCode = owner.queryInventoryable()->getInventory().removeItem(itemId);
        }
        else if (invenType == itQuest) {
            errorCode = owner.queryInventoryable()->getQuestInventory().removeItem(itemId);
        }
        else if (invenType == itBank) {
            errorCode = ecItemQuestItemNotRemove;
        }
		else if (invenType == itVehicle) {
			errorCode = owner.queryInventoryable()->getVehicleInventory().removeItem(itemId);
		}
        else {
            errorCode = ecInventoryInvalidInvenType;
        }
    }

    if (isFailed(errorCode)) {
        onRemoveInventoryItem(errorCode, invenType, itemId);
    }

	SNE_LOG_INFO("GameLog PlayerInventoryController::removeInventoryItem(Ec:%u, PID:%" PRIu64 ", itemId:%" PRIu64 "",
		errorCode, owner.getObjectId(), itemId);
}


RECEIVE_SRPC_METHOD_1(PlayerInventoryController, equipItem,
    ObjectId, itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canEquipOrUnEquipState()) {
        errorCode = ecItemCannotEquipOrUnEquipState;
    }

    if (isSucceeded(errorCode)) {
        EquipCode equipCode = invalidEquipCode;
        errorCode = owner.queryInventoryable()->getEquipInventory().equip(equipCode, itemId);
        if (isSucceeded(errorCode)) {
            assert(isValidEquipCode(equipCode));
            auto event = std::make_shared<EquipItemEvent>(owner.getGameObjectInfo(), equipCode);
            owner.queryKnowable()->broadcast(event, true);
        }
    }

    if (isFailed(errorCode)) {
        onEquipItem(errorCode, itemId);
    }
}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, unequipItem,
    ObjectId, itemId, SlotId, slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canEquipOrUnEquipState()) {
        errorCode = ecItemCannotEquipOrUnEquipState;
    }

    if (isSucceeded(errorCode)) {
        SlotId invenSlotId = slotId;
        EquipCode equipCode = invalidEquipCode;
        errorCode = owner.queryInventoryable()->getEquipInventory().unequip(equipCode, invenSlotId, itemId);
        if (isSucceeded(errorCode)) {
            assert(isValidEquipCode(equipCode));
            auto event = std::make_shared<UnequipItemEvent>(owner.getGameObjectInfo(), equipCode);
            owner.queryKnowable()->broadcast(event, true);
        }
    }

    if (isFailed(errorCode)) {
        onUnequipItem(errorCode, itemId, slotId);
    }
}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, equipAccessoryItem,
    ObjectId, itemId, AccessoryIndex, index)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canEquipOrUnEquipState()) {
        errorCode = ecItemCannotEquipOrUnEquipState;
    }
    if (isSucceeded(errorCode)) {
        errorCode = owner.queryInventoryable()->getAccessoryInventory().equipAccessory(itemId, index);
    }
    
    if (isFailed(errorCode)) {
        onEquipAccessoryItem(errorCode, itemId, index);
    }
}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, unequipAccessoryItem,
    ObjectId, itemId, SlotId, slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canEquipOrUnEquipState()) {
        errorCode = ecItemCannotEquipOrUnEquipState;
    }
    
    if (isSucceeded(errorCode)) {
        errorCode = owner.queryInventoryable()->getAccessoryInventory().unequipAccessory(itemId, slotId);
    }
    if (isFailed(errorCode)) {
        onUnequipAccessoryItem(errorCode, itemId, slotId);
    }
}


RECEIVE_SRPC_METHOD_4(PlayerInventoryController, divideItem,
	InvenType, invenType, ObjectId, itemId, uint8_t, count, SlotId, slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryItemManageState()->canInventoryChangeState()) {
        errorCode =  ecItemCannotChangeInventory;
    }

    if (isSucceeded(errorCode)) {
        if (invenType == itPc) {
            errorCode = owner.queryInventoryable()->getInventory().divideItem(itemId, count, slotId);
        }
        else if (invenType == itBank) {
            errorCode = owner.queryInventoryable()->getBankAccount().divideItem(itemId, count, slotId);
        }
		else if (invenType == itVehicle) {
			errorCode = owner.queryInventoryable()->getVehicleInventory().divideItem(itemId, count, slotId);
		}
        else {
            errorCode = ecInventoryInvalidInvenType;
        }
    }

    if (isFailed(errorCode)) {
        onDivideItem(errorCode, invenType, itemId, count);
    }
}


RECEIVE_SRPC_METHOD_3(PlayerInventoryController, movePlayerInvenItemToBankInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, SlotId, bankSlot)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = checkUseBank(gameObjectInfo);
    if (isSucceeded(errorCode)) {
        errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->playerInvenToOutsideInven(
            owner.queryInventoryable()->getInventory(), owner.queryInventoryable()->getBankAccount(), playerItemId, bankSlot);
    }
    if (isFailed(errorCode)) {
        onMovePlayerInvenItemToBankInvenItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_3(PlayerInventoryController, moveBankInvenItemToPlayerInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, bankItemId, SlotId, playerInvenSlot)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = checkUseBank(gameObjectInfo);
    if (isSucceeded(errorCode)) {
        errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->outsideInvenToPlayerInven(
            owner.queryInventoryable()->getInventory(), owner.queryInventoryable()->getBankAccount(), bankItemId, playerInvenSlot);
    }
    if (isFailed(errorCode)) {
        onMoveBankInvenItemToPlayerInvenItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_3(PlayerInventoryController, switchBankInvenItemAndPlayerInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, ObjectId, bankItemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    ErrorCode errorCode = checkUseBank(gameObjectInfo);
    if (isSucceeded(errorCode)) {
        errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->switchStaticObjectAndPlayerInven(
            owner.queryInventoryable()->getInventory(), owner.queryInventoryable()->getBankAccount(), playerItemId, bankItemId);
    }
    if (isFailed(errorCode)) {
        onSwitchBankInvenItemAndPlayerInvenItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, moveInvenItemToVehicleInvenItem,
	ObjectId, playerItemId, SlotId, vehicleInvenSlot)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Player& owner = getOwnerAs<go::Player>();
	if (! owner.canRequest()) {
		return;
	}

	const ErrorCode errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->playerInvenToOutsideInven(
		owner.queryInventoryable()->getInventory(), owner.queryInventoryable()->getVehicleInventory(), playerItemId, vehicleInvenSlot);
	
	if (isFailed(errorCode)) {
		onMovePlayerInvenItemToBankInvenItem(errorCode);
	}

}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, moveVehicleInvenItemToPlayerInvenItem,
	ObjectId, vehicleItemId, SlotId, playerInvenSlot)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Player& owner = getOwnerAs<go::Player>();
	if (! owner.canRequest()) {
		return;
	}

	const ErrorCode errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->outsideInvenToPlayerInven(
		owner.queryInventoryable()->getInventory(), owner.queryInventoryable()->getVehicleInventory(), vehicleItemId, playerInvenSlot);
	
	if (isFailed(errorCode)) {
		onMoveBankInvenItemToPlayerInvenItem(errorCode);
	}

}

RECEIVE_SRPC_METHOD_2(PlayerInventoryController, switchVehicleInvenItemAndPlayerInvenItem,
	ObjectId, playerItemId, ObjectId, vehicleItemId)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Player& owner = getOwnerAs<go::Player>();
	if (! owner.canRequest()) {
		return;
	}

	const ErrorCode errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->switchStaticObjectAndPlayerInven(
		owner.queryInventoryable()->getInventory(), owner.queryInventoryable()->getVehicleInventory(), playerItemId, vehicleItemId);
	
	if (isFailed(errorCode)) {
		onSwitchBankInvenItemAndPlayerInvenItem(errorCode);
	}
}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, depositGameMoney,
    GameObjectInfo, targetInfo, GameMoney, invenGameMoney)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = checkUseBank(targetInfo);
    if (isFailed(errorCode)) {
        onDepositGameMoney(errorCode, 0, 0);
        return;
    }

    GameMoney gameMoney = owner.queryMoneyable()->getGameMoney();
    if (invenGameMoney > gameMoney) {
        onDepositGameMoney(ecBankDipositNotEnoughGameMoney, 0, 0);
        return;
    }

    owner.queryMoneyable()->downGameMoney(invenGameMoney);
    BankAccount& bank = owner.queryInventoryable()->getBankAccount();
    bank.depositMoney(invenGameMoney);
    onDepositGameMoney(ecOk, bank.getGameMoney(), owner.queryMoneyable()->getGameMoney());
}


RECEIVE_SRPC_METHOD_2(PlayerInventoryController, withdrawGameMoney,
    GameObjectInfo, targetInfo, GameMoney, bankGameMoney)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = checkUseBank(targetInfo);
    if (isFailed(errorCode)) {
        onWithdrawGameMoney(errorCode, 0, 0);
        return;
    }

    BankAccount& bank = owner.queryInventoryable()->getBankAccount();

    GameMoney gameMoney = bank.getGameMoney();
    if (bankGameMoney > gameMoney) {
        onWithdrawGameMoney(ecBankWithdrawNotEnoughGameMoney, 0, 0);
        return;
    }

    owner.queryMoneyable()->upGameMoney(bankGameMoney);
    bank.withdrawMoney(bankGameMoney);
    onWithdrawGameMoney(ecOk, bank.getGameMoney(), owner.queryMoneyable()->getGameMoney());
}



FORWARD_SRPC_METHOD_4(PlayerInventoryController, onMoveInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_4(PlayerInventoryController, onSwitchInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, onRemoveInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, onEquipItem,
    ErrorCode, errorCode, ObjectId, itemId);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, onUnequipItem,
    ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_4(PlayerInventoryController, onDivideItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, uint8_t, count);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evItemEquipped,
    GameObjectInfo, creatureInfo, EquipCode, equipCode);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evItemUnequipped,
    GameObjectInfo, creatureInfo, EquipCode, equipCode);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, onEquipAccessoryItem,
    ErrorCode, errorCode, ObjectId, itemId, AccessoryIndex, index);

/// 악세사리 탈착
FORWARD_SRPC_METHOD_3(PlayerInventoryController, onUnequipAccessoryItem,
    ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, onMovePlayerInvenItemToBankInvenItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, onMoveBankInvenItemToPlayerInvenItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, onSwitchBankInvenItemAndPlayerInvenItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, onMoveInvenItemToVehicleInvenItem,
	ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, onMoveVehicleInvenItemToPlayerInvenItem,
	ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, onSwitchVehicleInvenItemAndPlayerInvenItem,
	ErrorCode, errorCode);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, onDepositGameMoney,
    ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, onWithdrawGameMoney,
    ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney);


/// 자신만 받는 이벤트
FORWARD_SRPC_METHOD_3(PlayerInventoryController, evInventoryItemCountUpdated,
    InvenType, invenType, ObjectId, itemId, uint8_t, ItemCount);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evInventoryItemAdded,
    InvenType, invenType, ItemInfo, itemInfo);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, evInventoryEquipItemChanged,
    ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, evEquipItemEnchanted,
    ObjectId, itemId, SocketSlotId, id, EquipSocketInfo, socketInfo);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evEquipItemUnenchanted,
    ObjectId, itemId, SocketSlotId, id);



FORWARD_SRPC_METHOD_2(PlayerInventoryController, evInventoryItemRemoved,
    InvenType, invenType, ObjectId, itemId);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, evInventoryItemMoved,
    InvenType, invenType, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, evInventoryItemSwitched,
    InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, evUnequipItemReplaced,
    ObjectId, itemId, SlotId, slotId, EquipPart, unequipPrt);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evEquipItemReplaced,
    ObjectId, itemId, EquipPart, equipPrt);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evInventoryWithEquipItemReplaced,
    ObjectId, itemId1, ObjectId, itemId2);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, evUnequipAccessoryItemReplaced, 
    ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPrt);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evEquipAccessoryItemReplaced,
    ObjectId, itemId, AccessoryIndex, equipPrt);


FORWARD_SRPC_METHOD_4(PlayerInventoryController, evInventoryWithAccessoryItemReplaced,
    ObjectId, unequipItemId, AccessoryIndex, unequipIndex,
    ObjectId, equipItemId, AccessoryIndex, equipIndex);


FORWARD_SRPC_METHOD_3(PlayerInventoryController, evInventoryInfoUpdated, 
    InvenType, invenTpe, bool, isCashSlot, uint8_t, currentExtendSlot);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, evQuestItemAdded, 
    QuestItemInfo, questItemInfo);


FORWARD_SRPC_METHOD_1(PlayerInventoryController, evQuestItemRemoved,
    ObjectId, itemId);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evQuestItemUsableCountUpdated,
    ObjectId, itemId, uint8_t, usableCount);


FORWARD_SRPC_METHOD_2(PlayerInventoryController, evQuestItemCountUpdated,
    ObjectId, itemId, uint8_t, stackCount);

// = sne::srpc::RpcForwarder overriding

void PlayerInventoryController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerInventoryController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerInventoryController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerInventoryController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


ErrorCode PlayerInventoryController::checkUseBank(const GameObjectInfo& objectInfo)
{
    if (! (objectInfo.isNpc() || objectInfo.isBuilding())) {
        return ecBankNotFindBankObject;
    }

    go::Entity* entity = getOwner().queryKnowable()->getEntity(objectInfo);
    if (! entity) {
        return ecBankNotAbilltyBankObject;
    }

    go::Bankable* bankable = entity->queryBankable();
    if (! bankable) {
        return ecBankNotAbilltyBankObject;
    }

    return bankable->canBankable(getOwner());
}



}}} // namespace gideon { namespace zoneserver { namespace gc {
