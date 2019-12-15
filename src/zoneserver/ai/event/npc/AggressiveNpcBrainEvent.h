#pragma once

#include "../BrainEvent.h"
#include "../../NpcStateBrain.h"
#include "../../state/npc/NpcBrainStateDef.h"
#include "../../../model/state/MoveState.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class AggressiveNpcBrainEvent
 */
class AggressiveNpcBrainEvent : public BrainEvent
{
public:
    AggressiveNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
        if (brain.canStrikeBack()) {
            if (! brain.queryStateable()->isInState(stateAttacking)) {
                static_cast<NpcStateBrain&>(brain).callLink();
                brain.queryStateable()->setNextState(stateAttacking);
            }
        }
        else if (brain.shouldFlee()) {
			MoveState* moveState = brain.getOwner().queryMoveState();
			if (! moveState->isRooted()) {
				brain.queryStateable()->setNextState(stateFleeing);
			}
        }
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
