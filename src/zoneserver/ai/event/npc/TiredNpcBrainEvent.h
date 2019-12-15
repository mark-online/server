#pragma once

#include "../BrainEvent.h"
#include "../../Brain.h"
#include "../../state/npc/NpcBrainStateDef.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class TiredNpcBrainEvent
 */
class TiredNpcBrainEvent : public BrainEvent
{
public:
    TiredNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
        brain.queryStateable()->setNextState(stateThinking);
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
