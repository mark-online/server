#include "ZoneServerPCH.h"
#include "PlayerItemController.h"
#include "../EntityController.h"
#include "../callback/InventoryCallback.h"
#include "../../model/item/Inventory.h"
#include "../../model/item/EquipInventory.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/AbstractAnchor.h"
#include "../../model/gameobject/ability/Castable.h"
#include "../../model/gameobject/ability/ItemCastable.h"
#include "../../model/gameobject/ability/Gliderable.h"
#include "../../model/gameobject/ability/Vehicleable.h"
#include "../../model/gameobject/ability/Storeable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/gameobject/ability/SkillLearnable.h"
#include "../../model/gameobject/ability/SOInventoryable.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/Castnotificationable.h"
#include "../../model/gameobject/skilleffect/CreatureEffectScriptApplier.h"
#include "../../model/state/ItemManageState.h"
#include "../../model/time/GlobalCoolDownTimer.h"
#include "../../service/time/GameTimer.h"
#include "../../service/Item/ItemOptionService.h"
#include "../../service/teleport/TeleportService.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../helper/InventoryHelper.h"
#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/ReprocessTable.h>
#include <gideon/cs/datatable/VehicleTable.h>
#include <gideon/cs/datatable/GliderTable.h>
#include <gideon/cs/datatable/FunctionTable.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {


IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerItemController);

PlayerItemController::PlayerItemController(go::Entity* owner) :
    Controller(owner),
    elementCastTime_(0)
{
}


void PlayerItemController::npcCraftComleted(DataCode recipeCode)
{
    go::Castable* castable = getOwner().queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (! notificationable) {
        return;
    }

    ObjectId addItemId = invalidObjectId;
    const ErrorCode errorCode = addNpcCraftItem(addItemId, recipeCode);
    if (isSucceeded(errorCode)) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(uctPlayerCraft, addItemId, getOwner().getGameObjectInfo());
        notificationable->notifyCompleteCasting(comleteInfo);
    }
    else {
        FailCompletedCastResultInfo failInfo;
        failInfo.setDataInfo(uctNpcCraft, errorCode, getOwner().getGameObjectInfo(), invalidDataCode);
        notificationable->notifyCompleteFailedCasting(failInfo);
    }
}


void PlayerItemController::playerCraftComleted(ObjectId recipeId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (! notificationable) {
        return;
    }
    ObjectId addItemId = invalidObjectId;
    const ErrorCode errorCode = addPlayerCraftItem(addItemId, recipeId);    
    if (isSucceeded(errorCode)) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(uctPlayerCraft, addItemId, getOwner().getGameObjectInfo());
        notificationable->notifyCompleteCasting(comleteInfo);
    }
    else {
        FailCompletedCastResultInfo failInfo;
        failInfo.setDataInfo(uctPlayerCraft, errorCode, getOwner().getGameObjectInfo(), invalidDataCode);
        notificationable->notifyCompleteFailedCasting(failInfo);
    }
}



void PlayerItemController::itemReprocessed(ObjectId itemId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (! notificationable) {
        return;
    }
    bool isSuccess = false;
    DataCode itemCode = invalidDataCode;
    const ErrorCode errorCode = reprocessItem(itemCode, isSuccess, itemId);
    if (isSucceeded(errorCode)) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(getOwner().getGameObjectInfo(), isSuccess, itemCode);
        notificationable->notifyCompleteCasting(comleteInfo);
    }
    else {
        FailCompletedCastResultInfo failInfo;
        failInfo.setDataInfo(uctReprocess, errorCode, getOwner().getGameObjectInfo(), invalidDataCode);
        notificationable->notifyCompleteFailedCasting(failInfo);
    }
}


void PlayerItemController::equipItemUpgraded(ObjectId equipItemId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (! notificationable) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! getOwner().queryItemManageState()->canItemUseableState()) {
        errorCode = ecItemCannotUseableState;
    }

    if (isSucceeded(errorCode)) {
        errorCode = getOwner().queryInventoryable()->getEquipInventory().upgradeEquipItem(equipItemId);     
    }
    
    if (isSucceeded(errorCode)) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(uctEquipUpgrade, getOwner().getGameObjectInfo(), true, equipItemId);
        notificationable->notifyCompleteCasting(comleteInfo);
    }
    else {
        FailCompletedCastResultInfo failInfo;
        failInfo.setDataInfo(uctEquipUpgrade, errorCode, getOwner().getGameObjectInfo(), invalidDataCode);
        notificationable->notifyCompleteFailedCasting(failInfo);
    }
}


