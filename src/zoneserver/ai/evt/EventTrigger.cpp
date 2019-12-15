#include "ZoneServerPCH.h"
#include "EventTrigger.h"
#include "EventTriggerManager.h"
#include "../../model/gameobject/Entity.h"
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

namespace {

/**
 * @struct EventTriggerEventHolder
 */
class EventTriggerEventHolder :
    private EventCallback
{
public:
    EventTriggerEventHolder(go::Entity& owner, Event& event, EntityActionCallback& actionCallback) :
        owner_(owner),
        event_(event),
        actionCallback_(actionCallback) {
        reset();
    }

    void reset() {
        isEnabled_ = true;
        nextTime_ = 0;
    }

    void process(go::Entity* target = nullptr) {
        if ((! isEnabled_) || (nextTime_ > 0)) {
            return;
        }

        if (! actionCallback_.isInPhase(event_.getEventPhaseMask())) {
            return;
        }

        if (! event_.process(owner_, target)) {
            return;
        }

        updateRepeatTimer();

        if (! event_.isRepeatable()) {
            isEnabled_ = false; // Disable non-repeatable events
        }

        if (! event_.hasChance()) {
            return;
        }

        event_.processActions(owner_, &actionCallback_, target);
    }

    void updateRepeatTimer() {
        if (event_.updateRepeatTimer(*this)) {
            isEnabled_ = true;
        }
    }

    bool decreaseTimer(GameTime eventTimeDiff) {
        if (! isEnabled_) {
            return true;
        }

        if (nextTime_ > 0) {
            if (nextTime_ >= eventTimeDiff) {
                // Do not decrement timers if event cannot trigger in this phase
                if (actionCallback_.isInPhase(event_.getEventPhaseMask())) {
                    nextTime_ -= eventTimeDiff;
                }
                return true;
            }
            else {
                nextTime_ = 0;
            }
        }
        return false;
    }

public:
    EventType getEventType() const {
        return event_.getEventType();
    }

    // = EventCallback overriding
private:
    virtual bool updateRepeatTimer(GameTime repeatMin, GameTime repeatMax) {
        if (! event_.isRepeatable()) {
            return false;
        }

        if (repeatMin == repeatMax) {
            nextTime_ = repeatMin;
        }
        else if (repeatMax > repeatMin) {
            nextTime_ = esut::random(repeatMin, repeatMax);
        }
        else {
            assert(false);
            isEnabled_ = false;
            return false;
        }
        return true;
    }

private:
    go::Entity& owner_;
    Event& event_;
    EntityActionCallback& actionCallback_;

    GameTime nextTime_;
    bool isEnabled_;
};


/**
 * @class ResetEventTriggerTask
 */
class ResetEventTriggerTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ResetEventTriggerTask>
{
public:
    ResetEventTriggerTask(EventTrigger& evTrigger) :
        evTrigger_(evTrigger) {}

private:
    virtual void run() {
        evTrigger_.reset();
    }

private:
    EventTrigger& evTrigger_;
};


/**
 * @class NormalEventTriggerTask
 */
class NormalEventTriggerTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<NormalEventTriggerTask>
{
public:
    NormalEventTriggerTask(EventTrigger& evTrigger, EventType eventType) :
        evTrigger_(evTrigger),
        eventType_(eventType) {}

private:
    virtual void run() {
        evTrigger_.processEvents(eventType_);
    }

private:
    EventTrigger& evTrigger_;
    EventType eventType_;
};


/**
 * @class CombatEnteredEventTriggerTask
 */
class CombatEnteredEventTriggerTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<CombatEnteredEventTriggerTask>
{
public:
    CombatEnteredEventTriggerTask(EventTrigger& evTrigger) :
        evTrigger_(evTrigger) {}

private:
    virtual void run() {
        for (const EventTriggerEventMultiMap::value_type& value :
            evTrigger_.getEventTriggerEventMultiMap()) {
            EventTriggerEventHolder* eventHolder = value.second;
            switch (eventHolder->getEventType()) {
            //case evtAggro:
            case evtTimerInCombat:
                eventHolder->updateRepeatTimer();
                break;
            }
        }

        evTrigger_.resetEventUpdateTime();
    }

private:
    EventTrigger& evTrigger_;
};


/**
 * @class MovedInLineOfSightEventTriggerTask
 */
class MovedInLineOfSightEventTriggerTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<MovedInLineOfSightEventTriggerTask>
{
public:
    MovedInLineOfSightEventTriggerTask(EventTrigger& evTrigger, go::Entity& entity) :
        evTrigger_(evTrigger),
        entity_(entity) {}

private:
    virtual void run() {
        evTrigger_.processEvents(evtLosInPeace, &entity_);
    }

private:
    EventTrigger& evTrigger_;
    go::Entity& entity_;
};

} // namespace {

