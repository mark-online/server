#pragma once

#include "../BrainEvent.h"
#include "../../Brain.h"
#include "../../state/npc/NpcBrainStateDef.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class ReturnedNpcBrainEvent
 */
class ReturnedNpcBrainEvent : public BrainEvent
{
public:
    ReturnedNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
        brain.combatStopped();
        brain.queryStateable()->setNextState(stateResting);
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
