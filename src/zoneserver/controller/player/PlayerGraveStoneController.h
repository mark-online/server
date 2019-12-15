#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/GraveStoneCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/GraveStoneRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
    class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerGraveStoneController
 * 비석 담당
 */
class ZoneServer_Export PlayerGraveStoneController : public Controller,
    public rpc::GraveStoneRpc,
	public GraveStoneCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerGraveStoneController);
public:
    PlayerGraveStoneController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

	virtual void destoryed(go::GraveStone& graveStone);
    virtual void graveStoneRemoved();

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    OVERRIDE_SRPC_METHOD_0(evSelfGraveStoneRemoved);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
