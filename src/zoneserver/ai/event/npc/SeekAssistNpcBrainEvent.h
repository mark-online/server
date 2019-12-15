#pragma once

#include "../BrainEvent.h"
#include "../../Brain.h"
#include "../../state/npc/NpcBrainStateDef.h"
#include "../../../model/state/MoveState.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class SeekAssistNpcBrainEvent
 */
class SeekAssistNpcBrainEvent : public BrainEvent
{
public:
    SeekAssistNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
		MoveState* moveState = brain.getOwner().queryMoveState();
		if (! moveState->isRooted()) {
			brain.queryStateable()->setNextState(stateSeekingAssist);
		}
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
