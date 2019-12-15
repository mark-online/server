#include "ZoneServerPCH.h"
#include "PlayerOutsideInventoryController.h"
#include "../../model/gameobject/Building.h"
#include "../../model/gameobject/ability/OutsideInventoryable.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/item/Inventory.h"
#include "../../model/state/ItemManageState.h"
#include "../../model/bank/BankAccount.h"
#include "../../service/anchor/AnchorService.h"
#include "../../service/inven/OrderInvenItemMoveService.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include <gideon/cs/datatable/BuildingTable.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerOutsideInventoryController);

PlayerOutsideInventoryController::PlayerOutsideInventoryController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerOutsideInventoryController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerOutsideInventoryController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerOutsideInventoryController::guildInventoryItemPushed(ErrorCode errorCode)
{
    onPushGuildInventoryItem(errorCode);
}


void PlayerOutsideInventoryController::guildInventoryItemPopped(ErrorCode errorCode)
{
    onPopGuildInventoryItem(errorCode);
}


void PlayerOutsideInventoryController::guildInventoryItemSwitched(ErrorCode errorCode)
{
    onSwitchGuildInventoryItem(errorCode);
}


void PlayerOutsideInventoryController::guildGameMoneyDepositted(ErrorCode errorCode, GameMoney gameMoney)
{
    go::Entity& owner = getOwner();
    if (isSucceeded(errorCode)) {
        owner.queryMoneyable()->downGameMoney(gameMoney);
    }
    onDepositGuildGameMoney(errorCode, owner.queryMoneyable()->getGameMoney());
}


void PlayerOutsideInventoryController::guildGameMoneyWithdrawed(ErrorCode errorCode, sec_t resetDayWithdrawTime, GameMoney todayWithdraw, GameMoney gameMoney)
{
    go::Entity& owner = getOwner();
    if (isSucceeded(errorCode)) {
        owner.queryMoneyable()->upGameMoney(gameMoney);
    }
    onWithdrawGuildGameMoney(errorCode, resetDayWithdrawTime, todayWithdraw, owner.queryMoneyable()->getGameMoney());
}

// = rpc::OutsideInventoryRpc overriding

RECEIVE_SRPC_METHOD_4(PlayerOutsideInventoryController, movePlayerInvenItemToOutsideInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, InvenType, buildInvenType, SlotId, buildInvenSlot)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    Inventory* buildingInventory = nullptr;
    ErrorCode errorCode = checkAddOutsideInvenItem(gameObjectInfo, playerItemId, buildInvenType);
    if (isSucceeded(errorCode)) {
        errorCode = getOutsideInventory(buildingInventory, gameObjectInfo, false, buildInvenType);
        if (isSucceeded(errorCode)) {
            errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->playerInvenToOutsideInven(
                owner.queryInventoryable()->getInventory(), *buildingInventory, playerItemId, buildInvenSlot);
        }
    }
    
    if (isFailed(errorCode)) {
        onMovePlayerInvenItemToOutsideInvenItem(errorCode);
    }

}


