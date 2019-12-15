#pragma once

#include "ZoneServer/controller/player/PlayerPartyController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerPartyController
 *
 * 테스트 용 mock MockPlayerPartyController
 */
class MockPlayerPartyController :
    public zoneserver::gc::PlayerPartyController,
    public sne::test::CallCounter
{
public:
    MockPlayerPartyController(zoneserver::go::Entity* owner);

private: // = rpc::PartyRpc overriding
    OVERRIDE_SRPC_METHOD_6(onPartyMemberSubInfo,
        ObjectId, objectId, CharacterClass, characterClass,
        CreatureLevel, level, HitPoints, hitPoints, 
        ManaPoints, manaPoints, Position, position);

    OVERRIDE_SRPC_METHOD_2(evPartyMemberLevelup,
        ObjectId, objectId, CreatureLevel, level);

    OVERRIDE_SRPC_METHOD_2(evPartyMemberMoved,
        ObjectId, objectId, Position, position);

public:
    ErrorCode lastErrorCode_;
};
