#include "ZoneServerPCH.h"
#include "StateBrain.h"
#include "state/BrainStateMachine.h"
#include "state/BrainState.h"
#include "evt/EventTrigger.h"
#include "../model/gameobject/Entity.h"

namespace gideon { namespace zoneserver { namespace ai {

StateBrain::StateBrain(go::Entity& owner, std::unique_ptr<evt::EventTrigger> eventTrigger) :
    Brain(owner),
    eventTrigger_(std::move(eventTrigger)),
    nextStateId_(-1),
    isActivated_(false)
{
    stateMachine_ = std::make_unique<BrainStateMachine>(*this);
}


StateBrain::~StateBrain()
{
    isActivated_ = false;
}

// = Brain overriding

void StateBrain::activate()
{
    std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());

    stateMachine_->setCurrentState(getEntryState());
    isActivated_ = true;
}


void StateBrain::deactivate()
{
    std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());

    stateMachine_->changeState(getEntryState());
    isActivated_ = false;
}


void StateBrain::analyze(GameTime diff)
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->update(diff);
    }

    sne::core::fsm::State<Brain>* currentState = nullptr;
    sne::core::fsm::State<Brain>* globalState = nullptr;
    {
        std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());

        if (! isActivated_) {
            return;
        }

        if (isStateChanged()) {
            BrainState* nextBrainState = getState(nextStateId_);
            if (nextBrainState != nullptr) {
                (void)stateMachine_->changeState(*nextBrainState);
            }
            else {
                assert(false);
            }
            nextStateId_ = -1;
        }

        currentState = stateMachine_->getCurrentState();
        globalState = stateMachine_->getGlobalState();
    }

    if (globalState != nullptr) {
        globalState->update(*this, diff);
    }

    if (currentState != nullptr) {
        currentState->update(*this, diff);
    }
}


void StateBrain::spawned()
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->spawned();
    }
}


void StateBrain::despawned()
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->despawned();
    }
}


void StateBrain::died()
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->died();
    }
}


void StateBrain::combatStarted()
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->combatStarted();
    }
}


void StateBrain::combatStopped()
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->combatStopped();
    }
}


void StateBrain::movedInLineOfSight(go::Entity& entity)
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->movedInLineOfSight(entity);
    }
}


void StateBrain::wanderPaused()
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->wanderPaused();
    }
}


void StateBrain::pathNodeArrived()
{
    if (eventTrigger_.get() != nullptr) {
        eventTrigger_->pathNodeArrived();
    }
}

// = Stateable overriding

void StateBrain::setNextState(int stateId)
{
    std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());

    if (! isActivated_) {
        return;
    }

    nextStateId_ = stateId;
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
