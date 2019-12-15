#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/CraftCallback.h"
#include "../callback/ItemCallback.h"
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/rpc/player/ItemRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/RecipeTable.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {


/**
 * @struct UsingRecipeInfo 
 */
struct UsingRecipeInfo
{
    ObjectId objectId_;
    GameTime startTime_;

    UsingRecipeInfo() :
        objectId_(invalidObjectId),
        startTime_(0) {}
        
    void reset() {
        objectId_ = invalidObjectId;
        startTime_ = 0;
    }
};


/**
 * @class PlayerItemController
 * 아이템 담당
 */
class ZoneServer_Export PlayerItemController : public Controller,
    public rpc::ItemRpc,
    public CraftCallback,
    public ItemCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerItemController);
public:
    PlayerItemController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    virtual void npcCraftComleted(DataCode recipeCode);
    virtual void playerCraftComleted(ObjectId recipeId); 
    virtual void itemReprocessed(ObjectId itemId);
    virtual void equipItemUpgraded(ObjectId equipItemId);
    virtual void equipItemEnchanted(ObjectId equipItemId, ObjectId gemItemId);

    virtual void functionItemUsed(ObjectId itemId);
public:
    // = rpc::ItemRpc overriding
    OVERRIDE_SRPC_METHOD_3(unenchantEquip,
        GameObjectInfo, checkObjectInfo, ObjectId, equipItemId, SocketSlotId, openSlotIndex);

	OVERRIDE_SRPC_METHOD_1(useVehicleItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(useGliderItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(selectVehicleItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(selectGliderItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(repairGliderItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(queryLootItemInfoMap,
		GameObjectInfo, targetInfo);

    OVERRIDE_SRPC_METHOD_4(onUnenchantEquip,
        ErrorCode, errorCode, ObjectId, equipItemId, uint8_t, openSlotIndex, GameMoney, currentGameMoney);
	OVERRIDE_SRPC_METHOD_2(onUseVehicleItem,
		ErrorCode, errorCode, VehicleInfo, info);
	OVERRIDE_SRPC_METHOD_2(onUseGliderItem,
		ErrorCode, errorCode, GliderInfo, info);
	OVERRIDE_SRPC_METHOD_2(onSelectVehicleItem,
		ErrorCode, errorCode, ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_2(onSelectGliderItem,
		ErrorCode, errorCode, ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_3(onRepairGliderItem,
		ErrorCode, errorCode, ObjectId, itemId, GameMoney, currentGameMoney);
	OVERRIDE_SRPC_METHOD_3(onLootItemInfoMap,
		ErrorCode, errorCode, GameObjectInfo, targetInfo, LootInvenItemInfoMap, infoMap);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    ErrorCode reprocessItem(DataCode& itemCode, bool& isSuccess, ObjectId itemId);

	ErrorCode checkUseVehicleItem(VehicleInfo& info, ObjectId itemId);
	ErrorCode checkUseGliderItem(GliderInfo& info, ObjectId itemId);	
    
    ErrorCode addCraftItem(ObjectId& addItemId, const datatable::RecipeTemplate& recipe);
    ErrorCode addNpcCraftItem(ObjectId& addItemId, DataCode recipeCode);
    ErrorCode addPlayerCraftItem(ObjectId& addItemId, ObjectId recipeId);
    ErrorCode applyFunctionScript(DataCode& itemCode, ObjectId itemId);

private:
    typedef sne::core::Map<ElementCode, GameTime> ElementCoolTimes;

    UsingRecipeInfo usingRecipeInfo_;
    ElementCoolTimes coolTimes_;
    GameTime elementCastTime_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
