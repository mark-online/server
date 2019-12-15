#pragma once

#include "EventTriggerDef.h"
#include <gideon/cs/datatable/SpawnTemplate.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/EntityPathInfo.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

/**
 * @class EventTimerInCombat
 */
class EventTimerInCombat : public Event
{
protected:
    virtual bool updateRepeatTimer(EventCallback& callback) const;

private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;

private:
    msec_t getInitialMin() const {
        return static_cast<msec_t>(getParam1());
    }
    msec_t getInitialMax() const {
        return static_cast<msec_t>(getParam2());
    }

    msec_t getRepeatMin() const {
        return static_cast<msec_t>(getParam3());
    }
    msec_t getRepeatMax() const {
        return static_cast<msec_t>(getParam4());
    }
};


/**
 * @class EventTimerInPeace
 */
class EventTimerInPeace : public EventTimerInCombat
{
private:
    virtual bool process(go::Entity& owner, go::Entity* target) const;

private:
    msec_t getInitialMin() const {
        return static_cast<msec_t>(getParam1());
    }
    msec_t getInitialMax() const {
        return static_cast<msec_t>(getParam2());
    }

    msec_t getRepeatMin() const {
        return static_cast<msec_t>(getParam3());
    }
    msec_t getRepeatMax() const {
        return static_cast<msec_t>(getParam4());
    }
};


/**
 * @class EventLosInPeace
 */
class EventLosInPeace : public Event
{
protected:
    virtual bool updateRepeatTimer(EventCallback& callback) const;

private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;

private:
    bool isHostileOnly() const {
        return getParam1() == 1;
    }

    int32_t getMaxDistance() const {
        return getParam2();
    }

    msec_t getRepeatMin() const {
        return static_cast<msec_t>(getParam3());
    }
    msec_t getRepeatMax() const {
        return static_cast<msec_t>(getParam4());
    }
};


/**
 * @class EventHp
 */
class EventHp : public Event
{
protected:
    virtual bool updateRepeatTimer(EventCallback& callback) const;

private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;

private:
    int32_t getMaxHpPct() const {
        return getParam1();
    }

    int32_t getMinHpPct() const {
        return getParam2();
    }

    msec_t getRepeatMin() const {
        return static_cast<msec_t>(getParam3());
    }
    msec_t getRepeatMax() const {
        return static_cast<msec_t>(getParam4());
    }
};


/**
 * @class EventSpawned
 */
class EventSpawned : public Event
{
    enum Condition {
        escUnknown = -1,
        escAlways = 0,
        escMap = 1,
        escBoundary
    };

    inline bool isValid(Condition value) const {
        return (escUnknown < value) && (value < escBoundary);
    }

private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;

private:
    Condition getCondition() const {
        return static_cast<Condition>(getParam1());
    }

    MapCode getMapCode() const {
        return static_cast<MapCode>(getParam2());
    }

    SpawnType getSpawnType() const {
        return static_cast<SpawnType>(getParam3());
    }
};


/**
 * @class EventDespawned
 */
class EventDespawned : public Event
{
private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;
};


/**
 * @class EventDied
 */
class EventDied : public Event
{
private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;
};


/**
 * @class EventWanderPaused
 */
class EventWanderPaused : public Event
{
private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;
};


/**
 * @class EventPathNodeArrived
 */
class EventPathNodeArrived : public Event
{
    enum Condition {
        epnaUnknown = -1,
        epnaAlways = 0,
        epnaForward = 1,
        epnaBackward = 2,
        epnaBoundary
    };

    inline bool isValid(Condition value) const {
        return (epnaUnknown < value) && (value < epnaBoundary);
    }

private:
    virtual bool initialize(const gdt::evt_t& eventAiTemplate);
    virtual bool process(go::Entity& owner, go::Entity* target) const;

private:
    EntityPathCode getEntityPathCode() const {
        return getParam1();
    }

    int32_t getNodeIndex() const {
        return getParam2();
    }

    Condition getCondition() const {
        return static_cast<Condition>(getParam3());
    }
};


/**
 * @class EventDeviceActivated
 */
class EventDeviceActivated : public Event
{
private:
    virtual bool process(go::Entity& owner, go::Entity* target) const;
};


/**
 * @class EventItemUsed
 */
class EventItemUsed : public Event
{
private:
    virtual bool process(go::Entity& owner, go::Entity* target) const;
};


/**
 * @class EventActivationMissionCompleted
 */
class EventActivationMissionCompleted : public Event
{
private:
    virtual bool process(go::Entity& owner, go::Entity* target) const;
};


/**
 * @class EventKillMissionCompleted
 */
class EventKillMissionCompleted : public Event
{
private:
    virtual bool process(go::Entity& owner, go::Entity* target) const;
};

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
