#include "ZoneServerPCH.h"
#include "PlayerCastController.h"
#include "../callback/VehicleCallback.h"
#include "../callback/GliderCallback.h"
#include "../callback/TeleportCallback.h"
#include "../callback/CraftCallback.h"
#include "../callback/ItemCallback.h"
#include "../EntityController.h"
#include "../../model/gameobject/Treasure.h"
#include "../../model/gameobject/Harvest.h"
#include "../../model/gameobject/GraveStone.h"
#include "../../model/gameobject/Device.h"
#include "../../model/gameobject/Creature.h"
#include "../../model/gameobject/skilleffect/EffectScriptApplier.h"
#include "../../model/state/GliderState.h"
#include "../../model/state/VehicleState.h"
#include "../../model/state/CastState.h"
#include "../../model/state/CreatureState.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/Craftable.h"
#include "../../model/gameobject/ability/Castable.h"
#include "../../model/gameobject/ability/SkillCastable.h"
#include "../../model/gameobject/ability/ItemCastable.h"
#include "../../model/gameobject/ability/Castnotificationable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Vehicleable.h"
#include "../../model/gameobject/ability/Gliderable.h"
#include "../../model/gameobject/ability/Teleportable.h"
#include "../../model/gameobject/ability/CastGameTimeable.h"
#include "../../model/gameobject/ability/BuildingProductionInventoryable.h"
#include "../../model/state/TreasureState.h"
#include "../../model/state/HarvestState.h"
#include "../../model/item/Inventory.h"
#include "../../service/distance/DistanceChecker.h"
#include "../../world/WorldMap.h"
#include <gideon/cs/datatable/VehicleTable.h>
#include <gideon/cs/datatable/GliderTable.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/ReprocessTable.h>
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/cs/datatable/FunctionTable.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class VehicleCastingTask
 */
class VehicleCastingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<VehicleCastingTask>
{
public:
	VehicleCastingTask(VehicleCallback& callback) :
		callback_(callback)
	{
	}

private:
    virtual void run() {
		callback_.mountVehicle();
    }

private:
	VehicleCallback& callback_;
};


/**
 * @class GliderCastingTask
 */
class GliderCastingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<GliderCastingTask>
{
public:
    GliderCastingTask(GliderCallback& callback) :
		callback_(callback)
	{
	}

private:
    virtual void run() {
        callback_.mountGlider();
    }

private:
	GliderCallback& callback_;
};


/**
 * @class BindRecallCastingTask
 */
class BindRecallCastingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<BindRecallCastingTask>
{
public:
	BindRecallCastingTask(TeleportCallback& callback, ObjectId bindRecallId) :
		callback_(callback),
		bindRecallId_(bindRecallId)
	{
	}

private:
    virtual void run() {
		callback_.completed(bindRecallId_);
    }

private:
	TeleportCallback& callback_;
	ObjectId bindRecallId_;
};

/**
 * @class NpcCraftTask
 */
class NpcCraftTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<NpcCraftTask>
{
public:
	NpcCraftTask(CraftCallback& callback, DataCode recipeCode) :
		callback_(callback),
		recipeCode_(recipeCode)
	{
	}

private:
    virtual void run() {
		callback_.npcCraftComleted(recipeCode_);
    }

private:
	CraftCallback& callback_;
	DataCode recipeCode_;
};


/**
 * @class PlayerCraftTask
 */
class PlayerCraftTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerCraftTask>
{
public:
	PlayerCraftTask(CraftCallback& callback, ObjectId recipeId) :
		callback_(callback),
		recipeId_(recipeId)
	{
	}

private:
    virtual void run() {
		callback_.playerCraftComleted(recipeId_);
    }

private:
	CraftCallback& callback_;
	ObjectId recipeId_;
};


/**
 * @class ReprocessTask
 */
class ReprocessTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ReprocessTask>
{
public:
	ReprocessTask(CraftCallback& callback, ObjectId itemId) :
		callback_(callback),
		itemId_(itemId)
	{
	}

private:
    virtual void run() {
		callback_.itemReprocessed(itemId_);
    }

private:
	CraftCallback& callback_;
	ObjectId itemId_;
};


/**
 * @class EquipUpgradeTask
 */
class EquipUpgradeTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<EquipUpgradeTask>
{
public:
	EquipUpgradeTask(CraftCallback& callback, ObjectId equipItemId) :
		callback_(callback),
		equipItemId_(equipItemId)
	{
	}

private:
    virtual void run() {
		callback_.equipItemUpgraded(equipItemId_);
    }

private:
	CraftCallback& callback_;
	ObjectId equipItemId_;
};


/**
 * @class EquipEnchantTask
 */
class EquipEnchantTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<EquipEnchantTask>
{
public:
	EquipEnchantTask(CraftCallback& callback, ObjectId equipItemId, ObjectId gemItemId) :
		callback_(callback),
		equipItemId_(equipItemId),
        gemItemId_(gemItemId)
	{
	}

private:
    virtual void run() {
		callback_.equipItemEnchanted(equipItemId_, gemItemId_);
    }

private:
	CraftCallback& callback_;
	ObjectId equipItemId_;
    ObjectId gemItemId_;
};


/**
 * @class UseFunctionItemCastingTask
 */
class UseFunctionItemCastingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<UseFunctionItemCastingTask>
{
public:
    UseFunctionItemCastingTask(ItemCallback& callback, ObjectId itemId) :
		callback_(callback),
        itemId_(itemId)
	{
	}

private:
    virtual void run() {
        callback_.functionItemUsed(itemId_);
    }
    
private:
	ItemCallback& callback_;
    ObjectId itemId_;
};

} // namespace {


// = PlayerCastController
IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerCastController);

void PlayerCastController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerCastController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerCastController::casted(const StartCastResultInfo& startInfo)
{
    evCasted(startInfo);
}


void PlayerCastController::canceled(const CancelCastResultInfo& cancelInfo)
{
    evCastCanceled(cancelInfo);
}


void PlayerCastController::failed(const FailCompletedCastResultInfo& failInfo)
{
    evCastCompleteFailed(failInfo);
}


void PlayerCastController::completed(const CompleteCastResultInfo& completeInfo)
{
    evCastCompleted(completeInfo);
}


