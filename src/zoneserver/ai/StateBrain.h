#pragma once

#include "Brain.h"

namespace gideon { namespace zoneserver { namespace ai {

namespace evt {
class EventTrigger;
} // namespace evt {

/**
 * @class StateBrain
 * FSM 기반의 AI
 */
class StateBrain :
    public Brain,
    private Stateable
{
public:
    StateBrain(go::Entity& owner, std::unique_ptr<evt::EventTrigger> eventTrigger);
    virtual ~StateBrain();

protected:
    // = Brain overriding
    virtual void activate();
    virtual void deactivate();

    virtual void analyze(GameTime diff);

    virtual void spawned();
    virtual void despawned();
    virtual void died();
    virtual void combatStarted();
    virtual void combatStopped();
    virtual void movedInLineOfSight(go::Entity& entity);
    virtual void wanderPaused();
    virtual void pathNodeArrived();

    virtual Stateable* queryStateable() { return this; }
    virtual const Stateable* queryStateable() const { return this; }

protected:
    // = Stateable overriding
    virtual void setNextState(int stateId);

    BrainStateMachine& getStateMachine() {
        return *stateMachine_;
    }

private:
    bool isStateChanged() const {
        return nextStateId_ != -1;
    }

private:
    std::unique_ptr<BrainStateMachine> stateMachine_;
    std::unique_ptr<ai::evt::EventTrigger> eventTrigger_;
    int nextStateId_;

    bool isActivated_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
