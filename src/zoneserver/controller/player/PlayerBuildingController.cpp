#include "ZoneServerPCH.h"
#include "PlayerBuildingController.h"
#include "../EntityController.h"
#include "../callback/BuildingStateCallback.h"
#include "../../model/gameobject/Building.h"
#include "../../model/gameobject/ability/OutsideInventoryable.h"
#include "../../model/gameobject/ability/Protectionable.h"
#include "../../model/gameobject/ability/Networkable.h"
#include "../../model/gameobject/ability/Buildable.h"
#include "../../model/gameobject/ability/Cheatable.h"
#include "../../model/gameobject/ability/Guildable.h"
#include "../../model/gameobject/ability/SkillCastable.h"
#include "../../model/gameobject/ability/NpcHireable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/item/Inventory.h"
#include "../../model/bank/BankAccount.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "../../ZoneService.h"
#include "../../world/WorldMap.h"
#include "../../service/anchor/AnchorService.h"
#include "../../service/teleport/TeleportService.h"
#include "../../service/inven/OrderInvenItemMoveService.h"
#include "../../service/distance/DistanceChecker.h"
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/3d/3d.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

namespace {

inline Position advancePosition(const Position& source, const Vector2& direction,
	float32_t distance)
{
	Position position(source);
	position.x_ += direction.x * distance;
	position.y_ += direction.y * distance;
	//position.z_ = ;
	return position;
}

} // namespace {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerBuildingController);

PlayerBuildingController::PlayerBuildingController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerBuildingController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerBuildingController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerBuildingController::buildingStartBuilt(const GameObjectInfo& buildingInfo,
    sec_t startTime)
{
    evBuildingStartBuilt(buildingInfo, startTime);
}


void PlayerBuildingController::buildingCompleted(const GameObjectInfo& buildingInfo)
{
    evBuildingCompleted(buildingInfo);
}


void PlayerBuildingController::buildingToFoundationReverted(const GameObjectInfo& buildingInfo)
{
    evBuildingToFundationReverted(buildingInfo);
}


void PlayerBuildingController::buildingBroken(const GameObjectInfo& buildingInfo)
{
    evBuildingBroken(buildingInfo);
}


void PlayerBuildingController::buildingDestoryed(const GameObjectInfo& buildingInfo)
{
    evBuildingDestoryed(buildingInfo);
}


void PlayerBuildingController::buildingInvincibleStateChanged(const GameObjectInfo& buildingInfo, bool isActivate)
{
    evBuildingInvincibleStateChanged(buildingInfo, isActivate);
}


void PlayerBuildingController::buildingActivateStateChanged(const GameObjectInfo& buildingInfo, bool isActivate)
{
	evBuildingActivateStateChanged(buildingInfo, isActivate);
}


void PlayerBuildingController::buildingDoorToggled(const GameObjectInfo& buildingInfo, bool isOpen)
{
    evBuildingDoorToggled(buildingInfo, isOpen);
}


void PlayerBuildingController::buildingSelectRecipeProductionAdded(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& taskInfo)
{
    evSelectProductionTaskAdded(buildingInfo, taskInfo);
}

void PlayerBuildingController::buildingSelectRecipeProductionUpdated(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& taskInfo)
{
    evSelectProductionTaskUpdated(buildingInfo, taskInfo);
}


void PlayerBuildingController::buildingSelectRecipeProductionRemoved(const GameObjectInfo& buildingInfo, RecipeCode recipeCode)
{
    evSelectProductionTaskRemoved(buildingInfo, recipeCode);
}


