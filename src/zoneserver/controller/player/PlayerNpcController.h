#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/DialogCallback.h"
#include "../callback/NpcTalkingCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/NpcRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerNpcController
 * NPC 관련
 */
class ZoneServer_Export PlayerNpcController : public Controller,
    public rpc::NpcDialogRpc,
    public rpc::NpcTalkingRpc,
    public DialogCallback,
    public NpcTalkingCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerNpcController);
public:
    PlayerNpcController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = DialogCallback overriding
    virtual void dialogOpened(const GameObjectInfo& npc, const GameObjectInfo& requester);
    virtual void dialogClosed(const GameObjectInfo& npc, const GameObjectInfo& requester);

private:
    // = NpcTalkingCallback overriding
    virtual void talkedFrom(const GameObjectInfo& talker, NpcTalkingCode talkingCode);

public:
    // = rpc::NpcDialogRpc overriding
    OVERRIDE_SRPC_METHOD_1(openDialog,
        GameObjectInfo, npcInfo);
    OVERRIDE_SRPC_METHOD_1(closeDialog,
        GameObjectInfo, npcInfo);

    OVERRIDE_SRPC_METHOD_2(evDialogOpened,
        GameObjectInfo, npc, GameObjectInfo, requester);
    OVERRIDE_SRPC_METHOD_2(evDialogClosed,
        GameObjectInfo, npc, GameObjectInfo, requester);

private:
    // = rpc::NpcTalkingRpc overriding
    OVERRIDE_SRPC_METHOD_2(evTalkedFrom,
        GameObjectInfo, talker, NpcTalkingCode, talkingCode);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
