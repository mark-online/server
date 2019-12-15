#pragma once

#include "BrainState.h"
#include "../event/BrainEvent.h"
#include <sne/core/container/Containers.h>

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class GlobalBrainState
 * 전역 상태
 * - 이벤트 처리를 위한 상태
 */
class GlobalBrainState : public BrainState
{
    typedef sne::core::HashSet<const sne::core::fsm::Event*> BrainEventSet;

public:
    void registerBrainEvent(BrainEvent& event) {
        handlableBrainEvents_.insert(&event);
    }

private:
    // = BrainState overriding
    virtual bool handleEvent(Brain& brain, sne::core::fsm::Event& event) const {
        if (! canHandleEvent(event)) {
            return false;
        }
        return static_cast<BrainEvent&>(event).handle(brain);
    }

private:
    bool canHandleEvent(const sne::core::fsm::Event& event) const {
        const BrainEventSet::const_iterator pos = handlableBrainEvents_.find(&event);
        return pos != handlableBrainEvents_.end();
    }

private:
    BrainEventSet handlableBrainEvents_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
