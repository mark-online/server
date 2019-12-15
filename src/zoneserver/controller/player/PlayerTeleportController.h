#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/TeleportCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/TeleportRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerTeleportController
 * 장치 오브젝트 관련
 */
class ZoneServer_Export PlayerTeleportController : public Controller,
	public rpc::TeleportRpc,
	public TeleportCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerTeleportController);

public:
    PlayerTeleportController(go::Entity* owner);

public:
    void initialize() {}
    void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);


	void teleportTo(MapCode toWorldMapCode, RegionCode toRegionCode,
		const Position& toPosition = Position());

private:
	virtual void completed(ObjectId bindRecallId);
	virtual void recallReadied();
	virtual void recallRequested(const Nickname& nickname, const WorldPosition& worldPosition);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
	// = rpc::TeleportRpc overriding
	OVERRIDE_SRPC_METHOD_2(teleportToRegion,
		MapCode, worldMapCode, RegionCode, spawnRegionCode);
	OVERRIDE_SRPC_METHOD_2(teleportToPosition,
		MapCode, worldMapCode, Position, position);

	OVERRIDE_SRPC_METHOD_1(addBindRecall,
		ObjectId, bindlingId);
	OVERRIDE_SRPC_METHOD_1(removeBindRecall,
		ObjectId, bindlingId);

	OVERRIDE_SRPC_METHOD_1(setRecallToPlayer,
		Nickname, nickname);
	OVERRIDE_SRPC_METHOD_1(responseRecall,
		bool, isAnswer);

	OVERRIDE_SRPC_METHOD_5(onTeleportTo,
		ErrorCode, errorCode, MapCode, worldMapCode, ObjectPosition, spawnPosition,
		MigrationTicket, migrationTicket, GameMoney, currentGameMoney);

	OVERRIDE_SRPC_METHOD_2(onAddBindRecall,
		ErrorCode, errorCode, BindRecallInfo, bindRecallInfo);
	OVERRIDE_SRPC_METHOD_2(onRemoveBindRecall,
		ErrorCode, errorCode, ObjectId, linkId);
	OVERRIDE_SRPC_METHOD_1(onSetRecallToPlayer,
		ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_2(onResponseRecall,
		ErrorCode, errorCode, MigrationTicket, migrationTicket);

	OVERRIDE_SRPC_METHOD_2(evBindRecallTicket,
		ErrorCode, errorCode, MigrationTicket, migrationTicket);

	OVERRIDE_SRPC_METHOD_1(evRecallRequested,
		Nickname, callerNickname);

private:
	ErrorCode fillBindRecall(BindRecallInfo& bindRecallInfo, ObjectId buildingId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