RECEIVE_SRPC_METHOD_4(PlayerOutsideInventoryController, moveOutsideInvenItemToPlayerInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, buildingItemId, InvenType, buildInvenType,
    SlotId, playerInvenSlot)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = checkOutOutsideInvenItem(gameObjectInfo, buildInvenType);
    if (isSucceeded(errorCode)) {
        Inventory* buildingInventory = nullptr;
        errorCode = getOutsideInventory(buildingInventory, gameObjectInfo, false, buildInvenType);
        if (isSucceeded(errorCode)) {
            errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->outsideInvenToPlayerInven(
                owner.queryInventoryable()->getInventory(), *buildingInventory, buildingItemId, playerInvenSlot);
        }
    }
    
    if (isFailed(errorCode)) {
        onMoveOutsideInvenItemToPlayerInvenItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_4(PlayerOutsideInventoryController, switchOutsideInvenItemAndPlayerInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, InvenType, buildInvenType,
    ObjectId, buildingItemId)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    Inventory* buildingInventory = nullptr;
    ErrorCode errorCode = getOutsideInventory(buildingInventory, gameObjectInfo, false, buildInvenType);
    if (isSucceeded(errorCode)) {
        errorCode = ORDER_INVEN_ITEM_MOVE_SERVICE->switchStaticObjectAndPlayerInven(
            owner.queryInventoryable()->getInventory(), *buildingInventory, playerItemId, buildingItemId);
    }
    if (isFailed(errorCode)) {
        onSwitchOutsideInvenItemAndPlayerInvenItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_4(PlayerOutsideInventoryController, moveItemOutsideInventory,
    GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId,
    SlotId, moveSlotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    Inventory* inventory = nullptr;
    ErrorCode errorCode = getOutsideInventory(inventory, gameObjectInfo, false, invenType);
    if (isSucceeded(errorCode)) {
        errorCode =inventory->moveItem(itemId, moveSlotId);
    }
    if (isFailed(errorCode)) {
        onMoveItemOutsideInventory(errorCode);
    }
}


RECEIVE_SRPC_METHOD_4(PlayerOutsideInventoryController, switchOutsideInventoryItem,
    GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    Inventory* inventory = nullptr;
    ErrorCode errorCode = getOutsideInventory(inventory, gameObjectInfo, false, invenType);
    if (isSucceeded(errorCode)) {
        errorCode =inventory->switchItem(itemId1, itemId2);
    }
    if (isFailed(errorCode)) {
        onSwitchOutsideInventoryItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_3(PlayerOutsideInventoryController, removeOutsideInventoryItem,
    GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    Inventory* inventory = nullptr;
    ErrorCode errorCode = getOutsideInventory(inventory, gameObjectInfo, false, invenType);
    if (isSucceeded(errorCode)) {
        errorCode =inventory->removeItem(itemId);
    }
    if (isFailed(errorCode)) {
        onRemoveOutsideInventoryItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_5(PlayerOutsideInventoryController, divideOutsideInventoryItem,
    GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId, uint8_t, count, SlotId, slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    
    Inventory* inventory = nullptr;
    ErrorCode errorCode = getOutsideInventory(inventory, gameObjectInfo, false, invenType);
    if (isSucceeded(errorCode)) {
        errorCode =inventory->divideItem(itemId, count, slotId);
    }

    if (isFailed(errorCode)) {
        onDivideOutsideInventoryItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_2(PlayerOutsideInventoryController, openOutsideInventory,
    GameObjectInfo, gameObjectInfo, InvenType, invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    Inventory* inventory = nullptr;
    const ErrorCode errorCode = getOutsideInventory(inventory, gameObjectInfo, true, invenType);
    if (isSucceeded(errorCode)) {
        onOpenOutsideInventory(ecOk, gameObjectInfo, invenType, inventory->getInventoryInfo());
    }
    else {
        onOpenOutsideInventory(errorCode, gameObjectInfo, invenType, InventoryInfo());
    }
}


RECEIVE_SRPC_METHOD_1(PlayerOutsideInventoryController, closeOutsideInventory,
    InvenType, invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    owner.queryInventoryable()->closeOutsideInventory(invenType);
}


RECEIVE_SRPC_METHOD_4(PlayerOutsideInventoryController, pushGuildInventoryItem,
    GameObjectInfo, gameObjectInfo, ObjectId, myItemId, VaultId, vaultId, SlotId, guildSlotId)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    // 거리검사, lock
    gameObjectInfo;
    ItemInfo itemInfo = owner.queryInventoryable()->getInventory().getItemInfo(myItemId);
    if (! itemInfo.isValid()) {
        onPushGuildInventoryItem(ecInventoryItemNotFound);
        return;
    }

    if (! ZONE_SERVICE->getCommunityServerProxy().isActivated()) {
        onPushGuildInventoryItem(ecCommunityIsBusy);
        return;
    }

    if (owner.queryItemManageState()->isInventoryLock()) {
        onPushGuildInventoryItem(ecInventoryLock);
        return;
    }

    owner.queryItemManageState()->lockLnventory();
    
    ZONE_SERVICE->getCommunityServerProxy().z2m_pushGuildInventoryItem(owner.getAccountId(),
        owner.getObjectId(), itemInfo, vaultId, guildSlotId);
    
}


RECEIVE_SRPC_METHOD_5(PlayerOutsideInventoryController, popGuildInventoryItem,
    GameObjectInfo, gameObjectInfo, VaultId, vaultId, ObjectId, guildItemId, SlotId, mySlotId, BaseItemInfo, baseItemInfo)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    // 거리검사
    gameObjectInfo;

    if (isValidSlotId(mySlotId)) {
        const ErrorCode errorCode = owner.queryInventoryable()->getInventory().checkAddSlot(mySlotId);
        if (isFailed(errorCode)) {
            onPopGuildInventoryItem(errorCode);
            return;
        }
    }
    else {
        if (! owner.queryInventoryable()->getInventory().canAddItemByBaseItemInfo(baseItemInfo)) {
            onPopGuildInventoryItem(ecInventoryIsFull);
            return;
        }
    }

    if (! ZONE_SERVICE->getCommunityServerProxy().isActivated()) {
        onPopGuildInventoryItem(ecCommunityIsBusy);
        return;
    }
    
    if (owner.queryItemManageState()->isInventoryLock()) {
        onPopGuildInventoryItem(ecInventoryLock);
        return;
    }

    owner.queryItemManageState()->lockLnventory();

    ZONE_SERVICE->getCommunityServerProxy().z2m_popGuildInventoryItem(owner.getAccountId(), 
        owner.getObjectId(), guildItemId, vaultId, mySlotId);
}


RECEIVE_SRPC_METHOD_4(PlayerOutsideInventoryController, switchGuildInventoryItem,
    GameObjectInfo, gameObjectInfo, VaultId, vaultId, ObjectId, guildItemId, ObjectId, myItemId)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    gameObjectInfo;
    ItemInfo itemInfo = owner.queryInventoryable()->getInventory().getItemInfo(myItemId);
    if (! itemInfo.isValid()) {
        onSwitchGuildInventoryItem(ecInventoryItemNotFound);
        return;
    }

    if (! ZONE_SERVICE->getCommunityServerProxy().isActivated()) {
        onSwitchGuildInventoryItem(ecCommunityIsBusy);
        return;
    }

    if (owner.queryItemManageState()->isInventoryLock()) {
        onSwitchGuildInventoryItem(ecInventoryLock);
        return;
    }

    owner.queryItemManageState()->lockLnventory();

    ZONE_SERVICE->getCommunityServerProxy().z2m_switchGuildInventoryItem(owner.getAccountId(),
        owner.getObjectId(), vaultId, guildItemId, itemInfo);    
}


RECEIVE_SRPC_METHOD_2(PlayerOutsideInventoryController, depositGuildGameMoney,
    GameObjectInfo, gameObjectInfo, GameMoney, gameMoney)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    gameObjectInfo;
    if (! ZONE_SERVICE->getCommunityServerProxy().isActivated()) {
        onDepositGuildGameMoney(ecCommunityIsBusy, 0);
        return;
    }

    if (owner.queryItemManageState()->isInventoryLock()) {
        onDepositGuildGameMoney(ecInventoryLock, 0);
        return;
    }

    if (owner.queryMoneyable()->getGameMoney() < gameMoney) {
        onDepositGuildGameMoney(ecBankDipositNotEnoughGameMoney, 0);
        return;
    }

    owner.queryItemManageState()->lockLnventory();

    ZONE_SERVICE->getCommunityServerProxy().z2m_depositGuildGameMoney(owner.getAccountId(), 
        owner.getObjectId(), gameMoney);
}


RECEIVE_SRPC_METHOD_2(PlayerOutsideInventoryController, withdrawGuildGameMoney,
    GameObjectInfo, gameObjectInfo, GameMoney, gameMoney)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    gameObjectInfo;
    if (! ZONE_SERVICE->getCommunityServerProxy().isActivated()) {
        onWithdrawGuildGameMoney(ecCommunityIsBusy, 0, 0, 0);
        return;
    }

    if (owner.queryItemManageState()->isInventoryLock()) {
        onWithdrawGuildGameMoney(ecInventoryLock, 0, 0, 0);
        return;
    }

    owner.queryItemManageState()->lockLnventory();

    ZONE_SERVICE->getCommunityServerProxy().z2m_withdrawGuildGameMoney(owner.getAccountId(), 
        owner.getObjectId(), gameMoney);
}


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onMovePlayerInvenItemToOutsideInvenItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onMoveOutsideInvenItemToPlayerInvenItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onSwitchOutsideInvenItemAndPlayerInvenItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onMoveItemOutsideInventory,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onSwitchOutsideInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onRemoveOutsideInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onDivideOutsideInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_4(PlayerOutsideInventoryController, onOpenOutsideInventory,
    ErrorCode, errorCode, GameObjectInfo, gameObjectInfo, InvenType, invenType,
    InventoryInfo, inventoryInfo);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onPushGuildInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onPopGuildInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerOutsideInventoryController, onSwitchGuildInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(PlayerOutsideInventoryController, onDepositGuildGameMoney,
    ErrorCode, errorCode, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_4(PlayerOutsideInventoryController, onWithdrawGuildGameMoney,
    ErrorCode, errorCode, sec_t, resetDayWithdrawTime, GameMoney, todayWithdraw, GameMoney, gameMoney);


// = sne::srpc::RpcForwarder overriding

void PlayerOutsideInventoryController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerOutsideInventoryController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


ErrorCode PlayerOutsideInventoryController::getOutsideInventory(Inventory*& buildingInventory,
    const GameObjectInfo& gameObjectInfo, bool isOpen, InvenType invenType)
{    
    // TODO 거리 체크
    if (! isOpen) {
        if (invenType == itBuildingProduction && invenType == itBuildingSelectProduction) {
            return ecAnchorInvalidInvenType;
        }
    }

    go::Building* building = ANCHOR_SERVICE->getBuilding(gameObjectInfo);
    if (! building) {
        return ecAnchorNotFind;
    }

    go::OutsideInventoryable* inventoryable = building->queryOutsideInventoryable();
    if (!inventoryable) {
        return ecAnchorNotExistInventory;
    }

    go::Entity& owner = getOwner();
    const ErrorCode errorCode = inventoryable->checkInventoryUseable(invenType, owner);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    
    buildingInventory = inventoryable->getInventory(invenType);
    owner.queryInventoryable()->openOutsideInventory(invenType);
    return ecOk;            
}


ErrorCode PlayerOutsideInventoryController::checkAddOutsideInvenItem(const GameObjectInfo& gameObjectInfo,
    ObjectId playerItemId, InvenType invenType)
{
    go::Building* building = ANCHOR_SERVICE->getBuilding(gameObjectInfo);
    if (! building) {
        return ecAnchorNotFind;
    }

    go::OutsideInventoryable* inventoryable = building->queryOutsideInventoryable();
    if (! inventoryable) {
        return ecAnchorNotExistInventory;
    }

    go::Entity& owner = getOwner();
    DataCode itemCode = owner.queryInventoryable()->getInventory().getItemCode(playerItemId);
    if (! isValidDataCode(itemCode)) {
        return ecInventoryItemNotFound;
    }
    return inventoryable->canAddItem(invenType, itemCode);
}


ErrorCode PlayerOutsideInventoryController::checkOutOutsideInvenItem(const GameObjectInfo& gameObjectInfo, 
    InvenType invenType)
{
    go::Building* building = ANCHOR_SERVICE->getBuilding(gameObjectInfo);
    if (! building) {
        return ecAnchorNotFind;
    }
    go::OutsideInventoryable* inventoryable = building->queryOutsideInventoryable();
    if (! inventoryable) {
        return ecAnchorNotExistInventory;
    }

    return inventoryable->canOutItem(invenType, getOwner());
}


}}} // namespace gideon { namespace zoneserver { namespace gc {