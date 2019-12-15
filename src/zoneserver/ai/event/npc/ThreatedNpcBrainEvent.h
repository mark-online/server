#pragma once

#include "../BrainEvent.h"
#include "../../Brain.h"
#include "../../state/npc/NpcBrainStateDef.h"
#include "../../../model/state/MoveState.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class ThreatedNpcBrainEvent
 */
class ThreatedNpcBrainEvent : public BrainEvent
{
public:
    ThreatedNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
        if (brain.canStrikeBack()) {
            brain.queryStateable()->setNextState(stateAttacking);
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
