#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/HarvestCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/HarvestRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver {
class Inventory;
class QuestInventory;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerInventoryController
 * 아이템 담당
 */
class ZoneServer_Export PlayerHarvestController : public Controller,
    public rpc::HarvestRpc,
    public HarvestCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerHarvestController);
public:
    PlayerHarvestController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = HarvestCallback overriding
    virtual void rewarded(ObjectId harvestId, const BaseItemInfos& baseItems,
        const QuestItemInfos& questItems);

public:
    // = rpc::HarvestRpc overriding
    OVERRIDE_SRPC_METHOD_1(acquireHarvest,
        DataCode, itemCode);

    OVERRIDE_SRPC_METHOD_2(onAcquireHarvest,
        ErrorCode, errorCode, DataCode, itemCode);

    OVERRIDE_SRPC_METHOD_3(evHarvestRewarded,
        ObjectId, haravestId, BaseItemInfos, itemInfos,
        QuestItemInfos, questItemInfos);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
