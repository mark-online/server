#include "ZoneServerPCH.h"
#include "PlayerGraveStoneController.h"
#include "../../model/item/Inventory.h"
#include "../../model/gameobject/GraveStone.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/state/ItemManageState.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/state/ItemManageState.h"
#include "../../model/state/VehicleState.h"
#include "../../service/item/GraveStoneService.h"
#include "../../service/distance/DistanceChecker.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {


IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerGraveStoneController);

PlayerGraveStoneController::PlayerGraveStoneController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerGraveStoneController::destoryed(go::GraveStone& graveStone)
{
	if (graveStone.shouldDespawn()) {
		GRAVE_STONE_SERVICE->unregisterGraveStone(graveStone);
	}
}


void PlayerGraveStoneController::graveStoneRemoved()
{
    evSelfGraveStoneRemoved();
}


void PlayerGraveStoneController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerGraveStoneController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


// = sne::srpc::RpcForwarder overriding

void PlayerGraveStoneController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerGraveStoneController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerGraveStoneController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerGraveStoneController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


FORWARD_SRPC_METHOD_0(PlayerGraveStoneController, evSelfGraveStoneRemoved);

}}}  // namespace gideon { namespace zoneserver { namespace gc {


