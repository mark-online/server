#pragma once

#include "EventTriggerDef.h"
#include <gideon/cs/datatable/SpawnTemplate.h>
#include <gideon/cs/shared/data/NpcInfo.h>
#include <gideon/cs/shared/data/DeviceInfo.h>
#include <gideon/cs/shared/data/EntityPathInfo.h>
#include <boost/noncopyable.hpp>

namespace gdt {
class evt_t;
} // namespace gdt {

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

/**
 * @class ActionThreat
 */
class ActionThreat : public Action
{
    enum Condition {
        atcUnknown = -1,
        atcValue = 0,
        atcPercent = 1,
        atcBoundary
    };

    inline bool isValid(Condition value) const {
        return (atcUnknown < value) && (value < atcBoundary);
    }

    Condition toCondition(int value) {
        return static_cast<Condition>(value);
    }

private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    Condition getCondition() const {
        return static_cast<Condition>(getParam1());
    }

    int getValue() const {
        return getParam2();
    }

    DataCode getTargetEntityCode() const {
        return getParam3();
    }
};


/**
 * @class ActionFlee
 */
class ActionFlee : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    msec_t getDuration() const {
        return getParam1();
    }
};


/**
 * @class ActionFleeForAssist
 */
class ActionFleeForAssist : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;
};


/**
 * @class ActionSetPhase
 */
class ActionSetPhase : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    EventPhase getPhase() const {
        return EventPhase(getParam1());
    }
};


/**
 * @class ActionIncreasePhase
 */
class ActionIncreasePhase : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    EventPhase getIncreasement() const {
        return EventPhase(getParam1());
    }
    EventPhase getDecreasement() const {
        return EventPhase(getParam1());
    }
};


/**
 * @class ActionRandomPhaseRange
 */
class ActionRandomPhaseRange : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    EventPhase getMinPhase() const {
        return EventPhase(getParam1());
    }
    EventPhase getMaxPhase() const {
        return EventPhase(getParam1());
    }
};


/**
 * @class ActionPlayAction
 */
class ActionPlayAction : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    uint32_t getActionCode() const {
        return getParam1();
    }
};


/**
 * @class ActionSummonNpc
 */
class ActionSummonNpc : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    NpcCode getNpcCode() const {
        return getParam1();
    }

    msec_t getSpawnDelay() const {
        return getParam2();
    }

    msec_t getLifeTime() const {
        return getParam3();
    }

    float32_t getKeepAwayDistance() const {
        return getParam4() / 100.0f;
    }

    SpawnType getSpawnType() const {
        return static_cast<SpawnType>(getParam5());
    }
};


/**
 * @class ActionFormUp
 */
class ActionFormUp : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;
};


/**
 * @class ActionTransform
 */
class ActionTransform : public Action
{
    enum Condition {
        atcUnknown = -1,
        atcNpc = 0,
        atcMonster = 1,
        // atcPlayer = 2, // TODO: 플레이어 변이는 나중에
        atcBoundary
    };

    inline bool isValid(Condition value) const {
        return (atcUnknown < value) && (value < atcBoundary);
    }

    Condition toCondition(int value) {
        return static_cast<Condition>(value);
    }

private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    Condition getCondition() const {
        return static_cast<Condition>(getParam1());
    }

    NpcCode getNpcCode() const {
        return getParam2();
    }

    GameTime getDuration() const {
        return getParam3();
    }
};


/**
 * @class ActionTeleport
 */
class ActionTeleport : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    /// NpcCode or RegionCode
    DataCode getDestinationCode() const {
        return getParam1();
    }
};


/**
 * @class ActionMoveThruPath
 */
class ActionMoveThruPath : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    /// EntityPathCode
    EntityPathCode getEntityPathCode() const {
        return getParam1();
    }
};


/**
 * @class ActionActivateDevice
 */
class ActionActivateDevice : public Action
{
private:
    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const;

private:
    DeviceCode getDeviceCode() const {
        return getParam1();
    }
};

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
