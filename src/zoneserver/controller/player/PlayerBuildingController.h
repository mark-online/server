#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/BuildCallback.h"
#include "../callback/BuildingTeleportCallback.h"
#include "../callback/BuildingProductionTaskCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/BuildingRpc.h>
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
 * @class PlayerBuildingController
 * 건물 담당
 */
class ZoneServer_Export PlayerBuildingController : public Controller,
    public rpc::BuildingRpc,
    public BuildCallback,
	public BuildingTeleportCallback,
    public BuildingProductionTaskCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerBuildingController);
public:
    PlayerBuildingController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = BuildCallback override
    virtual void buildingStartBuilt(const GameObjectInfo& buildingInfo, sec_t startTime);
    virtual void buildingCompleted(const GameObjectInfo& buildingInfo);
    virtual void buildingToFoundationReverted(const GameObjectInfo& buildingInfo);
    virtual void buildingBroken(const GameObjectInfo& buildingInfo);
    virtual void buildingDestoryed(const GameObjectInfo& buildingInfo);
    virtual void buildingInvincibleStateChanged(const GameObjectInfo& buildingInfo, bool isActivate);
	virtual void buildingActivateStateChanged(const GameObjectInfo& buildingInfo, bool isActivate);
    virtual void buildingDoorToggled(const GameObjectInfo& buildingInfo, bool isOpen);
    // = BuildingTeleportCallback override
	virtual void teleportToOrderZone(ErrorCode errorCode, BuildingCode toBuildingCode,
		ObjectId fromBuildingId, const WorldPosition& worldPosition);
    
    // = BuildingProductionTaskCallback override
    virtual void buildingSelectRecipeProductionAdded(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& taskInfo);
    virtual void buildingSelectRecipeProductionUpdated(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& taskInfo);
    virtual void buildingSelectRecipeProductionRemoved(const GameObjectInfo& buildingInfo, RecipeCode recipeCode);

