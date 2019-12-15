#pragma once

#include "../BrainState.h"
#include "../../Brain.h"
#include "../../aggro/AggroList.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../service/movement/MovementManager.h"
#include "../../../service/movement/MovementManager.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class FleeingNpcBrainState
 * 도망 상태
 */
class FleeingNpcBrainState : public BrainState
{
private:
    virtual void entry(Brain& brain) {
        go::Npc& npc = static_cast<go::Npc&>(brain.getOwner());
        fright_ = brain.getAggroList()->selectVictim(npc);
        npc.setFleeState();
        npc.queryTargetSelectable()->unselectTarget();
        brain.getAggroList()->clear();

        movementManager_ = &npc.queryMoveable()->getMovementManager();
        movementManager_->setTimedFleeingMovement(fright_);

        canUpdate_ = true;
    }

    virtual void exit(Brain& brain) {
        go::Npc& npc = static_cast<go::Npc&>(brain.getOwner());
        npc.unsetFleeState();
    }

    virtual void update(Brain& brain, msec_t /*diff*/) {
        if (! canUpdate_) {
            return;
        }

        if (! movementManager_->isCompleted()) {
            return;
        }

        brain.queryStateable()->handleEvent(eventActive);
        canUpdate_ = false;
    }

private:
    // = BrainState overriding
    virtual bool handleEvent(Brain& brain, sne::core::fsm::Event& event) const {
        BrainEvent& brainEvent = static_cast<BrainEvent&>(event);
        const int eventId = brainEvent.getEventId();
        if (eventId == eventActive) {
            return brainEvent.handle(brain);
        }
        return true; // 다른 이벤트는 처리하지 않는다
    }

private:
    MovementManager* movementManager_;
    go::Entity* fright_;
    bool canUpdate_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
