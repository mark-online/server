#pragma once

#include "EventTriggerDef.h"
#include <gideon/cs/shared/data/Time.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/container/Containers.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

namespace {
class EventTriggerEventHolder;
} // namespace {


typedef sne::core::MultiMap<evt::EventType, EventTriggerEventHolder*> EventTriggerEventMultiMap;


/**
 * @class EventTrigger
 */
class EventTrigger :
    private EntityActionCallback
{
    enum {
        eventUpdateInterval = 500 //< ms
    };

    typedef std::mutex LockType;

public:
    EventTrigger(go::Entity& owner);

    void update(GameTime diff);

    void reset();

    void resetEventUpdateTime();

public:
    void spawned();
    void despawned();
    void died();
    void combatStarted();
    void combatStopped();
    void movedInLineOfSight(go::Entity& entity);
    void wanderPaused();
    void pathNodeArrived();

    // = Device 관련
    void deviceActivated();

public:
    void processEvents(EventType eventType, go::Entity* entity = nullptr);

public:
    /// @internal
    const EventTriggerEventMultiMap& getEventTriggerEventMultiMap() const {
        return eventAiEventMultiMap_;
    }

private:
    void asyncProcessEvent(sne::base::RunnablePtr task);

    void resetEventUpdateTime_i() {
        eventUpdateLeftTime_ = eventUpdateInterval;
        eventTimeDiff_ = 0;
    }

private:
    bool hasEvent(evt::EventType eventType) const {
        return eventAiEventMultiMap_.find(eventType) != eventAiEventMultiMap_.end();
    }

    // = EntityActionCallback overriding
private:
    // TODO: EventTrigger로 옮길 것!!!
    virtual void setPhase(EventPhase p) {
        eventPhase_ = p;
    }

    virtual void increasePhase(EventPhase p) {
        if (p >= evpDefault) {
            eventPhase_ = EventPhase(eventPhase_ + p);
        }
        else {
            decreasePhase(EventPhase(std::abs(p)));
        }
    }

    virtual void decreasePhase(EventPhase p) {
        eventPhase_ = EventPhase(eventPhase_ - ((eventPhase_ < p) ? p - eventPhase_ : p));
    }

    virtual bool isInPhase(EventPhaseBits bits) const {
        if (bits == evpbAlways) {
            return true;
        }
        return ((1 << (eventPhase_ - 1)) & bits) != 0;
    }

private:
    go::Entity& owner_;
    EventTriggerEventMultiMap eventAiEventMultiMap_;
    GameTime eventUpdateLeftTime_; //< 다음 갱신까지 남은 시간
    GameTime eventTimeDiff_;
    EventPhase eventPhase_;

    mutable LockType lock_;
};

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
