#include "ZoneServerPCH.h"
#include "PlayerDuelController.h"
#include "../EntityController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/FieldDuelable.h"
#include "../../model/state/FieldDuelState.h"
#include "../../service/duel/DuelService.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {


IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerDuelController);

PlayerDuelController::PlayerDuelController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerDuelController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerDuelController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerDuelController::fieldDuelRequested(ObjectId playerId, const Nickname& nickname)
{
    evFieldDuelRequested(playerId, nickname);
}


void PlayerDuelController::fieldDuelResponed(ObjectId playerId, bool isAnswer)
{
    evFieldDuelReponded(playerId, isAnswer);
}


void PlayerDuelController::fieldDuelReadied(const Position& centerPosition)
{
    evFieldDuelReadied(centerPosition);
}


void PlayerDuelController::fieldDuelStarted()
{
    evFieldDuelStarted();
}


void PlayerDuelController::fieldDeulResult(ObjectId winnerId, const Nickname& winner,
    ObjectId loserId, const Nickname& loser)
{
    evFieldDuelResult(winnerId, winner, loserId, loser);
}


void PlayerDuelController::fieldDuelStopped(FieldDuelResultType type)
{
    FieldDuelState* state = getOwner().queryFieldDuelState();
    if (state) {
        state->stopFieldDuel();
    }

    go::FieldDuelable* able = getOwner().queryFieldDuelable();
    if (able) {
        able->setFieldDuel();
    }

    evFieldDuelStopped(type);
}


// = sne::srpc::RpcForwarder overriding

void PlayerDuelController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerDuelController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerDuelController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerDuelController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


RECEIVE_SRPC_METHOD_1(PlayerDuelController, requestFieldDuel,
    ObjectId, playerId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    
    const ErrorCode errorCode = requestFieldDuelToPlayer(playerId);
    onRequestFieldDuel(errorCode);
}


RECEIVE_SRPC_METHOD_2(PlayerDuelController, responseFieldDuel,
    ObjectId, playerId, bool, isAnswer)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = responseFieldDuelToPlayer(playerId, isAnswer);
    
    onResponseFieldDuel(errorCode, isAnswer);
}


RECEIVE_SRPC_METHOD_0(PlayerDuelController, cancelFieldDuel)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    FieldDualPtr ptr = owner.queryFieldDuelable()->getFieldDualPtr();
    if (ptr.get() != nullptr) {
        ptr->stop(fdrtCancelPlayer, invalidObjectId);
    }
}


FORWARD_SRPC_METHOD_1(PlayerDuelController, onRequestFieldDuel,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(PlayerDuelController, onResponseFieldDuel,
    ErrorCode, errorCode, bool, isAnswer);


FORWARD_SRPC_METHOD_2(PlayerDuelController, evFieldDuelRequested,
    ObjectId, playerId, Nickname, nickname);


FORWARD_SRPC_METHOD_2(PlayerDuelController, evFieldDuelReponded,
    ObjectId, playerId, bool, isAnswer);


FORWARD_SRPC_METHOD_1(PlayerDuelController, evFieldDuelReadied,
    Position, centerPosition);


FORWARD_SRPC_METHOD_0(PlayerDuelController, evFieldDuelStarted)


FORWARD_SRPC_METHOD_1(PlayerDuelController, evFieldDuelStopped,
    FieldDuelResultType, type);


FORWARD_SRPC_METHOD_4(PlayerDuelController, evFieldDuelResult,
    ObjectId, winnerId, Nickname, winerNickname, ObjectId, loserId, Nickname, loseNickname);


ErrorCode PlayerDuelController::requestFieldDuelToPlayer(ObjectId playerId)
{
    go::Entity& owner = getOwner();

    go::Entity* target = owner.queryKnowable()->getEntity(GameObjectInfo(otPc, playerId));
    if (! target) {
        return ecDuelTargetNotFind;
    }

    ErrorCode errorCode = DUEL_SERVICE->canFieldDueling(owner, *target);
    if (isSucceeded(errorCode)) {
        FieldDuelCallback* callback = target->getController().queryFieldDuelCallback();
        if  (callback) {
            callback->fieldDuelRequested(owner.getObjectId(), owner.getNickname());
        }
    }
    
    return errorCode;
}


ErrorCode PlayerDuelController::responseFieldDuelToPlayer(ObjectId playerId, bool isAnswer)
{
    go::Entity& owner = getOwner();
    go::Entity* target = owner.queryKnowable()->getEntity(GameObjectInfo(otPc, playerId));
    if (! target) {
        return ecDuelTargetNotFind;
    }

    ErrorCode errorCode = ecOk;
    if (isAnswer) {
        errorCode = DUEL_SERVICE->canFieldDueling(owner, *target);
        if (isFailed(errorCode)) {
            return errorCode;
        }
        else {
            DUEL_SERVICE->startFieldDueling(*target, owner);
        }
    }


    FieldDuelCallback* callback = target->getController().queryFieldDuelCallback();
    if  (callback) {
        callback->fieldDuelResponed(owner.getObjectId(), isAnswer);
    }


    return errorCode;

}

}}}  // namespace gideon { namespace zoneserver { namespace gc {


