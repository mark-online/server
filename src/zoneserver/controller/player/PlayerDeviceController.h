#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/DeviceCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/DeviceRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerDeviceController
 * 장치 오브젝트 관련
 */
class ZoneServer_Export PlayerDeviceController : public Controller,
    public rpc::DeviceRpc,
    public DeviceCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerDeviceController);

public:
    PlayerDeviceController(go::Entity* owner);

public:
    void initialize() {}
    void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = DeviceCallback overriding
    virtual void deviceDeactivated(ObjectId deviceId);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    // = rpc::DeviceRpc overriding
    OVERRIDE_SRPC_METHOD_1(evDeviceDeactivated,
        ObjectId, deviceId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