void PlayerBuildingController::teleportToOrderZone(ErrorCode errorCode, BuildingCode toBuildingCode,
	ObjectId fromBuildingId, const WorldPosition& worldPosition)
{
	if (isFailed(errorCode)) {
		onTeleportToBuliding(errorCode);
		return;
	}

	go::Entity& owner = getOwner();

	const datatable::BuildingTemplate* buildingTemplate = BUILDING_TABLE->getBuildingTemplate(toBuildingCode);
	if (! buildingTemplate) {
		onTeleportToBuliding(ecServerInternalError);
		return;
	}

	go::Building* building = 
		ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, fromBuildingId));
	if (! building) {
		onTeleportToBuliding(ecServerInternalError);
		return;
	}


	WorldPosition newPosition = worldPosition;
	Vector2 directionToTarget = getDirection(worldPosition.heading_);
	static_cast<Position>(newPosition) = advancePosition(worldPosition, directionToTarget,
		buildingTemplate->getModelingRadius() + 1.0f);
	newPosition.z_ = worldPosition.z_;

	MigrationTicket migrationTicket;
	errorCode = TELEPORT_SERVICE->teleportRecall(migrationTicket, owner, newPosition);	
	onTeleportToBuliding(errorCode);

	if (isSucceeded(errorCode) && migrationTicket.isValid()) {
		evBuildingTeleportToOrderZoneSuccessed(migrationTicket);
		owner.queryNetworkable()->logout();
	}
	
	// TODO 뭔가 깍는다
	// abstractAnchor
}


RECEIVE_SRPC_METHOD_2(PlayerBuildingController, createBuilding,
    ObjectId, itemId, ObjectPosition, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        ANCHOR_SERVICE->spawnBuilding(position, itemId, owner);
    onCreateBuilding(errorCode);    
}


RECEIVE_SRPC_METHOD_1(PlayerBuildingController, buildBuilding,
    ObjectId, buildingId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = ecAnchorNotFind;
    go::Building* building = ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, buildingId));
    if (building) {
        go::Buildable* buildable = building->queryBuildable();
        if (buildable) {
            errorCode = buildable->build(owner);                        
        }
        else {
            errorCode = ecAnchorNotBuild;
        }
    }
        
    onBuildBuilding(errorCode);    
}


RECEIVE_SRPC_METHOD_3(PlayerBuildingController, fireToTarget,
    GameObjectInfo, anchorInfo, SkillCode, skillCode, GameObjectInfo, targetInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = fireToTarget(skillCode, anchorInfo, targetInfo);

    onFireToTarget(errorCode, anchorInfo, skillCode, targetInfo);
}


RECEIVE_SRPC_METHOD_3(PlayerBuildingController, fireToPosition,
    GameObjectInfo, anchorInfo, SkillCode, skillCode, Position, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = fireToPosition(skillCode, anchorInfo, position);

    onFireToPosition(errorCode, anchorInfo, skillCode, position);
}


RECEIVE_SRPC_METHOD_0(PlayerBuildingController, queryWorldMarkBuildings)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    BuildingMarkInfos buildingMarkInfos;
    ANCHOR_SERVICE->fillBuildingMarkInfos(buildingMarkInfos);
    onWorldMarkBuildings(buildingMarkInfos);
}


RECEIVE_SRPC_METHOD_1(PlayerBuildingController, queryBuildingInfo,
    ObjectId, buildingId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    
    sec_t expireIncibleCooltime = 0;
    BuildingOwnerInfo ownerInfo;
	BuildingItemTaskInfos consumeTaskInfos;
	BuildingItemTaskInfos resouceTaskInfos;
    BuildingItemTaskInfos selectProductionTaskInfos;
    const ErrorCode errorCode = fillBuildingInfo(expireIncibleCooltime,
        ownerInfo, consumeTaskInfos, resouceTaskInfos, selectProductionTaskInfos, buildingId);
    if (isSucceeded(errorCode)) {
        onBuildingInfo(expireIncibleCooltime, ownerInfo, consumeTaskInfos, resouceTaskInfos,
            selectProductionTaskInfos);
    }    
}


RECEIVE_SRPC_METHOD_1(PlayerBuildingController, queryBuildingExpireTime,
    GameObjectInfo, objectInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    go::Building* building = ANCHOR_SERVICE->getBuilding(objectInfo);
    if (! building) {
        onBuildingExpireTime(ecAnchorNotFind, objectInfo, 0);
        return;
    }
    onBuildingExpireTime(ecOk, objectInfo, building->getExpireTime());
}


RECEIVE_SRPC_METHOD_1(PlayerBuildingController, queryBuildingInvincibleExpireTime,
    GameObjectInfo, objectInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    go::Building* building = ANCHOR_SERVICE->getBuilding(objectInfo);
    if (! building) {
        onBuildingInvincibleExpireTime(ecAnchorNotFind, objectInfo, 0);
        return;
    }
    onBuildingInvincibleExpireTime(ecOk, objectInfo, building->getInvincibleExpireTime());
}