// = rpc::SkillRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerCastController, startCasting,
    StartCastInfo, startInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    go::Castable* castable = owner.queryCastable();
    if (castable) {
        if (castable->isCasting()) {
            errorCode = ecCastAlreadyCasting;
        }    
        else if (startInfo.castType_ == uctSkillTo) {
            errorCode = castSkillTo(startInfo.getCastSkillCode(), startInfo.getTargetInfo());
        }
        else if (startInfo.castType_ == uctSkillAt) {
            errorCode = castSkillAt(startInfo.getCastSkillCode(), startInfo.getTargetPosition());
        }
        else if (startInfo.castType_ == uctItemTo) {
            errorCode = castItemTo(startInfo.getItemId(), startInfo.getTargetInfo());
        }
        else if (startInfo.castType_ == uctItemAt) {
            errorCode = castItemAt(startInfo.getItemId(), startInfo.getTargetPosition());
        }
        else if (startInfo.castType_ == uctTreasure) {
            errorCode = openTreasure(startInfo.getTargetInfo());
        }
        else if (startInfo.castType_ == uctHavest) {
            errorCode = startHarvest(startInfo.getTargetInfo());
        }
		else if (startInfo.castType_ == uctDeviceActivation) {
			errorCode = startActiveDevice(startInfo.getTargetInfo());
		}
		else if (startInfo.castType_ == uctVehicle) {
			errorCode = startVehicle();
		}
		else if (startInfo.castType_ == uctGlider) {
			errorCode = startGlider();
		}
		else if (startInfo.castType_ == uctBindRecall) {
			errorCode = startBindRecall(startInfo.getBindRecallId());
		}
		else if (startInfo.castType_ == uctItemLooting) {
			errorCode = startLootingItem(startInfo.getTargetInfo(), startInfo.getLootInvenId());					
		}
        else if (startInfo.castType_ == uctBuildingItemLooting) {
            errorCode = startLootingBuildingItem(startInfo.getTargetInfo(), startInfo.getItemId(), startInfo.getInvenType());			
        }
        else if (startInfo.castType_ == uctNpcCraft) {
            errorCode = startNpcCraft(startInfo.getTargetInfo(), startInfo.getRecipeCode());
        }
        else if (startInfo.castType_ == uctPlayerCraft) {
            errorCode = startPlayerCraft(startInfo.getTargetInfo(), startInfo.getItemId());
        }
        else if (startInfo.castType_ == uctReprocess) {
            errorCode = startReprocess(startInfo.getTargetInfo(), startInfo.getItemId());
        }
        else if (startInfo.castType_ == uctEquipUpgrade) {
            errorCode = startEquipUpgrade(startInfo.getTargetInfo(), startInfo.getEquipItemId());
        }
        else if (startInfo.castType_ == uctEquipEnchant) {
            errorCode = startEquipEnchant(startInfo.getTargetInfo(), startInfo.getEquipItemId(), startInfo.getGemItemId());
        }
        else if (startInfo.castType_ == uctFunctionItem) {
            errorCode = startUseFunctionItem(startInfo.getItemId());
        }
    }
    else {
        errorCode = ecServerInternalError;
    }
    
    if (isSucceeded(errorCode)) {
        if (getOwner().queryCreatureState()->isHidden()) {
            getOwnerAs<go::Creature>().getEffectScriptApplier().cancelRemoveEffect(escHide);
        }
    }

    onStartCasting(errorCode, startInfo);
}


RECEIVE_SRPC_METHOD_0(PlayerCastController, cancelCasting)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }  

    go::Castable* castable = owner.queryCastable();
    if (castable) {
        castable->cancelCasting();
    }
}


FORWARD_SRPC_METHOD_2(PlayerCastController, onStartCasting,
    ErrorCode, errorCode, StartCastInfo, startInfo);


FORWARD_SRPC_METHOD_1(PlayerCastController, evCasted,
    StartCastResultInfo, startInfo);

FORWARD_SRPC_METHOD_1(PlayerCastController, evCastCanceled,
    CancelCastResultInfo, cancelInfo);


FORWARD_SRPC_METHOD_1(PlayerCastController, evCastCompleted,
    CompleteCastResultInfo, completeInfo);


FORWARD_SRPC_METHOD_1(PlayerCastController, evCastCompleteFailed,
    FailCompletedCastResultInfo, failInfo);

// = sne::srpc::RpcForwarder overriding

void PlayerCastController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerCastController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerCastController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerCastController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


ErrorCode PlayerCastController::castSkillTo(SkillCode skillCode, const GameObjectInfo& targetInfo)
{
	go::SkillCastable* skillCastable = getOwner().querySkillCastable();
	if (! skillCastable) {
		return ecServerInternalError;
	}

	const ErrorCode errorCode = skillCastable->checkSkillCasting(skillCode, targetInfo);
	if (isFailed(errorCode)) {
		return errorCode;
	}

	return skillCastable->castTo(targetInfo, skillCode);
}


ErrorCode PlayerCastController::castSkillAt(SkillCode skillCode, const Position& position)
{
	go::SkillCastable* skillCastable = getOwner().querySkillCastable();
	if (! skillCastable) {
		return ecServerInternalError;
	}

	const ErrorCode errorCode = skillCastable->checkSkillCasting(skillCode, position);
	if (isFailed(errorCode)) {
		return errorCode;
	}

	return skillCastable->castAt(position, skillCode);
}


