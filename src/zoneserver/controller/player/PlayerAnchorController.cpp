#include "ZoneServerPCH.h"
#include "PlayerAnchorController.h"
#include "../../model/item/Inventory.h"
#include "../../model/gameobject/Anchor.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../world/WorldMap.h"
#include "../../world/region/SpawnMapRegion.h"
#include "../../model/state/ItemManageState.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerAnchorController);

PlayerAnchorController::PlayerAnchorController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerAnchorController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerAnchorController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


RECEIVE_SRPC_METHOD_2(PlayerAnchorController, createAnchor,
    ObjectId, itemId, ObjectPosition, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    const ErrorCode errorCode = spawnAnchor(itemId, position);
    if (isFailed(errorCode)) {
        onCreateAnchor(errorCode);
    }
}


FORWARD_SRPC_METHOD_1(PlayerAnchorController, onCreateAnchor,
    ErrorCode, errorCode);

// = sne::srpc::RpcForwarder overriding

void PlayerAnchorController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerAnchorController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


ErrorCode PlayerAnchorController::spawnAnchor(ObjectId itemId, const ObjectPosition& position)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return ecServerInternalError;
    }

    Inventory& inventory = owner.queryInventoryable()->getInventory();
    DataCode itemCode = inventory.getItemCode(itemId);
    if (! isValidDataCode(itemCode)) {
        return ecInventoryItemNotFound;
    }

    if (owner.queryCastGameTimeable()->isLocalCooldown(itemCode)) {
        return ecAnchorNotEnoughCoolTime;
    }

    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return ecServerInternalError;
    }

    SpawnMapRegion& globalSpawnMapRegion =
        static_cast<SpawnMapRegion&>(worldMap->getGlobalMapRegion());
    go::Entity* anchor;
    const ErrorCode errorCode = globalSpawnMapRegion.spawnAnchor(anchor, itemCode, position, &owner);
    if (isSucceeded(errorCode)) {
        (void)inventory.useItemsByItemId(itemId, 1);
    }
    return errorCode;
}

}}} // namespace gideon { namespace zoneserver { namespace gc {