RECEIVE_SRPC_METHOD_1(PlayerBuildingController, destoryBuildingCommand,
    GameObjectInfo, objectInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    go::Building* building = ANCHOR_SERVICE->getBuilding(objectInfo);
    if (building) {
        BuildingStateCallback* callback = building->getController().queryBuildingStateCallback();
        if (callback) {
            callback->buildDestroyed();
            static_cast<go::Entity*>(building)->despawn();
        }
    }
}


RECEIVE_SRPC_METHOD_3(PlayerBuildingController, teleportToBuliding,
	MapCode, worldMap, ObjectId, fromBuildingId, ObjectId, toBuildingId)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	const WorldMap* currentWorld = owner.getCurrentWorldMap();
	if (! currentWorld) {
		return;
	}

	ErrorCode errorCode = ecOk;
	if (currentWorld->getMapCode() == worldMap) {
		errorCode = teleportToSameZone(GameObjectInfo(otBuilding, fromBuildingId), GameObjectInfo(otBuilding, toBuildingId));
		onTeleportToBuliding(errorCode);
	}
	else {
		errorCode = teleportToOrderZone(worldMap, GameObjectInfo(otBuilding, fromBuildingId), toBuildingId);
		if (isFailed(errorCode)) {
			onTeleportToBuliding(errorCode);
		}
	}
}


RECEIVE_SRPC_METHOD_3(PlayerBuildingController, startSelectRecipeProduction,
    ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    go::Building* building = ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, buildingId));
    if (! building) {
        onStartSelectRecipeProduction(ecAnchorNotFind);
        return;
    }

    const ErrorCode errorCode = 
        building->querySelectRecipeProductionable()->startRecipeProduction(owner, recipeCode, count);
    onStartSelectRecipeProduction(errorCode);
}


RECEIVE_SRPC_METHOD_1(PlayerBuildingController, toggleBuildingDoor,
    ObjectId, buildingId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    go::Building* building = ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, buildingId));

    if (! building) {
        onToggleBuildingDoor(ecAnchorNotFind);
        return;
    }

    BuildingStateCallback* callback = building->getController().queryBuildingStateCallback();
    if (! callback) {
        onToggleBuildingDoor(ecServerInternalError);
        return;
    }
    
    const ErrorCode errorCode = callback->toggleDoor(owner);
    if (isFailed(errorCode)) {
        onToggleBuildingDoor(errorCode);
    }
}


RECEIVE_SRPC_METHOD_3(PlayerBuildingController, buyBuildingGuard,
    ObjectId, buildingId, BuildingGuardCode, guardCode, uint8_t, rentDay)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    go::Building* building = ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, buildingId));
    if (! building) {
        onBuyBuildingGuard(ecAnchorNotFind, 0);
        return;
    }
    
    ErrorCode errorCode = building->queryNpcHireable()->buyBuildingGuard(owner, guardCode, rentDay); 
    
    onBuyBuildingGuard(errorCode, owner.queryMoneyable()->getGameMoney());
}


RECEIVE_SRPC_METHOD_3(PlayerBuildingController, spawnBuildingGuard,
    ObjectId, mercenaryId, ObjectId, buildingId, ObjectPosition, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! ANCHOR_SERVICE->isModelingIntersection(position, 0.3f)) {
        onSpawnBuildingGuard(ecAnchorNearDistanceBuildAnchor);
        return;
    }

    go::Building* building = ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, buildingId));
    if (! building) {
        onSpawnBuildingGuard(ecAnchorNotFind);
        return;
    }

    const ErrorCode errorCode = building->queryNpcHireable()->spawnBuildingGuard(owner, mercenaryId, position);
    onSpawnBuildingGuard(errorCode);
}


FORWARD_SRPC_METHOD_1(PlayerBuildingController, onCreateBuilding,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, onBuildBuilding,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_4(PlayerBuildingController, onFireToTarget,
    ErrorCode, errorCode, GameObjectInfo, anchorInfo, SkillCode, skillCode, GameObjectInfo, targetInfo);


FORWARD_SRPC_METHOD_4(PlayerBuildingController, onFireToPosition,
    ErrorCode, errorCode, GameObjectInfo, anchorInfo, SkillCode, skillCode, Position, position);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, onWorldMarkBuildings,
    BuildingMarkInfos, buildingMarkInfos);


FORWARD_SRPC_METHOD_5(PlayerBuildingController, onBuildingInfo,
    sec_t, expireIncibleCooltime, BuildingOwnerInfo, ownerInfo,
	BuildingItemTaskInfos, consumeTaskInfos, BuildingItemTaskInfos, resouceTaskInfos,
    BuildingItemTaskInfos, selectProductionTaskInfos);


FORWARD_SRPC_METHOD_3(PlayerBuildingController, onBuildingExpireTime,
    ErrorCode, errorCode, GameObjectInfo, objectInfo, sec_t, expireTime);


FORWARD_SRPC_METHOD_3(PlayerBuildingController, onBuildingInvincibleExpireTime,
    ErrorCode, errorCode, GameObjectInfo, objectInfo, sec_t, expireTime);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, onTeleportToBuliding,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, onStartSelectRecipeProduction,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, onToggleBuildingDoor,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, onBuyBuildingGuard,
    ErrorCode, errorCode, GameMoney, currentGameMoney);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, onSpawnBuildingGuard,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, evBuildingActivateStateChanged,
	GameObjectInfo, buildingInfo, bool, isActivate);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, evBuildingToFundationReverted,
    GameObjectInfo, buildingInfo);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, evBuildingStartBuilt,
    GameObjectInfo, buildingInfo, sec_t, startBuildTime);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, evBuildingCompleted,
    GameObjectInfo, buildingInfo);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, evBuildingBroken,
    GameObjectInfo, buildingInfo);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, evBuildingDestoryed,
    GameObjectInfo, buildingInfo);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, evBuildingInvincibleStateChanged,
    GameObjectInfo, buildingInfo, bool, isActivate);


FORWARD_SRPC_METHOD_1(PlayerBuildingController, evBuildingTeleportToOrderZoneSuccessed,
	MigrationTicket, ticket);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, evBuildingDoorToggled,
    GameObjectInfo, buildingInfo, bool, isOpen);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, evSelectProductionTaskAdded,
    GameObjectInfo, buildingInfo, BuildingItemTaskInfo, resouceTaskInfo);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, evSelectProductionTaskUpdated,
    GameObjectInfo, buildingInfo, BuildingItemTaskInfo, resouceTaskInfo);


FORWARD_SRPC_METHOD_2(PlayerBuildingController, evSelectProductionTaskRemoved,
    GameObjectInfo, buildingInfo, RecipeCode, recipeCode);


// = sne::srpc::RpcForwarder overriding

void PlayerBuildingController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerBuildingController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


ErrorCode PlayerBuildingController::fireToTarget(SkillCode skillCode,
    const GameObjectInfo buildingInfo, const GameObjectInfo& targetInfo)
{
    go::Building* building = ANCHOR_SERVICE->getBuilding(buildingInfo);
    if (! building) {
        return ecAnchorNotFind;
    }

    const BuildingOwnerInfo& ownerInfo = building->queryStaticObjectOwnerable()->getBuildingOwnerInfo();
    go::Entity& owner = getOwner();
    if (isPrivateOwnerType(ownerInfo.ownerType_)) {
        if (owner.getObjectId() != ownerInfo.playerOwnerInfo_.objectId_) {
            return ecAnchorNotOwner;
        }
    }
    else if (isGuildOwnerType(ownerInfo.ownerType_)) {
        if (owner.queryGuildable()->getGuildId() != ownerInfo.guildInfo_.guildId_) {
            return ecAnchorNotOwner;
        }
    }
    else {
        return ecAnchorNotOwner;
    }

    go::SkillCastable* skillCastable = building->querySkillCastable();
    assert(skillCastable);

    const ErrorCode errorCode = skillCastable->checkSkillCasting(skillCode, targetInfo);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    return skillCastable->castTo(targetInfo, skillCode);
}


