#pragma once

#include <gideon/cs/shared/data/Code.h>
#include <esut/Random.h>

namespace gdt {
class evt_t;
} // namespace gdt {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

enum {
    maxActions = 3
};


/// Event AI가 엔티티(entityCode)를 처리할 수 있는가?
bool isAllowedEntityCode(DataCode entityCode);


/**
 * @enum EventPhase
 * 이벤트 단계
 */
enum EventPhase
{
    evpAlways  = 0,
    evpDefault = 0,
    evp1       = 1,
    evpMax     = 7,

    evpCount   = 6
};


inline bool isValid(EventPhase p)
{
    return (evpDefault <= p) && (p <= evpMax);
}


inline bool isValidIncDec(EventPhase p)
{
    return (evpDefault < p) && (p <= evpMax);
}


/**
 * @enum EventPhaseBits
 */
enum EventPhaseBits
{
    evpbAlways = 0,
    evpb1Bit   = 1,
    evpb2Bit   = 2,
    evpb3Bit   = 4,
    evpb4Bit   = 8,
    evpb5Bit   = 16,
    evpb6Bit   = 32,
    evpbAll    = (evpb1Bit + evpb2Bit + evpb3Bit + evpb4Bit + evpb5Bit + evpb6Bit)
};


inline bool isValid(EventPhaseBits bits)
{
    return (evpbAlways <= bits) && (bits <= evpbAll);
}


/**
 * @enum EventType
 */
enum EventType
{
    evtUnknown = -1,

    evtTimerInCombat = 0,
    evtTimerInPeace = 1,

    evtLosInPeace = 2,

    evtHp = 10,

    evtSpawned = 100,
    evtDespawned = 101,
    evtDied = 102,

    evtWanderPaused = 131,
    evtPathNodeArrived = 132,

    evtDeviceActivated = 200,
    evtItemUsed = 201,

    evtActivationMissionCompleted = 210,
    evtKillMissionCompleted = 211,

    evtBoundary
};

inline bool isValid(EventType value)
{
    return (evtUnknown < value) && (value < evtBoundary);
}


inline EventType toEventType(int value)
{
    return static_cast<EventType>(value);
}


/**
 * @enum EventFlags
 */
enum EventFlags
{
    evfRepeatable = 1
};


inline EventFlags toEventFlags(int value)
{
    return static_cast<EventFlags>(value);
}


class Action;
class EntityActionCallback;


/**
 * @class EventCallback
 */
class EventCallback
{
public:
    virtual ~EventCallback() {}

    virtual bool updateRepeatTimer(msec_t repeatMin, msec_t repeatMax) = 0;
};


/**
 * @class Event
 * - 주의: 공유해서 사용하므로 상태를 가지면 안된다!!
 */
class Event : public boost::noncopyable
{
    typedef std::array<std::unique_ptr<Action>, maxActions> Actions;

public:
    virtual ~Event() {}

    virtual bool initialize(const gdt::evt_t& eventAiTemplate);

    virtual bool process(go::Entity& owner, go::Entity* target = nullptr) const = 0;

    virtual bool updateRepeatTimer(EventCallback& callback) const {
        callback;
        return false;
    }

public:
    void processActions(go::Entity& owner, EntityActionCallback* callback, go::Entity* target = nullptr) const;

    void setAction(std::unique_ptr<Action> action, int index);

public:
    DataCode getEntityCode() const { return entityCode_; }
    EventPhaseBits getEventPhaseMask() const { return eventPhaseMask_; }
    EventType getEventType() const { return eventType_; }

    bool isRepeatable() const {
        return (eventFlags_ & evt::evfRepeatable) != 0;
    }

    bool hasChance() const {
        return eventChance_ > uint8_t(esut::random(0, 99));
    }

protected:
    int getParam1() const { return param1_; }
    int getParam2() const { return param2_; }
    int getParam3() const { return param3_; }
    int getParam4() const { return param4_; }

private:
    DataCode entityCode_;
    EventType eventType_;
    EventFlags eventFlags_;
    EventPhaseBits eventPhaseMask_;
    uint8_t eventChance_;
    int param1_;
    int param2_;
    int param3_;
    int param4_;

    Actions actions_;
};


/**
 *enum ActionType
 **/
enum ActionType
{
    actUnknown = -1,

    actNothing = 0,
    actThreat = 1,
    actFlee = 9,
    actFleeForAssist = 10,

    actSetPhase = 20,
    actIncreasePhase = 21,
    actRandomPhaseRange = 22,

    actPlayAction = 51,

    actSummonNpc = 100,
    actFormUp = 101,
    actTransform = 102,

    actTeleport = 200,
    actMoveThruPath = 201,

    actActivateDevice = 210,

    actBoundary
};


inline bool isValid(ActionType value)
{
    return (actUnknown < value) && (value < actBoundary);
}


/**
 * @class EntityActionCallback
 */
class EntityActionCallback
{
public:
    virtual ~EntityActionCallback() {}

    virtual void setPhase(EventPhase p) = 0;
    virtual void increasePhase(EventPhase p) = 0;
    virtual void decreasePhase(EventPhase p) = 0;

    virtual bool isInPhase(EventPhaseBits bits) const = 0;
};


/**
 * @class Action
 */
class Action : public boost::noncopyable
{
public:
    Action() {}
    virtual ~Action() {}

    virtual bool initialize(EventType eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    virtual void process(EntityActionCallback* callback, go::Entity& owner, go::Entity* target) const = 0;

protected:
    ActionType getActionType() const {
        return actionType_;
    }

    int getParam1() const {
        return param1_;
    }

    int getParam2() const {
        return param2_;
    }

    int getParam3() const {
        return param3_;
    }

    int getParam4() const {
        return param4_;
    }

    int getParam5() const {
        return param5_;
    }

private:
    ActionType actionType_;
    int param1_;
    int param2_;
    int param3_;
    int param4_;
    int param5_;
};

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
