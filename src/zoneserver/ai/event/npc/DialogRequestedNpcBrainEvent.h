#pragma once

#include "../BrainEvent.h"
#include "../../NpcStateBrain.h"
#include "../../state/npc/NpcBrainStateDef.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class DialogRequestedNpcBrainEvent
 */
class DialogRequestedNpcBrainEvent : public BrainEvent
{
public:
    DialogRequestedNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
        brain.queryStateable()->setNextState(stateDialog);
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
