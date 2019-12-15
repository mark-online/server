#include "ZoneServerTestPCH.h"
#include "MockPlayerPartyController.h"

using namespace gideon::zoneserver;

MockPlayerPartyController::MockPlayerPartyController(zoneserver::go::Entity* owner) :
    PlayerPartyController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}

// = rpc::PartyRpc overriding

DEFINE_SRPC_METHOD_6(MockPlayerPartyController, onPartyMemberSubInfo,
    ObjectId, objectId, CharacterClass, characterClass,
    CreatureLevel, level, HitPoints, hitPoints, 
    ManaPoints, manaPoints, Position, position)
{
    addCallCount("onPartyMemberSubInfo");
    objectId, characterClass, level, hitPoints, manaPoints, position;
}


DEFINE_SRPC_METHOD_2(MockPlayerPartyController, evPartyMemberLevelup,
    ObjectId, objectId, CreatureLevel, level)
{
    addCallCount("evPartyMemberLevelup");
    objectId, level;
}


DEFINE_SRPC_METHOD_2(MockPlayerPartyController, evPartyMemberMoved,
    ObjectId, objectId, Position, position)
{
    addCallCount("evPartyMemberMoved");
    objectId, position;
}
