#pragma once

#include "../BrainEvent.h"
#include "../../Brain.h"
#include "../../state/npc/NpcBrainStateDef.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class ActiveNpcBrainEvent
 */
class ActiveNpcBrainEvent : public BrainEvent
{
public:
    ActiveNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
        brain.queryStateable()->setNextState(stateActive);
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
