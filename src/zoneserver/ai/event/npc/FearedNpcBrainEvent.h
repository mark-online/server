#pragma once

#include "../BrainEvent.h"
#include "../../Brain.h"
#include "../../state/npc/NpcBrainStateDef.h"
#include "../../../model/state/MoveState.h"
#include "../../../model/gameobject/Entity.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class FearedNpcBrainEvent
 */
class FearedNpcBrainEvent : public BrainEvent
{
public:
    FearedNpcBrainEvent(NpcBrainEventId eventId) :
        BrainEvent(eventId) {}

private:
    virtual bool handle(Brain& brain) {
		MoveState* moveState = brain.getOwner().queryMoveState();
		if (! moveState->isRooted()) {
			brain.queryStateable()->setNextState(stateFleeing);
		}
        return true;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
