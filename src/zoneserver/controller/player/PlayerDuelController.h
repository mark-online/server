#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/FieldDuelCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/FieldDuelRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
    class RpcingExtension;
}} // namespace sne { namespace sgp {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerDuelController
 * 비석 담당
 */
class ZoneServer_Export PlayerDuelController : public Controller,
    public rpc::FieldDuelRpc,
    public FieldDuelCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerDuelController);
public:
    PlayerDuelController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    virtual void fieldDuelRequested(ObjectId playerId, const Nickname& nickname);
    virtual void fieldDuelResponed(ObjectId playerId, bool isAnswer);
    virtual void fieldDuelReadied(const Position& centerPosition);
    virtual void fieldDuelStarted();

    virtual void fieldDuelStopped(FieldDuelResultType type);
    virtual void fieldDeulResult(ObjectId winnerId, const Nickname& wniner,
        ObjectId loserId, const Nickname& loser);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);


public:
    OVERRIDE_SRPC_METHOD_1(requestFieldDuel,
        ObjectId, playerId);

    OVERRIDE_SRPC_METHOD_2(responseFieldDuel,
        ObjectId, playerId, bool, isAnswer);

    OVERRIDE_SRPC_METHOD_0(cancelFieldDuel);

public:
    OVERRIDE_SRPC_METHOD_1(onRequestFieldDuel,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_2(onResponseFieldDuel,
        ErrorCode, errorCode, bool, isAnswer);

public:
    OVERRIDE_SRPC_METHOD_2(evFieldDuelRequested,
        ObjectId, playerId, Nickname, nickname);

    OVERRIDE_SRPC_METHOD_2(evFieldDuelReponded,
        ObjectId, playerId, bool, isAnswer);

    OVERRIDE_SRPC_METHOD_1(evFieldDuelReadied,
        Position, centerPosition);

    OVERRIDE_SRPC_METHOD_0(evFieldDuelStarted);
    
    OVERRIDE_SRPC_METHOD_1(evFieldDuelStopped,
        FieldDuelResultType, type);

    OVERRIDE_SRPC_METHOD_4(evFieldDuelResult,
        ObjectId, winnerId, Nickname, winerNickname, ObjectId, loserId, Nickname, loseNickname);

private:
    ErrorCode requestFieldDuelToPlayer(ObjectId playerId);
    ErrorCode responseFieldDuelToPlayer(ObjectId playerId, bool isAnswer);

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