void PlayerItemController::equipItemEnchanted(ObjectId equipItemId, ObjectId gemItemId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (! notificationable) {
        return;
    }

    bool isSuccess = false;
    const ErrorCode errorCode = getOwner().queryInventoryable()->getEquipInventory().enchant(isSuccess,
        equipItemId, gemItemId);

    if (isSucceeded(errorCode)) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(uctEquipEnchant, getOwner().getGameObjectInfo(), isSuccess, equipItemId);
        notificationable->notifyCompleteCasting(comleteInfo);
    }
    else {
        FailCompletedCastResultInfo failInfo;
        failInfo.setDataInfo(uctEquipEnchant, errorCode, getOwner().getGameObjectInfo(), invalidDataCode);
        notificationable->notifyCompleteFailedCasting(failInfo);
    }
}


void PlayerItemController::functionItemUsed(ObjectId itemId)
{
    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (! notificationable) {
        return;
    }

    DataCode itemCode = invalidDataCode;
    const ErrorCode errorCode = applyFunctionScript(itemCode, itemId); 
    if (isSucceeded(errorCode)) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(getOwner().getGameObjectInfo(), itemId, itemCode);
        notificationable->notifyCompleteCasting(comleteInfo);
    }
    else {
        FailCompletedCastResultInfo failInfo;
        failInfo.setDataInfo(uctFunctionItem, errorCode, getOwner().getGameObjectInfo(), itemCode);
        notificationable->notifyCompleteFailedCasting(failInfo);
    }
}


void PlayerItemController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerItemController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}

// = rpc::EntityStatusRpc overriding

RECEIVE_SRPC_METHOD_3(PlayerItemController, unenchantEquip,
    GameObjectInfo, checkObjectInfo, ObjectId, equipItemId, SocketSlotId, socketSlotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    checkObjectInfo;
    
    // TODO: npc 검사
	//ErrorCode ecManufacture = checkManufacture(checkObjectInfo);
	//if (isFailed(ecManufacture)) {
	//	onUnenchantEquip(ecManufacture, equipItemId, socketSlotId, owner.queryMoneyable()->getGameMoney());
	//	return;
	//}
    
    const GameMoney fee = 100;
    ErrorCode errorCode = ecOk;
    if (fee <= owner.queryMoneyable()->getGameMoney()) {
        errorCode = owner.queryInventoryable()->getEquipInventory().unenchant(equipItemId, socketSlotId);
        if (isSucceeded(errorCode)) {
            owner.queryMoneyable()->downGameMoney(fee);
        } 
    }
    else {
        errorCode = ecItemUnenchantNotEnoughGameMoney;
    }
    onUnenchantEquip(errorCode, equipItemId, socketSlotId, owner.queryMoneyable()->getGameMoney());

	SNE_LOG_INFO("GameLog PlayerItemController::unenchantEquip(Ec:%u, PID:%" PRIu64 ", EID:%" PRIu64 ")",
		errorCode, owner.getObjectId(), equipItemId);
}


RECEIVE_SRPC_METHOD_1(PlayerItemController, useVehicleItem,
	ObjectId, itemId)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	VehicleInfo info;
	const ErrorCode errorCode = checkUseVehicleItem(info, itemId);
	if (isSucceeded(errorCode)) {
		owner.queryInventoryable()->getInventory().useItemsByItemId(itemId, 1);
		owner.queryVehicleable()->addVehicle(info);
	}
	onUseVehicleItem(errorCode, info);
}


RECEIVE_SRPC_METHOD_1(PlayerItemController, useGliderItem,
	ObjectId, itemId)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	GliderInfo info;
	const ErrorCode errorCode = checkUseGliderItem(info, itemId);
	if (isSucceeded(errorCode)) {
		owner.queryInventoryable()->getInventory().useItemsByItemId(itemId, 1);
		owner.queryGliderable()->addGlider(info);
	}
	onUseGliderItem(errorCode, info);
}


RECEIVE_SRPC_METHOD_1(PlayerItemController, selectVehicleItem,
	ObjectId, itemId)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	const ErrorCode errorCode = owner.queryVehicleable()->selectVehicle(itemId);
	onSelectVehicleItem(errorCode, itemId);
}


RECEIVE_SRPC_METHOD_1(PlayerItemController, selectGliderItem,
	ObjectId, itemId)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	const ErrorCode errorCode = owner.queryGliderable()->selectGlider(itemId);
	onSelectGliderItem(errorCode, itemId);
}


RECEIVE_SRPC_METHOD_1(PlayerItemController, repairGliderItem,
	ObjectId, itemId)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	const ErrorCode errorCode = owner.queryStoreable()->repairGlider(itemId);

	onRepairGliderItem(errorCode, itemId, owner.queryMoneyable()->getGameMoney());
}


RECEIVE_SRPC_METHOD_1(PlayerItemController, queryLootItemInfoMap,
	GameObjectInfo, targetInfo)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	go::Entity* target = owner.queryKnowable()->getEntity(targetInfo);
	if (! target) {
		onLootItemInfoMap(ecEntityNotFound, targetInfo, LootInvenItemInfoMap());
		return;
	}

	go::SOInventoryable* soInventoryable = target->querySOInventoryable();
	if (! soInventoryable) {
		onLootItemInfoMap(ecEntityNotFound, targetInfo, LootInvenItemInfoMap());
		return;
	}
	LootInvenItemInfoMap itemInfoMap = soInventoryable->getLootInvenItemInfoMap();
	onLootItemInfoMap(ecOk, targetInfo, itemInfoMap);
}


FORWARD_SRPC_METHOD_4(PlayerItemController, onUnenchantEquip,
    ErrorCode, errorCode, ObjectId, equipItemId, uint8_t, openSlotIndex, GameMoney, currentGameMoney);


FORWARD_SRPC_METHOD_2(PlayerItemController, onUseVehicleItem,
	ErrorCode, errorCode, VehicleInfo, info);


FORWARD_SRPC_METHOD_2(PlayerItemController, onUseGliderItem,
	ErrorCode, errorCode, GliderInfo, info);


FORWARD_SRPC_METHOD_2(PlayerItemController, onSelectVehicleItem,
	ErrorCode, errorCode, ObjectId, itemId);


FORWARD_SRPC_METHOD_2(PlayerItemController, onSelectGliderItem,
	ErrorCode, errorCode, ObjectId, itemId);


FORWARD_SRPC_METHOD_3(PlayerItemController, onRepairGliderItem,
	ErrorCode, errorCode, ObjectId, itemId, GameMoney, currentGameMoney);


FORWARD_SRPC_METHOD_3(PlayerItemController, onLootItemInfoMap,
	ErrorCode, errorCode, GameObjectInfo, targetInfo, LootInvenItemInfoMap, infoMap);

// = sne::srpc::RpcForwarder overriding

void PlayerItemController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerItemController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerItemController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerItemController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


ErrorCode PlayerItemController::reprocessItem(DataCode& itemCode, bool& isSuccess, ObjectId itemId)
{
    Inventory& inventory = getOwner().queryInventoryable()->getInventory();
    ItemInfo item = inventory.getItemInfo(itemId);
    if (! item.isValid()) {
        return ecInventoryItemNotFound;
    }

    const datatable::ReprocessInfo* reprocessInfo = REPROCESS_TABLE->getReprocessInfo(item.itemCode_);
    if (! reprocessInfo) {
        return ecItemNotReprocessableItem;
    }

    const BaseItemInfos dropItems = reprocessInfo->getReprocessItems();
    if (! dropItems.empty()) {
        if (! inventory.canAddItemByBaseItemInfos(dropItems)) {
            return ecInventoryIsFull;
        }
    }

    if (item.isEquipment()) {
        if (item.isEquipped()) {
            return ecEquipNotReprocess;
        }
        bool shouldRemove = false;
        ErrorCode errorCode = getOwner().queryInventoryable()->getEquipInventory().downgradeEquipItem(shouldRemove, itemId);
        if (isFailed(errorCode)) {
            return errorCode;
        }
        if (shouldRemove) {
            inventory.useItemsByItemId(itemId, 1);
        }
    }
    else {
        inventory.useItemsByItemId(itemId, 1);
    }

    if (! dropItems.empty()) {
        for (const BaseItemInfo& dropItem : dropItems) {
            if (dropItem.isEquipment()) {
                assert(false);
                continue;
            }
            inventory.addItem(createAddItemInfoByBaseItemInfo(dropItem));
        }
        isSuccess = true;
    }
    itemCode = item.itemCode_;
    return ecOk;
}