ErrorCode PlayerBuildingController::fireToPosition(SkillCode skillCode, 
    const GameObjectInfo buildingInfo, const Position& position)
{
    go::Building* building = ANCHOR_SERVICE->getBuilding(buildingInfo);
    if (! building) {
        return ecAnchorNotFind;
    }

    const BuildingOwnerInfo& ownerInfo = building->queryStaticObjectOwnerable()->getBuildingOwnerInfo();
    go::Entity& owner = getOwner();
    if (isPrivateOwnerType(ownerInfo.ownerType_)) {
        if (owner.getObjectId() != ownerInfo.playerOwnerInfo_.objectId_) {
            return ecAnchorNotOwner;
        }
    }
    else if (isGuildOwnerType(ownerInfo.ownerType_)) {
        if (owner.queryGuildable()->getGuildId() != ownerInfo.guildInfo_.guildId_) {
            return ecAnchorNotOwner;
        }
    }
    else {
        return ecAnchorNotOwner;
    }

    go::SkillCastable* skillCastable = building->querySkillCastable();
    assert(skillCastable);

    const ErrorCode errorCode = skillCastable->checkSkillCasting(skillCode, position);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    return skillCastable->castAt(position, skillCode);
}


ErrorCode PlayerBuildingController::fillBuildingInfo(sec_t& expireIncibleCooltime,
	BuildingOwnerInfo& ownerInfo, BuildingItemTaskInfos& consumeTaskInfos, BuildingItemTaskInfos& resouceTaskInfos,
    BuildingItemTaskInfos& selectProductionTaskInfos, ObjectId buildingId)
{
    go::Building* building = 
        ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, buildingId));
    if (! building) {
        return ecAnchorNotFind;
    }

    if (! DISTANCE_CHECKER->checkBuildingDistance(getOwner().getPosition(), building->getPosition())) {
        return ecAnchorNotFind;
    }

    building->fillBuildingInfo(expireIncibleCooltime, ownerInfo, consumeTaskInfos, resouceTaskInfos, selectProductionTaskInfos);
    return ecOk;
}


ErrorCode PlayerBuildingController::teleportToSameZone(const GameObjectInfo& fromInfo, const GameObjectInfo& toInfo)
{
	go::Entity& owner = getOwner();

	go::Building* fromBuilding = ANCHOR_SERVICE->getBuilding(fromInfo);
	if (! fromBuilding) {
		return ecAnchorNotFind;
	}

	const ErrorCode fromEc = fromBuilding->canTeleport(owner);
	if (isFailed(fromEc)) {
		return fromEc;
	}

	go::AbstractAnchor* toBuilding = ANCHOR_SERVICE->getBuilding(toInfo);
	if (! toBuilding) {
		return ecAnchorNotFind;
	}
	const ErrorCode toEc = toBuilding->canTeleport(owner);
	if (isFailed(toEc)) {
		return toEc;
	}

	WorldPosition worldPosition = toBuilding->getWorldPosition();
	WorldPosition newPosition = toBuilding->getWorldPosition();

	Vector2 directionToTarget = getDirection(worldPosition.heading_);
	static_cast<Position>(newPosition) = advancePosition(worldPosition, directionToTarget,
		toBuilding->getModelingRadiusSize() + 1.0f);
	newPosition.z_ = worldPosition.z_;

	MigrationTicket ticket;
	return TELEPORT_SERVICE->teleportRecall(ticket, owner, newPosition);;
}


ErrorCode PlayerBuildingController::teleportToOrderZone(MapCode worldCode,
	const GameObjectInfo& fromInfo, ObjectId toId)
{
	go::Entity& owner = getOwner();

	go::Building* fromBuilding = ANCHOR_SERVICE->getBuilding(fromInfo);
	if (! fromBuilding) {
		return ecAnchorNotFind;
	}

	const ErrorCode fromEc = fromBuilding->canTeleport(owner);
	if (isFailed(fromEc)) {
		return fromEc;
	}

	ZoneCommunityServerProxy& communityServerProxy =
		ZONE_SERVICE->getCommunityServerProxy();
	communityServerProxy.z2m_requestTeleportToBuilding(
		owner.getObjectId(), worldCode, owner.queryGuildable()->getGuildId(), toId);

	return ecOk;
}


}}} // namespace gideon { namespace zoneserver { namespace gc {