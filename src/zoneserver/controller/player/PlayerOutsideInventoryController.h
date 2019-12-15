#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/OutsideInventoryCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/OutsideInventoryRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver {
class Inventory;
}} // namespace gideon { namespace zoneserver


namespace gideon { namespace zoneserver { namespace go {
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerOutsideInventoryController
 * 외부 인벤토리 담당
 */
class ZoneServer_Export PlayerOutsideInventoryController : public Controller,
    public rpc::OutsideInventoryRpc,
    public OutsideInventoryCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerOutsideInventoryController);
public:
    PlayerOutsideInventoryController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    virtual void guildInventoryItemPushed(ErrorCode errorCode);
    virtual void guildInventoryItemPopped(ErrorCode errorCode);
    virtual void guildInventoryItemSwitched(ErrorCode errorCode);
    virtual void guildGameMoneyDepositted(ErrorCode errorCode, GameMoney gameMoney);
    virtual void guildGameMoneyWithdrawed(ErrorCode errorCode, sec_t resetDayWithdrawTime, GameMoney todayWithdraw, GameMoney gameMoney);
public:
    // = rpc::OutsideInventoryRpc overriding
    OVERRIDE_SRPC_METHOD_4(movePlayerInvenItemToOutsideInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, InvenType, buildInvenType,
        SlotId, buildInvenSlot);
    OVERRIDE_SRPC_METHOD_4(moveOutsideInvenItemToPlayerInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, buildingItemId, InvenType, buildInvenType,
        SlotId, playerInvenSlot);
    OVERRIDE_SRPC_METHOD_4(switchOutsideInvenItemAndPlayerInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, InvenType, buildInvenType,
        ObjectId, buildingItemId);
    OVERRIDE_SRPC_METHOD_4(moveItemOutsideInventory,
        GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId,
        SlotId, moveSlotId);
    OVERRIDE_SRPC_METHOD_4(switchOutsideInventoryItem,
        GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_3(removeOutsideInventoryItem,
        GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_5(divideOutsideInventoryItem,
        GameObjectInfo, gameObjectInfo, InvenType, invenType, ObjectId, itemId, uint8_t, count, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_2(openOutsideInventory,
        GameObjectInfo, gameObjectInfo, InvenType, invenType);
    OVERRIDE_SRPC_METHOD_1(closeOutsideInventory,
        InvenType, invenType);
    OVERRIDE_SRPC_METHOD_4(pushGuildInventoryItem,
        GameObjectInfo, gameObjectInfo, ObjectId, myItemId, VaultId, vaultId, SlotId, guildSlotId);
    OVERRIDE_SRPC_METHOD_5(popGuildInventoryItem,
        GameObjectInfo, gameObjectInfo, VaultId, vaultId, ObjectId, guildItemId, SlotId, mySlotId, BaseItemInfo, baseItemInfo);
    OVERRIDE_SRPC_METHOD_4(switchGuildInventoryItem,
        GameObjectInfo, gameObjectInfo, VaultId, vaultId, ObjectId, guildItemId, ObjectId, myItemId);
    OVERRIDE_SRPC_METHOD_2(depositGuildGameMoney,
        GameObjectInfo, gameObjectInfo, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_2(withdrawGuildGameMoney,
        GameObjectInfo, gameObjectInfo, GameMoney, gameMoney);


    OVERRIDE_SRPC_METHOD_1(onMovePlayerInvenItemToOutsideInvenItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onMoveOutsideInvenItemToPlayerInvenItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onSwitchOutsideInvenItemAndPlayerInvenItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onMoveItemOutsideInventory,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onSwitchOutsideInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onRemoveOutsideInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onDivideOutsideInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_4(onOpenOutsideInventory,
        ErrorCode, errorCode, GameObjectInfo, gameObjectInfo, InvenType, invenType,
        InventoryInfo, inventoryInfo);

    OVERRIDE_SRPC_METHOD_1(onPushGuildInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onPopGuildInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onSwitchGuildInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onDepositGuildGameMoney,
        ErrorCode, errorCode, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_4(onWithdrawGuildGameMoney,
        ErrorCode, errorCode, sec_t, resetDayWithdrawTime, GameMoney, todayWithdraw, GameMoney, gameMoney);

    ErrorCode getOutsideInventory(Inventory*& buildingInventory,
        const GameObjectInfo& gameObjectInfo, bool isOpen, InvenType invenType);
    ErrorCode checkAddOutsideInvenItem(const GameObjectInfo& gameObjectInfo, 
        ObjectId playerItemId, InvenType invenType);
    ErrorCode checkOutOutsideInvenItem(const GameObjectInfo& gameObjectInfo, 
        InvenType invenType);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