ErrorCode PlayerCastController::castItemTo(ObjectId itemId, const GameObjectInfo& targetInfo)
{
	go::ItemCastable* itemCastable = getOwner().queryItemCastable();
	if (! itemCastable) {
		return ecServerInternalError;
	}

	const ErrorCode errorCode = itemCastable->checkItemCasting(itemId, targetInfo);
	if (isFailed(errorCode)) {
		return errorCode;
	}
	return itemCastable->castTo(targetInfo, itemId);
}


ErrorCode PlayerCastController::castItemAt(ObjectId itemId, const Position& position)
{
	go::ItemCastable* itemCastable = getOwner().queryItemCastable();
	if (! itemCastable) {
		return ecServerInternalError;
	}

	const ErrorCode errorCode = itemCastable->checkItemCasting(itemId, position);
	if (isFailed(errorCode)) {
		return errorCode;
	}
	return itemCastable->castAt(position, itemId);
}



ErrorCode PlayerCastController::openTreasure(const GameObjectInfo& treasueInfo)
{
    TreasureState* treasureState = getOwner().queryTreasureState();
    if (! treasureState) {
        return ecServerInternalError;
    }

    if (! treasureState->canTreasureOpen()) {
        return ecTreasureCannotOpenState;
    }

    go::Entity* target = getOwner().queryKnowable()->getEntity(treasueInfo);
    if (! target) {
        return ecTreasureInvalidTreasure;
    }

	if (! target->isTreasure()) {
		return ecEntityNotFound;
	}

    go::Treasure& treasure = static_cast<go::Treasure&>(*target);
    return treasure.startOpenTreasure(getOwner());
}


ErrorCode PlayerCastController::startHarvest(const GameObjectInfo& harvestInfo)
{
    HarvestState* harvestState = getOwner().queryHarvestState();
    if (! harvestState) {
        return ecServerInternalError;
    }

    if (! harvestState->canHarvest()) {
        return ecHarvestNotHarvestState;
    }

    go::Entity* target = getOwner().queryKnowable()->getEntity(harvestInfo);
    if (! target) {
        return ecEntityNotFound;
    }

	if (! target->isHarvest()) {
		return ecEntityNotFound;
	}

    go::Harvest& harvest = static_cast<go::Harvest&>(*target);
    return harvest.startHarvest(getOwner());
}


ErrorCode PlayerCastController::startActiveDevice(const GameObjectInfo& deviceInfo)
{
	go::Entity* target =
		getOwner().queryKnowable()->getEntity(GameObjectInfo(deviceInfo));
	if (! target) {
		return ecEntityNotFound;
	}

	if (! target->isDevice()) {
		return ecEntityNotFound;
	}

	return static_cast<go::Device*>(target)->activate(getOwnerAs<go::Entity>());
}


ErrorCode PlayerCastController::startVehicle()
{
	go::Entity& owner = getOwner();

    const WorldMap* worldMap = owner.getCurrentWorldMap();
    if (worldMap) {
        MapType mapType = worldMap->getMapType();
        if (mapType == mtRandomDundeon) {
            return ecDungeonNotEnterdVehicleMount;
        }
        else if (mapType == mtArena) {
            return ecArenaNotEnterdVehicleMount;
        }
    }
	EntityVehicleInfo vehicleInfo = owner.queryVehicleable()->getEntityVehicle();
	if (! vehicleInfo.isValid()) {
		return ecVehicleNotSelectVehicle;
	}
	
	VehicleState* state = owner.queryVehicleState();
	if (! state) {
		return ecServerInternalError;
	}

	if (! state->canMountVehicle()) {
		return ecVehicleCannotMountState;
	}

	VehicleCallback* callback = owner.getController().queryVehicleMountCallback();
	if (! callback) {
		return ecServerInternalError;
	}

	go::Castable* castable = owner.queryCastable();
	if (! castable) {
		return ecServerInternalError;
	}
	
	const gdt::vehicle_t* vehicle = VEHICLE_TABLE->getVehicle(vehicleInfo.vehicleCode_);
	if (! vehicle) {
		return ecVehicleInvalidVehicle;
	}

    GameTime castTime = vehicle->cast_time();
	if (castTime > 0) {
		auto task = std::make_unique<VehicleCastingTask>(*callback);
		castable->startCasting(vehicleInfo.vehicleCode_, false, std::move(task), vehicle->cast_time());	

		go::CastNotificationable* notificationable = owner.queryCastNotificationable();
		if (notificationable) {
			StartCastResultInfo startCastInfo;
			startCastInfo.set(owner.getGameObjectInfo(), vehicleInfo.vehicleCode_, vehicleInfo.harnessCode_);
			notificationable->notifyStartCasting(startCastInfo);
		}
	}
	else {
		callback->mountVehicle();
	}

	return ecOk;
}


