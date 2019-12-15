#include "ZoneServerPCH.h"
#include "PlayerTreasureController.h"
#include "../../model/gameobject/Treasure.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/state/TreasureState.h"
#include "../../model/state/VehicleState.h"
#include "../../service/spawn/SpawnService.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerTreasureController);

PlayerTreasureController::PlayerTreasureController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerTreasureController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerTreasureController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}

// = rpc::TreasureRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerTreasureController, closeTreasure,
    ObjectId, treasureId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    go::Entity* target = 
        getOwner().queryKnowable()->getEntity(GameObjectInfo(otTreasure, treasureId));
    if (! target) {
        onCloseTreasure(ecTreasureInvalidTreasure, treasureId);
        return;
    }

    go::Treasure& treasure = static_cast<go::Treasure&>(*target);
    if (! treasure.canClose(getOwner().getObjectId())) {
        onCloseTreasure(ecTreasureOrderOwnerTreasure, treasureId);
        return;
    }

    WorldMap* worldMap = treasure.getCurrentWorldMap();
    if (worldMap) {
        const ErrorCode errorCode = treasure.despawn();
        if (isFailed(errorCode)) {
            assert(false);
            SNE_LOG_ERROR("Failed to despawn Treasure(%" PRIu64 ")[E%d]",
                treasure.getObjectId(), errorCode);
        }
    }

    onCloseTreasure(ecOk, treasureId);
}


FORWARD_SRPC_METHOD_2(PlayerTreasureController, onCloseTreasure,
    ErrorCode, errorCode, ObjectId, treasureId);


// = sne::srpc::RpcForwarder overriding

void PlayerTreasureController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTreasureController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerTreasureController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTreasureController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {