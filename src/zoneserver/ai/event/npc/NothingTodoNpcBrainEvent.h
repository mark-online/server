#pragma once

#include "../BrainEvent.h"
#include "../../Brain.h"
#include "../../state/npc/NpcBrainStateDef.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class NothingTodoNpcBrainEvent
 */
class NothingTodoNpcBrainEvent : public BrainEvent
{
public:
    NothingTodoNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
        brain.queryStateable()->setNextState(stateIdle);
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
