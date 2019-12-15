#include "ZoneServerPCH.h"
#include "PlayerDeviceController.h"
#include "../../model/gameobject/Device.h"
#include "../../controller/EntityController.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>

namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerDeviceController);

PlayerDeviceController::PlayerDeviceController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerDeviceController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerDeviceController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}

// = DeviceCallback overriding

void PlayerDeviceController::deviceDeactivated(ObjectId deviceId)
{
    evDeviceDeactivated(deviceId);
}

// = sne::srpc::RpcForwarder overriding

void PlayerDeviceController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerDeviceController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerDeviceController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerDeviceController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::DeviceRpc overriding

FORWARD_SRPC_METHOD_1(PlayerDeviceController, evDeviceDeactivated,
    ObjectId, deviceId);

}}} // namespace gideon { namespace zoneserver { namespace gc {