ErrorCode PlayerCastController::startGlider()
{
	go::Entity& owner = getOwnerAs<go::Entity>();
    const WorldMap* worldMap = owner.getCurrentWorldMap();
    if (worldMap) {
        MapType mapType = worldMap->getMapType();
        if (mapType == mtRandomDundeon) {
            return ecDungeonNotEnterdGliderMount;
        }
        else if (mapType == mtArena) {
            return ecArenaNotEnterdGliderMount;
        }
    }

	go::Gliderable* gliderable = owner.queryGliderable();
	if (! gliderable) {
		return ecServerInternalError;
	}
	GliderCode code = gliderable->getGliderCode();
	if (! isValidGliderCode(code)) {
		return ecVehicleNotSelectVehicle;
	}

	if (gliderable->getGliderDurability() <= 0) {
		return ecGliderZeroDurability;
	}

	GliderState* state = owner.queryGliderState();
	if (! state) {
		return ecServerInternalError;
	}

	if (! state->canReadyMonutGlider()) {
		return ecGliderCannotReadyMountState;
	}

	GliderCallback* callback = owner.getController().queryGliderCallback();
	if (! callback) {
		return ecServerInternalError;
	}

	go::Castable* castable = owner.queryCastable();
	if (! castable) {
		return ecServerInternalError;
	}

	const gdt::glider_t* glider = GLIDER_TABLE->getGlider(code);
	if (! glider) {
		return ecVehicleInvalidGlider;
	}
    
	GameTime castTime = glider->cast_time();
	if (castTime > 0) {
		auto task = std::make_unique<GliderCastingTask>(*callback);
        castable->startCasting(code, false, std::move(task), glider->cast_time());

		go::CastNotificationable* notificationable = owner.queryCastNotificationable();
		if (notificationable) {
			StartCastResultInfo startCastInfo;
			startCastInfo.set(uctGlider, owner.getGameObjectInfo(), code);
			notificationable->notifyStartCasting(startCastInfo);
		}
	}
	else {
		callback->mountGlider();
	}

	return ecOk;
}


ErrorCode PlayerCastController::startBindRecall(ObjectId linkId)
{
	go::Entity& owner = getOwner();

	const BindRecallInfo* bindRecallInfo = owner.queryTeleportable()->getBindRecallInfo(linkId);
	if (! bindRecallInfo) {
		return ecTeleportNotAvailable;
	}    

	go::Castable* castable = owner.queryCastable();
	if (! castable) {
		return ecServerInternalError;
	}

	TeleportCallback* callback = owner.getController().queryTeleportCallback();
	if (! callback) {
		return ecServerInternalError;
	}

    if (! owner.queryCastState()->canCasting()) {
        return ecAnchorNotBindRecallState;
    }

	go::CastGameTimeable* timeable = owner.queryCastGameTimeable();
	if (timeable) {
		timeable->setCooldown(bindRecallCode,  bindRecallCooltime, 0, 0);
	}

	auto task = std::make_unique<BindRecallCastingTask>(*callback, linkId);
	castable->startCasting(bindRecallCode, false, std::move(task), bindRecallTime);

	go::CastNotificationable* notificationable = owner.queryCastNotificationable();
	if (notificationable) {
		StartCastResultInfo startCastInfo;
		startCastInfo.set(owner.getGameObjectInfo());
		notificationable->notifyStartCasting(startCastInfo);
	}

	return ecOk;
}


