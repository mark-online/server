#include "ZoneServerPCH.h"
#include "PlayerNpcController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Dialogable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/state/CreatureState.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerNpcController);

PlayerNpcController::PlayerNpcController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerNpcController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerNpcController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}

// = DialogCallback overriding

void PlayerNpcController::dialogOpened(const GameObjectInfo& npc, const GameObjectInfo& requester)
{
    evDialogOpened(npc, requester);
}


void PlayerNpcController::dialogClosed(const GameObjectInfo& npc, const GameObjectInfo& requester)
{
    evDialogClosed(npc, requester);
}

// = NpcTalkingCallback overriding

void PlayerNpcController::talkedFrom(const GameObjectInfo& talker, NpcTalkingCode talkingCode)
{
    evTalkedFrom(talker, talkingCode);
}

// = sne::srpc::RpcForwarder overriding

void PlayerNpcController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerNpcController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerNpcController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerNpcController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::NpcDialogRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerNpcController, openDialog,
    GameObjectInfo, npcInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    CreatureState* state = owner.queryCreatureState();
    assert(state != nullptr);
    if (! state->canDialog()) {
        return;
    }

    go::Knowable* knowable = getOwner().queryKnowable();
    go::Entity* target = knowable->getEntity(npcInfo);
    if (! target) {
        return;
    }

    go::Dialogable* dialogable = target->queryDialogable();
    if (! dialogable) {
        return;
    }
    dialogable->openDialog(owner);
}


RECEIVE_SRPC_METHOD_1(PlayerNpcController, closeDialog,
    GameObjectInfo, npcInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    go::Knowable* knowable = getOwner().queryKnowable();
    go::Entity* target = knowable->getEntity(npcInfo);
    if (! target) {
        return;
    }

    go::Dialogable* dialogable = target->queryDialogable();
    if (! dialogable) {
        return;
    }
    dialogable->closeDialog(owner.getGameObjectInfo());
}


FORWARD_SRPC_METHOD_2(PlayerNpcController, evDialogOpened,
    GameObjectInfo, npc, GameObjectInfo, requester);


FORWARD_SRPC_METHOD_2(PlayerNpcController, evDialogClosed,
    GameObjectInfo, npc, GameObjectInfo, requester);

// = rpc::NpcTalkingRpc overriding

FORWARD_SRPC_METHOD_2(PlayerNpcController, evTalkedFrom,
    GameObjectInfo, talker, NpcTalkingCode, talkingCode);

}}} // namespace gideon { namespace zoneserver { namespace gc {