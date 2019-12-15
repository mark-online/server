#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/BuildCallback.h"
#include "../callback/AnchorAbillityCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/AnchorRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerAnchorController
 * 아이템 담당
 */
class ZoneServer_Export PlayerAnchorController : public Controller,
    public rpc::AnchorRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerAnchorController);
public:
    PlayerAnchorController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

public:
    OVERRIDE_SRPC_METHOD_2(createAnchor,
        ObjectId, itemId, ObjectPosition, position);

    OVERRIDE_SRPC_METHOD_1(onCreateAnchor,
        ErrorCode, errorCode);
private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:

    ErrorCode spawnAnchor(ObjectId itemId, const ObjectPosition& position);

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