ErrorCode PlayerCastController::startLootingItem(const GameObjectInfo& graveStoneInfo, LootInvenId itemId)
{
	go::Entity* target = getOwner().queryKnowable()->getEntity(graveStoneInfo);
	if (! target) {
		return ecEntityNotFound;
	}

	if (! (target->isGraveStone() || target->isAnchor() || target->isTreasure())) {
		return ecEntityNotFound;
	}

	return target->querySOInventoryable()->startLooting(getOwner(), itemId);
}


ErrorCode PlayerCastController::startLootingBuildingItem(const GameObjectInfo& buildingInfo,
    ObjectId itemId, InvenType invenType)
{
    go::Entity* target = getOwner().queryKnowable()->getEntity(buildingInfo);
    if (! target) {
        return ecEntityNotFound;
    }

    if (! target->isBuilding()) {
        return ecEntityNotFound;
    }

    return target->queryBuildingProductionInventoryable()->startLooting(getOwner(), itemId, invenType);
}


ErrorCode PlayerCastController::startNpcCraft(const GameObjectInfo& npcInfo, DataCode recipeCode)
{
    go::Castable* castable = getOwner().queryCastable();
    if (! castable) {
        return ecServerInternalError;
    }
    gc::CraftCallback* craftCallback = getOwner().getController().queryCraftCallback();
    if (! craftCallback) {
        return ecServerInternalError;
    }
    GameTime completeMicSec = 0;
    const ErrorCode errorCode = checkCanCraftable(completeMicSec, npcInfo, recipeCode, false);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    auto task = std::make_unique<NpcCraftTask>(*craftCallback, recipeCode);
    castable->startCasting(recipeCode, false, std::move(task), completeMicSec);

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (notificationable) {
        StartCastResultInfo startCastInfo;
        startCastInfo.set(uctNpcCraft, getOwner().getGameObjectInfo(), npcInfo, recipeCode);
        notificationable->notifyStartCasting(startCastInfo);
    }

    return ecOk;
}


ErrorCode PlayerCastController::startPlayerCraft(const GameObjectInfo& npcInfo, ObjectId recipeId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (! castable) {
        return ecServerInternalError;
    }

    gc::CraftCallback* craftCallback = getOwner().getController().queryCraftCallback();
    if (! craftCallback) {
        return ecServerInternalError;
    }

    Inventory& inventory = getOwner().queryInventoryable()->getInventory();
    ItemInfo recipeInfo = inventory.getItemInfo(recipeId);
    if (! recipeInfo.isValid()) {
        return ecInventoryItemNotFound;
    }
    GameTime completeMicSec = 0;
    const ErrorCode errorCode = checkCanCraftable(completeMicSec, npcInfo, recipeInfo.itemCode_, true);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    auto task = std::make_unique<PlayerCraftTask>(*craftCallback, recipeId);
    castable->startCasting(recipeInfo.itemCode_, false, std::move(task), completeMicSec);

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (notificationable) {
        StartCastResultInfo startCastInfo;
        startCastInfo.set(uctPlayerCraft, getOwner().getGameObjectInfo(), npcInfo, recipeInfo.itemCode_);
        notificationable->notifyStartCasting(startCastInfo);
    }
    return ecOk;
}


