#include "ZoneServerPCH.h"
#include "PlayerTeleportController.h"
#include "../EntityController.h"
#include "../../model/gameobject/Building.h"
#include "../../model/gameobject/ability/Castable.h"
#include "../../model/gameobject/ability/CastNotificationable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/gameobject/ability/Teleportable.h"
#include "../../model/gameobject/ability/Protectionable.h"
#include "../../model/gameobject/ability/Networkable.h"
#include "../../model/state/CombatState.h"
#include "../../world/World.h"
#include "../../service/teleport/TeleportService.h"
#include "../../service/anchor/AnchorService.h"
#include <gideon/cs/datatable/WorldMapTable.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerTeleportController);

PlayerTeleportController::PlayerTeleportController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerTeleportController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerTeleportController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerTeleportController::teleportTo(MapCode toWorldMapCode, RegionCode toRegionCode,
    const Position& toPosition)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    MigrationTicket migrationTicket;
    ObjectPosition spawnPosition;
    const ErrorCode errorCode =
        TELEPORT_SERVICE->teleportTo(migrationTicket, spawnPosition, owner,
            toWorldMapCode, toRegionCode, toPosition);

    onTeleportTo(errorCode, toWorldMapCode, spawnPosition, migrationTicket,
        owner.queryMoneyable()->getGameMoney());

    if (isSucceeded(errorCode) && migrationTicket.isValid()) {
        const gdt::map_t* mapTemplate = WORLDMAP_TABLE->getMap(toWorldMapCode); assert(mapTemplate != nullptr);
        if (shouldReleaseBeginnerProtection(WorldType(mapTemplate->world_type()))) {
            owner.queryProtectionable()->releaseBeginnerProtection(false);
        }
		owner.queryNetworkable()->logout();		
    }
}

// = TeleportCallback overriding

void PlayerTeleportController::completed(ObjectId bindRecallId)
{
	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	go::Castable* castable = owner.queryCastable();
	if (castable) {
		castable->releaseCastState(false);
	}

	go::CastNotificationable* notificationable = owner.queryCastNotificationable();
	if (notificationable) {
		CompleteCastResultInfo comleteInfo;
		comleteInfo.set(owner.getGameObjectInfo());
		notificationable->notifyCompleteCasting(comleteInfo);
	}

	MigrationTicket migrationTicket;
	ErrorCode errorCode = ecOk;
	const BindRecallInfo* bindRecallInfo = owner.queryTeleportable()->getBindRecallInfo(bindRecallId);
	if (! bindRecallInfo) {
		errorCode = ecTeleportNotAvailable;
	}
	else {
		errorCode = TELEPORT_SERVICE->teleportRecall(migrationTicket, owner,
			bindRecallInfo->wordlPosition_);
	}

	evBindRecallTicket(errorCode, migrationTicket);

	if (isSucceeded(errorCode) && migrationTicket.isValid()) {
		owner.queryNetworkable()->logout();		
	}
}


void PlayerTeleportController::recallReadied()
{
	go::Entity& owner = getOwner();
	
	go::Entity* player = WORLD->getPlayer(owner.queryTeleportable()->getCallee());
	if (! player) {
		// 커뮤니티에 알린다.
		return;
	}

	TeleportCallback* callback = player->getController().queryTeleportCallback();
	if (callback) {
		callback->recallRequested(owner.getNickname(), owner.getWorldPosition());
	}
}


void PlayerTeleportController::recallRequested(const Nickname& nickname, const WorldPosition& worldPosition)
{
	go::Entity& owner = getOwner();
	owner.queryTeleportable()->setRecallPosition(worldPosition);
	evRecallRequested(nickname);	
}


// = sne::srpc::RpcForwarder overriding

void PlayerTeleportController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTeleportController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerTeleportController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTeleportController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


// = rpc::TeleportRpc overriding

RECEIVE_SRPC_METHOD_2(PlayerTeleportController, teleportToRegion,
    MapCode, worldMapCode, RegionCode, spawnRegionCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    teleportTo(worldMapCode, spawnRegionCode, Position());
}


RECEIVE_SRPC_METHOD_2(PlayerTeleportController, teleportToPosition,
    MapCode, worldMapCode, Position, position)
{
    sne::server::Profiler profiler(__FUNCTION__);
    /// 현재 오벨리스크만 사용한다. 다른이동 사용시에는 수정해야함..
    if (getOwner().queryCombatState()->isCombating()) {
        MigrationTicket migrationTicket;
        ObjectPosition spawnPosition;
        onTeleportTo(ecTeleportNotUseObeliskPlayerCombatState, worldMapCode, spawnPosition, migrationTicket,
            getOwner().queryMoneyable()->getGameMoney());
    }

    teleportTo(worldMapCode, invalidRegionCode, position);
}


