#pragma once

#include <sne/core/fsm/Event.h>

namespace gideon { namespace zoneserver { namespace ai {

class Brain;

/**
 * @class BrainEvent
 */
class BrainEvent : public sne::core::fsm::Event
{
public:
    BrainEvent(int eventId) :
        eventId_(eventId) {}

    int getEventId() const {
        return eventId_;
    }

public:
    virtual bool handle(Brain& brain) = 0;

private:
    const int eventId_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