ErrorCode PlayerCastController::startReprocess(const GameObjectInfo& npcInfo, ObjectId itemId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (! castable) {
        return ecServerInternalError;
    }

    gc::CraftCallback* craftCallback = getOwner().getController().queryCraftCallback();
    if (! craftCallback) {
        return ecServerInternalError;
    }

    ItemInfo reprocessItemInfo = getOwner().queryInventoryable()->getInventory().getItemInfo(itemId);
    if (! reprocessItemInfo.isValid()) {
        return ecInventoryItemNotFound;
    }

    const datatable::ReprocessInfo* reprocess = REPROCESS_TABLE->getReprocessInfo(reprocessItemInfo.itemCode_);
    if (! reprocess) {
        return ecCraftNotReprocessItem;
    }


    const ErrorCode errorCode = checkCanUpgrade(npcInfo, reprocess->getCraftType());
    if (isFailed(errorCode)) {
        return errorCode;
    }

    const GameTime castTime = GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"reprocess_cast_time");
    auto task = std::make_unique<ReprocessTask>(*craftCallback, itemId);
    castable->startCasting(reprocessItemInfo.itemCode_, false, std::move(task), castTime);

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (notificationable) {
        StartCastResultInfo startCastInfo;
        startCastInfo.set(uctReprocess, getOwner().getGameObjectInfo(), npcInfo, reprocessItemInfo.itemCode_);
        notificationable->notifyStartCasting(startCastInfo);
    }
    return ecOk;
}


ErrorCode PlayerCastController::startEquipUpgrade(const GameObjectInfo& npcInfo, ObjectId equipItemId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (! castable) {
        return ecServerInternalError;
    }

    gc::CraftCallback* craftCallback = getOwner().getController().queryCraftCallback();
    if (! craftCallback) {
        return ecServerInternalError;
    }

    const ErrorCode errorCode = checkCanUpgrade(npcInfo, crtBlackSmith);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    ItemInfo equipItemInfo = getOwner().queryInventoryable()->getInventory().getItemInfo(equipItemId);
    if (! equipItemInfo.isValid()) {
        return ecInventoryItemNotFound;
    }

    const GameTime castTime = GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"equip_upgrade_cast_time");

    auto task = std::make_unique<EquipUpgradeTask>(*craftCallback, equipItemId);
    castable->startCasting(equipItemInfo.itemCode_, false, std::move(task), castTime);

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (notificationable) {
        StartCastResultInfo startCastInfo;
        startCastInfo.set(uctReprocess, getOwner().getGameObjectInfo(), npcInfo, equipItemInfo.itemCode_);
        notificationable->notifyStartCasting(startCastInfo);
    }

    return ecOk;
}


ErrorCode PlayerCastController::startEquipEnchant(const GameObjectInfo& npcInfo, ObjectId equipItemId, ObjectId gemItemId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (! castable) {
        return ecServerInternalError;
    }

    gc::CraftCallback* craftCallback = getOwner().getController().queryCraftCallback();
    if (! craftCallback) {
        return ecServerInternalError;
    }
    const ErrorCode errorCode = checkCanUpgrade(npcInfo, crtBlackSmith);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    ItemInfo equipItemInfo = getOwner().queryInventoryable()->getInventory().getItemInfo(equipItemId);
    if (! equipItemInfo.isValid()) {
        return ecInventoryItemNotFound;
    }

    ItemInfo gemItemInfo = getOwner().queryInventoryable()->getInventory().getItemInfo(gemItemId);
    if (! gemItemInfo.isValid()) {
        return ecInventoryItemNotFound;
    }

    const GameTime castTime = GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"equip_enchant_cast_time");

    auto task = std::make_unique<EquipEnchantTask>(*craftCallback, equipItemId, gemItemId);
    castable->startCasting(equipItemInfo.itemCode_, false, std::move(task), castTime);

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (notificationable) {
        StartCastResultInfo startCastInfo;
        startCastInfo.set(getOwner().getGameObjectInfo(), npcInfo, equipItemInfo.itemCode_, gemItemInfo.itemCode_);
        notificationable->notifyStartCasting(startCastInfo);
    }

    return ecOk;
}


