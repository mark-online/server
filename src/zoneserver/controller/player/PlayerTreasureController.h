#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/TreasureRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerInventoryController
 * 아이템 담당
 */
class ZoneServer_Export PlayerTreasureController : public Controller,
    public rpc::TreasureRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerTreasureController);
public:
    PlayerTreasureController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

public:
    // = rpc::TreasureRpc overriding
    OVERRIDE_SRPC_METHOD_1(closeTreasure,
        ObjectId, treasureId);

    OVERRIDE_SRPC_METHOD_2(onCloseTreasure,
        ErrorCode, errorCode, ObjectId, treasureId);
    
private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