RECEIVE_SRPC_METHOD_1(PlayerTeleportController, addBindRecall,
    ObjectId, bindlingId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    BindRecallInfo bindRecallInfo;
    ErrorCode errorCode = fillBindRecall(bindRecallInfo, bindlingId);
    if (isSucceeded(errorCode)) {
        errorCode = owner.queryTeleportable()->addBindRecall(bindRecallInfo);
        if (isSucceeded(errorCode)) {
            DatabaseGuard db(SNE_DATABASE_MANAGER);
            db->asyncAddBindRecall(owner.getAccountId(), owner.getObjectId(), bindRecallInfo);
        }
    }
    onAddBindRecall(errorCode, bindRecallInfo);
}


RECEIVE_SRPC_METHOD_1(PlayerTeleportController, removeBindRecall,
    ObjectId, linkId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const BindRecallInfo* bindRecallInfo = owner.queryTeleportable()->getBindRecallInfo(linkId);
    if (! bindRecallInfo) {
        onRemoveBindRecall(ecAnchorNotExistBindRecall, linkId);
        return;
    }

    owner.queryTeleportable()->removeBindRecall(linkId);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncRemoveBindRecall(owner.getAccountId(), owner.getObjectId(), linkId);

    onRemoveBindRecall(ecOk, linkId);
}


RECEIVE_SRPC_METHOD_1(PlayerTeleportController, setRecallToPlayer,
	Nickname, nickname)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	go::Castable* castable = owner.queryCastable();
	if (castable && castable->isCasting()) {
		onSetRecallToPlayer(ecCastAlreadyCasting);
		return;
	}

	owner.queryTeleportable()->setCallee(nickname);
	onSetRecallToPlayer(ecOk);
}


RECEIVE_SRPC_METHOD_1(PlayerTeleportController, responseRecall,
	bool, isAnswer)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

	if (! isAnswer) {
		onResponseRecall(ecOk, MigrationTicket());
	}
	else {
		const WorldPosition& worldPosition = owner.queryTeleportable()->getRecallPosition();
		MigrationTicket migrationTicket;
		const ErrorCode errorCode = TELEPORT_SERVICE->teleportRecall(migrationTicket, owner,
			worldPosition);
		onResponseRecall(errorCode, migrationTicket);
	}

	owner.queryTeleportable()->setRecallPosition(WorldPosition());
}


FORWARD_SRPC_METHOD_5(PlayerTeleportController, onTeleportTo,
    ErrorCode, errorCode, MapCode, worldMapCode, ObjectPosition, spawnPosition,
    MigrationTicket, migrationTicket, GameMoney, currentGameMoney);


FORWARD_SRPC_METHOD_2(PlayerTeleportController, onAddBindRecall,
    ErrorCode, errorCode, BindRecallInfo, bindRecallInfo);


FORWARD_SRPC_METHOD_2(PlayerTeleportController, onRemoveBindRecall,
    ErrorCode, errorCode, ObjectId, linkId);


FORWARD_SRPC_METHOD_1(PlayerTeleportController, onSetRecallToPlayer,
	ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(PlayerTeleportController, onResponseRecall,
	ErrorCode, errorCode, MigrationTicket, migrationTicket);


FORWARD_SRPC_METHOD_2(PlayerTeleportController, evBindRecallTicket,
    ErrorCode, errorCode, MigrationTicket, migrationTicket);


FORWARD_SRPC_METHOD_1(PlayerTeleportController, evRecallRequested,
	Nickname, callerNickname)

ErrorCode PlayerTeleportController::fillBindRecall(BindRecallInfo& bindRecallInfo, ObjectId buildingId)
{
	go::Entity& owner = getOwner(); 
	const BindRecallInfo* info = owner.queryTeleportable()->getBindRecallInfo(buildingId);
	if (info) {
		return ecAnchorAlreadyAddBindRecall;
	}

	go::Building* building = 
		ANCHOR_SERVICE->getBuilding(GameObjectInfo(otBuilding, buildingId));
	if (! building) {
		return ecAnchorNotFind;
	}

	return building->fillBindRecallInfo(bindRecallInfo, owner.getObjectId());    
}



}}} // namespace gideon { namespace zoneserver { namespace gc {