ErrorCode PlayerCastController::startUseFunctionItem(ObjectId itemId)
{
    go::Castable* castable = getOwner().queryCastable();
    if (! castable) {
        return ecServerInternalError;
    }

    ItemCallback* itemCallback = getOwner().getController().queryItemCallback();
    if (! itemCallback) {
        return ecServerInternalError;
    }

    Inventory& inventory = getOwner().queryInventoryable()->getInventory();
    ItemInfo functionItem = inventory.getItemInfo(itemId);
    if (! functionItem.isValid()) {
        return ecInventoryItemNotFound;
    }

    if (! functionItem.isFunction()) {
        return ecItemNotFunctionItem;
    }

    const gdt::function_t* function = FUNCTION_TABLE->getFunction(functionItem.itemCode_);
    if (! function) {
        return ecServerInternalError;
    }

    auto task = std::make_unique<UseFunctionItemCastingTask>(*itemCallback, itemId);
    castable->startCasting(functionItem.itemCode_, false, std::move(task), function->casting_mil_sec());

    go::CastNotificationable* notificationable = getOwner().queryCastNotificationable();
    if (notificationable) {
        StartCastResultInfo startCastInfo;
        startCastInfo.set(uctFunctionItem, getOwner().getGameObjectInfo(), functionItem.itemCode_);
        notificationable->notifyStartCasting(startCastInfo);
    }
    return ecOk;
}


ErrorCode PlayerCastController::checkCanCraftable(GameTime& completeMicSec, const GameObjectInfo& targetInfo, DataCode recipeCode, bool isPlayerCraft)
{
    completeMicSec = 0;
    go::Entity* target = getOwner().queryKnowable()->getEntity(targetInfo);
    if (! target) {
        return ecCraftNotFindTarget;
    }

    if (! target->isNpc()) {
        return ecCraftNotFindTarget;
    }

    if (! DISTANCE_CHECKER->checkNpcDistance(target->getPosition(), getOwner().getPosition())) {
        return ecStoreNpcFarDistance;    
    }


    const go::Craftable* craftable = target->queryCraftable();
    if (! craftable) {
        return ecServerInternalError;
    }

    const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(recipeCode);
    if (! recipe) {
        return ecItemInvalidRecipeItem;
    }

    if (isPlayerCraft) {
        if (! recipe->isRecipeItem()) {
            return ecCraftHasNotRecipeItem;
        }
    }
 
    if (! craftable->hasCraftFunction(recipe->getCraftType())) {
        return ecCraftNpcHasNotCraftFunction;
    }

    if (! getOwner().queryInventoryable()->getInventory().checkHasItemsByBaseItemInfos(recipe->getNeedItemInfos())) {
        return ecItemNotEnoughElementItem;
    }

    if (! getOwner().queryInventoryable()->getInventory().canAddItemByBaseItemInfo(BaseItemInfo(recipe->getCompleteItemCode(), 1))) {
        return ecInventoryIsFull;
    }

    completeMicSec = recipe->getCompleteMilSec();

    return ecOk;
}


ErrorCode PlayerCastController::checkCanUpgrade(const GameObjectInfo& npcInfo, CraftType craftType)
{
    go::Entity* target = getOwner().queryKnowable()->getEntity(npcInfo);
    if (! target) {
        return ecCraftNotFindTarget;
    }

    if (! target->isNpc()) {
        return ecCraftNotFindTarget;
    }

    if (! DISTANCE_CHECKER->checkNpcDistance(target->getPosition(), getOwner().getPosition())) {
        return ecStoreNpcFarDistance;    
    }

    const go::Craftable* craftable = target->queryCraftable();
    if (! craftable) {
        return ecServerInternalError;
    }

    if (! craftable->hasCraftFunction(craftType)) {
        return ecCraftNpcHasNotCraftFunction;
    }

    return ecOk;
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