ErrorCode PlayerItemController::checkUseVehicleItem(VehicleInfo& info, ObjectId itemId)
{
	ItemInfo item = getOwner().queryInventoryable()->getInventory().getItemInfo(itemId);
	if (! item.isValid()) {
		return ecInventoryItemNotFound;
	}

	if (! item.isVehicle()) {
		return ecItemInvalidVelicleItem;
	}

	const gdt::vehicle_t* vehicle = VEHICLE_TABLE->getVehicle(item.itemCode_);
	if (! vehicle) {
		return ecItemInvalidVelicleItem;
	}

	info.objectId_ = itemId;
	info.vehicleCode_ = item.itemCode_;
	info.birthDay_ = getTime();
	info.peakAge_ = uint8_t(esut::random(vehicle->min_peak_time(), vehicle->max_peak_time()));

	return ecOk;
}


ErrorCode PlayerItemController::checkUseGliderItem(GliderInfo& info, ObjectId itemId)
{
	ItemInfo item = getOwner().queryInventoryable()->getInventory().getItemInfo(itemId);
	if (! item.isValid()) {
		return ecInventoryItemNotFound;
	}

	if (! item.isGlider()) {
		return ecItemInvalidGliderItem;
	}

	const gdt::glider_t* glider = GLIDER_TABLE->getGlider(item.itemCode_);
	if (! glider) {
		return ecItemInvalidGliderItem;
	}

	info.objectId_ = itemId;
	info.gliderCode_ = item.itemCode_;
	info.durability_ = glider->glide_durability_sec();

	return ecOk;
}


ErrorCode PlayerItemController::addCraftItem(ObjectId& addItemId, const datatable::RecipeTemplate& recipe)
{
    Inventory& inventory = getOwner().queryInventoryable()->getInventory();
    if (! inventory.checkHasItemsByBaseItemInfos(recipe.getNeedItemInfos())) {
        return ecItemNotEnoughElementItem;
    }

    if (! inventory.canAddItemByBaseItemInfo(BaseItemInfo(recipe.getCompleteItemCode(), 1))) {
        return ecInventoryIsFull;
    }

    inventory.useItemsByBaseItemInfos(recipe.getNeedItemInfos());
    const AddItemInfo completeItem(createAddItemInfoByItemCode(recipe.getCompleteItemCode(), 1));
    return inventory.addItem(addItemId, completeItem);
}


ErrorCode PlayerItemController::addNpcCraftItem(ObjectId& addItemId, DataCode recipeCode)
{
    const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(recipeCode);
    if (! recipe) {
        return ecItemInvalidRecipeItem;
    }

    return addCraftItem(addItemId, *recipe);    
}


ErrorCode PlayerItemController::addPlayerCraftItem(ObjectId& addItemId, ObjectId recipeId)
{
    ItemInfo recipeItem = getOwner().queryInventoryable()->getInventory().getItemInfo(recipeId);
    if (! recipeItem.isValid()) {
        return ecInventoryItemNotFound;
    }

    const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(recipeItem.itemCode_);
    if (! recipe) {
        return ecItemInvalidRecipeItem;
    }

    const ErrorCode errorCode = addCraftItem(addItemId, *recipe);
    if (isSucceeded(errorCode)) {
        getOwner().queryInventoryable()->getInventory().useItemsByItemId(recipeId, 1);
    }

    return errorCode;
}


ErrorCode PlayerItemController::applyFunctionScript(DataCode& itemCode, ObjectId itemId)
{
    Inventory& inventory = getOwner().queryInventoryable()->getInventory();
    ItemInfo functionItem = inventory.getItemInfo(itemId);
    if (! functionItem.isValid()) {
        return ecInventoryItemNotFound;
    }
    itemCode = functionItem.itemCode_;
    const gdt::function_t* function = FUNCTION_TABLE->getFunction(functionItem.itemCode_);
    if (! function) {
        return ecServerInternalError;
    }

    FunctionItemScriptType scriptType = toFunctionItemScriptType(function->item_script_type());
    if (fistExtendFreeInventory == scriptType || fistExtendCashInventory == scriptType ) {
        bool isCashSlot = scriptType == fistExtendCashInventory;
        uint8_t slotExtendCount = static_cast<uint8_t>(function->item_script_value());
        if (! getOwner().queryInventoryable()->canExtendSlot(isCashSlot, slotExtendCount)) {
            return ecItemNotExtendIventorySlot;
        }
        const uint8_t slotCount = getOwner().queryInventoryable()->extendSlot(isCashSlot, slotExtendCount);  
        getOwner().getController().queryInventoryCallback()->inventoryInfoUpdated(itPc, isCashSlot, slotCount, true);
    }
    else {
        return ecServerInternalError;
    }
    
    inventory.useItemsByItemId(itemId, 1);
    return ecOk;
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
