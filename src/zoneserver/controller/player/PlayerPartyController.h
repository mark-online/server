#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/PartyCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/PartyRpc.h>
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
class ZoneServer_Export PlayerPartyController : public Controller,
    public rpc::PartyRpc,
    public PartyCallback,    
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerPartyController);
public:
    PlayerPartyController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);
private:
    virtual void partyMemberLevelup(ObjectId memberId, CreatureLevel level);
    virtual void partyMemberMoved(ObjectId objectId, const Position& position);


public: // = PartyRpc overriding
    OVERRIDE_SRPC_METHOD_1(queryPartyMemberSubInfo,
        ObjectId, playerId);

    OVERRIDE_SRPC_METHOD_6(onPartyMemberSubInfo,
        ObjectId, objectId, CharacterClass, characterClass,
        CreatureLevel, level, HitPoints, hitPoints, ManaPoints, manaPoints, Position, position);

    OVERRIDE_SRPC_METHOD_2(evPartyMemberLevelup,
        ObjectId, objectId, CreatureLevel, level);

    OVERRIDE_SRPC_METHOD_2(evPartyMemberMoved,
        ObjectId, objectId, Position, position);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