// = EventTrigger

EventTrigger::EventTrigger(go::Entity& owner) :
    owner_(owner),
    eventPhase_(evpDefault)
{
    resetEventUpdateTime();

    EventTriggerManager::EventMultiMap* eventMap =
        EVT_MANAGER.getEventMap(owner_.getEntityCode());
    if (! eventMap) {
        return;
    }

    for (EventTriggerManager::EventMultiMap::value_type& value : *eventMap) {
        Event* event = value.second;
        eventAiEventMultiMap_.insert(
            EventTriggerEventMultiMap::value_type(event->getEventType(),
                new EventTriggerEventHolder(owner_, *event, *this)));
    }
}


void EventTrigger::update(GameTime diff)
{
    std::unique_lock<LockType> lock(lock_);

    eventTimeDiff_ += diff;
    if (eventUpdateLeftTime_ <= diff) {
        for (EventTriggerEventMultiMap::value_type& value : eventAiEventMultiMap_) {
            EventTriggerEventHolder* eventHolder = value.second;
            if (eventHolder->decreaseTimer(eventTimeDiff_)) {
                continue;
            }

            switch (eventHolder->getEventType()) {
            case evtTimerInCombat:
            case evtTimerInPeace:
            case evtHp:
                eventHolder->process();
                break;
            }
        }

        resetEventUpdateTime_i();
    }
    else {
        eventUpdateLeftTime_ -= diff;
    }
}


void EventTrigger::spawned()
{
    asyncProcessEvent(
        std::make_unique<ResetEventTriggerTask>(*this));

    if (hasEvent(evtSpawned)) {
        asyncProcessEvent(
            std::make_unique<NormalEventTriggerTask>(*this, evtSpawned));
    }
}


void EventTrigger::despawned()
{
    if (hasEvent(evtDespawned)) {
        asyncProcessEvent(
            std::make_unique<NormalEventTriggerTask>(*this, evtDespawned));
    }
}


void EventTrigger::died()
{
    if (hasEvent(evtDied)) {
        asyncProcessEvent(
            std::make_unique<NormalEventTriggerTask>(*this, evtDied));
    }
}


void EventTrigger::combatStarted()
{
    asyncProcessEvent(
        std::make_unique<CombatEnteredEventTriggerTask>(*this));
}


void EventTrigger::combatStopped()
{
    asyncProcessEvent(
        std::make_unique<ResetEventTriggerTask>(*this));
}


void EventTrigger::movedInLineOfSight(go::Entity& entity)
{
    if (! entity.isValid()) {
        return;
    }

    asyncProcessEvent(
        std::make_unique<MovedInLineOfSightEventTriggerTask>(*this, entity));
}


void EventTrigger::wanderPaused()
{
    if (hasEvent(evtWanderPaused)) {
        asyncProcessEvent(
            std::make_unique<NormalEventTriggerTask>(*this, evtWanderPaused));
    }
}


void EventTrigger::pathNodeArrived()
{
    if (hasEvent(evtPathNodeArrived)) {
        asyncProcessEvent(
            std::make_unique<NormalEventTriggerTask>(*this, evtPathNodeArrived));
    }
}


void EventTrigger::deviceActivated()
{
    assert(owner_.getObjectType() == otDevice);

    asyncProcessEvent(
        std::make_unique<NormalEventTriggerTask>(*this, evtDeviceActivated));
}


void EventTrigger::reset()
{
    std::unique_lock<LockType> lock(lock_);

    eventPhase_ = evpDefault;
    resetEventUpdateTime_i();

    for (EventTriggerEventMultiMap::value_type& value : eventAiEventMultiMap_) {
        EventTriggerEventHolder* eventHolder = value.second;
        eventHolder->reset();
        switch (eventHolder->getEventType()) {
        case evtTimerInPeace:
            eventHolder->updateRepeatTimer();
            break;
        }
    }
}


void EventTrigger::resetEventUpdateTime()
{
    std::unique_lock<LockType> lock(lock_);

    resetEventUpdateTime_i();
}


void EventTrigger::asyncProcessEvent(sne::base::RunnablePtr task)
{
    TASK_SCHEDULER->schedule(std::move(task));
}


void EventTrigger::processEvents(EventType eventType, go::Entity* entity)
{
    EventTriggerEventMultiMap::const_iterator pos = eventAiEventMultiMap_.lower_bound(eventType);
    const EventTriggerEventMultiMap::const_iterator end = eventAiEventMultiMap_.upper_bound(eventType);
    for (; pos != end; ++pos) {
        EventTriggerEventHolder* eventHolder = (*pos).second;
        eventHolder->process(entity);
    }
}

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
