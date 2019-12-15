#pragma once

#include <gideon/cs/shared/data/Time.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} //namespace go {
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace ai {

class BrainState;
class BrainStateMachine;
class AggroList;

/**
 * @class Stateable
 * FSM AI
 */
class Stateable
{
public:
    virtual ~Stateable() {}

public:
    virtual void setNextState(int stateId) = 0;

public:
    /**
     * event를 처리한다
     * - dead-lock을 방지하기 위해 우회 처리
     */
    virtual void asyncHandleEvent(int eventId) = 0;

    virtual void handleEvent(int eventId) = 0;

public:
    virtual BrainStateMachine& getStateMachine() = 0;

    virtual bool isInState(int stateId) const = 0;

protected:
    virtual BrainState& getEntryState() = 0;
    virtual BrainState* getState(int stateId) = 0;
};


/**
 * @class Scriptable
 * Script AI
 */
class Scriptable
{
public:
    virtual ~Scriptable() {}
};


/**
 * @class Brain
 */
class Brain : public boost::noncopyable
{
public:
    Brain(go::Entity& owner);
    virtual ~Brain() {}

    virtual void initialize() = 0;
    virtual void finalize() = 0;

public:
    virtual void activate() = 0;
    virtual void deactivate() = 0;

    virtual void analyze(GameTime diff) = 0;

public:
    virtual Stateable* queryStateable() { return nullptr; }
    virtual const Stateable* queryStateable() const { return nullptr; }

    virtual Scriptable* queryScriptable() { return nullptr; }
    virtual const Scriptable* queryScriptable() const { return nullptr; }

public:
    virtual void spawned() = 0;
    virtual void despawned() = 0;
    virtual void attacked(const go::Entity& attacker) = 0;
    virtual void died() = 0;
    virtual void combatStarted() = 0;
    virtual void combatStopped() = 0;
    virtual void movedInLineOfSight(go::Entity& entity) = 0;
    virtual void wanderPaused() = 0;
    virtual void pathNodeArrived() = 0;

public:
    virtual AggroList* getAggroList() { return nullptr; }

    /// 반격할 수 있는가?
    virtual bool canStrikeBack() const { return false; }

    /// 위협을 받았을 때 도망쳐야 하는가?
    virtual bool shouldFlee() const { return false; }

public:
    go::Entity& getOwner() {
        return owner_;
    }

    const go::Entity& getOwner() const {
        return owner_;
    }

protected:
    /// 소환수가 공격 당했을 때 소환자에게 알린다
    void notifyToSummons(const go::Entity& attacker, int eventId = -1);
    /// 소환수에게 공격을 알린다
    void notifyToSummoner(const go::Entity& attacker, int eventId);

private:
    go::Entity& owner_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
