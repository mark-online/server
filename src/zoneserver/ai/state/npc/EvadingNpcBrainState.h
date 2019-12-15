#pragma once

#include "../BrainState.h"
#include "../../Brain.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../service/movement/MovementManager.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class EvadingNpcBrainState
 * 스폰 위치로 복귀하는 상태
 */
class EvadingNpcBrainState : public BrainState
{
private:
    virtual void entry(Brain& brain) {
        assert(brain.getOwner().queryThinkable() != nullptr);

        go::Npc& owner = static_cast<go::Npc&>(brain.getOwner());
        owner.setEvadeState();
        owner.queryTargetSelectable()->unselectTarget();

        go::Liveable* liveable = brain.getOwner().queryLiveable();
        if (liveable != nullptr) {
            liveable->getCreatureStatus().restorePoints();
        }

        movementManager_ = &owner.queryMoveable()->getMovementManager();
        movementManager_->setReturnMovement();

        canUpdate_ = true;
    }

    virtual void exit(Brain& brain) {
        go::Npc& npc = static_cast<go::Npc&>(brain.getOwner());
        npc.unsetEvadeState();
    }

    virtual void update(Brain& brain, msec_t /*diff*/) {
        if (! canUpdate_) {
            return;
        }

        if (! movementManager_->isCompleted()) {
            return;
        }

        brain.queryStateable()->handleEvent(eventReturned);
        canUpdate_ = false;
    }

private:
    // = BrainState overriding
    virtual bool handleEvent(Brain& brain, sne::core::fsm::Event& event) const {
        BrainEvent& brainEvent = static_cast<BrainEvent&>(event);
        const int eventId = brainEvent.getEventId();
        if (eventId == eventReturned) {
            return brainEvent.handle(brain);
        }
        return true; // 다른 이벤트는 처리하지 않는다
    }

private:
    MovementManager* movementManager_;
    bool canUpdate_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
