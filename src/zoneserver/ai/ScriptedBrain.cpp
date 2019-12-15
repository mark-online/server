#include "ZoneServerPCH.h"
#include "ScriptedBrain.h"
#include "state/BrainStateMachine.h"
#include "state/BrainState.h"
#include "evt/EventTrigger.h"
#include "../model/gameobject/Entity.h"

namespace gideon { namespace zoneserver { namespace ai {

ScriptedBrain::ScriptedBrain(go::Entity& owner) :
    Brain(owner),
    isActivated_(false)
{
}


ScriptedBrain::~ScriptedBrain()
{
    isActivated_ = false;
}

// = Brain overriding

void ScriptedBrain::activate()
{
    std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());

    isActivated_ = true;
}


void ScriptedBrain::deactivate()
{
    std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());

    isActivated_ = false;
}

// = Scriptable overriding

}}} // namespace gideon { namespace zoneserver { namespace ai {