public:
    OVERRIDE_SRPC_METHOD_2(createBuilding,
        ObjectId, itemId, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(buildBuilding,
        ObjectId, buildingId);
    OVERRIDE_SRPC_METHOD_3(fireToTarget,
        GameObjectInfo, anchorInfo, SkillCode, skillCode, GameObjectInfo, targetInfo);
    OVERRIDE_SRPC_METHOD_3(fireToPosition,
        GameObjectInfo, anchorInfo, SkillCode, skillCode, Position, position);
    OVERRIDE_SRPC_METHOD_0(queryWorldMarkBuildings);
    OVERRIDE_SRPC_METHOD_1(queryBuildingInfo,
        ObjectId, buildingId);
    OVERRIDE_SRPC_METHOD_1(queryBuildingExpireTime,
        GameObjectInfo, objectInfo);
    OVERRIDE_SRPC_METHOD_1(queryBuildingInvincibleExpireTime,
        GameObjectInfo, objectInfo);
	OVERRIDE_SRPC_METHOD_1(destoryBuildingCommand,
		GameObjectInfo, objectInfo);
	OVERRIDE_SRPC_METHOD_3(teleportToBuliding,
		MapCode, worldMap, ObjectId, fromBuildingId, ObjectId, toBuildingId);
    OVERRIDE_SRPC_METHOD_3(startSelectRecipeProduction,
        ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count);
    OVERRIDE_SRPC_METHOD_1(toggleBuildingDoor,
        ObjectId, buildingId);
    OVERRIDE_SRPC_METHOD_3(buyBuildingGuard,
        ObjectId, buildingId, BuildingGuardCode, guardCode, uint8_t, rentDay);
    OVERRIDE_SRPC_METHOD_3(spawnBuildingGuard,
        ObjectId, mercenaryId, ObjectId, buildingId, ObjectPosition, position);

    OVERRIDE_SRPC_METHOD_1(onCreateBuilding,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onBuildBuilding,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_4(onFireToTarget,
        ErrorCode, errorCode, GameObjectInfo, anchorInfo, SkillCode, skillCode, GameObjectInfo, targetInfo);
    OVERRIDE_SRPC_METHOD_4(onFireToPosition,
        ErrorCode, errorCode, GameObjectInfo, anchorInfo, SkillCode, skillCode, Position, position);
    OVERRIDE_SRPC_METHOD_1(onWorldMarkBuildings,
        BuildingMarkInfos, buildingMarkInfos);
    OVERRIDE_SRPC_METHOD_5(onBuildingInfo,
        sec_t, expireIncibleCooltime, BuildingOwnerInfo, ownerInfo,
		BuildingItemTaskInfos, consumeTaskInfos, BuildingItemTaskInfos, resouceTaskInfos,
        BuildingItemTaskInfos, selectProductionTaskInfos);

    OVERRIDE_SRPC_METHOD_3(onBuildingExpireTime,
        ErrorCode, errorCode, GameObjectInfo, objectInfo, sec_t, expireTime);
    OVERRIDE_SRPC_METHOD_3(onBuildingInvincibleExpireTime,
        ErrorCode, errorCode, GameObjectInfo, objectInfo, sec_t, expireTime);
	OVERRIDE_SRPC_METHOD_1(onTeleportToBuliding,
		ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onStartSelectRecipeProduction,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onToggleBuildingDoor,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onBuyBuildingGuard,
        ErrorCode, errorCode, GameMoney, currentGameMoney);
    OVERRIDE_SRPC_METHOD_1(onSpawnBuildingGuard,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_2(evBuildingStartBuilt,
        GameObjectInfo, buildingInfo, sec_t, startBuildTime);
    OVERRIDE_SRPC_METHOD_1(evBuildingCompleted,
        GameObjectInfo, buildingInfo);

    OVERRIDE_SRPC_METHOD_1(evBuildingToFundationReverted,
        GameObjectInfo, buildingInfo);

    OVERRIDE_SRPC_METHOD_1(evBuildingBroken,
        GameObjectInfo, buildingInfo);

    OVERRIDE_SRPC_METHOD_1(evBuildingDestoryed,
        GameObjectInfo, buildingInfo);

    OVERRIDE_SRPC_METHOD_2(evBuildingInvincibleStateChanged,
        GameObjectInfo, buildingInfo, bool, isActivate);

	OVERRIDE_SRPC_METHOD_2(evBuildingActivateStateChanged,
		GameObjectInfo, buildingInfo, bool, isActivate);
	
	OVERRIDE_SRPC_METHOD_1(evBuildingTeleportToOrderZoneSuccessed,
		MigrationTicket, ticket);
    
    OVERRIDE_SRPC_METHOD_2(evBuildingDoorToggled,
        GameObjectInfo, buildingInfo, bool, isOpen);

    OVERRIDE_SRPC_METHOD_2(evSelectProductionTaskAdded,
        GameObjectInfo, buildingInfo, BuildingItemTaskInfo, resouceTaskInfo);

    OVERRIDE_SRPC_METHOD_2(evSelectProductionTaskUpdated,
        GameObjectInfo, buildingInfo, BuildingItemTaskInfo, resouceTaskInfo);

    OVERRIDE_SRPC_METHOD_2(evSelectProductionTaskRemoved,
        GameObjectInfo, buildingInfo, RecipeCode, recipeCode);

private:
    ErrorCode fireToTarget(SkillCode skillCode, const GameObjectInfo anchorInfo, const GameObjectInfo& targetInfo);
    ErrorCode fireToPosition(SkillCode skillCode, const GameObjectInfo anchorInfo, const Position& position);

	ErrorCode teleportToSameZone(const GameObjectInfo& fromInfo, const GameObjectInfo& toInfo);
	ErrorCode teleportToOrderZone(MapCode worldCode, const GameObjectInfo& fromInfo, ObjectId toId);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    ErrorCode fillBuildingInfo(sec_t& expireIncibleCooltime,
		BuildingOwnerInfo& ownerInfo, BuildingItemTaskInfos& consumeTaskInfos,
		BuildingItemTaskInfos& resouceTaskInfos, BuildingItemTaskInfos& selectProductionTaskInfos, ObjectId buildingId);
    ErrorCode checkBuyBuildingGuard();
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
