#include "ZoneServerPCH.h"
#include "NpcBrainStateFactory.h"
#include "IdleNpcBrainState.h"
#include "ActiveNpcBrainState.h"
#include "AttackingNpcBrainState.h"
#include "ThinkingNpcBrainState.h"
#include "EvadingNpcBrainState.h"
#include "RestingNpcBrainState.h"
#include "DialogNpcBrainState.h"
#include "FleeingNpcBrainState.h"
#include "SeekingAssistNpcBrainState.h"
#include "../GlobalBrainState.h"

namespace gideon { namespace zoneserver { namespace ai {

NpcBrainStateFactory::NpcBrainStateFactory()
{
    initStates();
}


NpcBrainStateFactory::~NpcBrainStateFactory()
{
    for (BrainStateMap::value_type& value : brainStateMap_) {
        BrainState* state = value.second;
        delete state;
    }
}


std::unique_ptr<GlobalBrainState> NpcBrainStateFactory::createGlobalBrainState()
{
    return std::make_unique<GlobalBrainState>();
}


BrainState* NpcBrainStateFactory::getBrainState(NpcBrainStateId id)
{
    const BrainStateMap::iterator pos = brainStateMap_.find(id);
    if (pos != brainStateMap_.end()) {
        return (*pos).second;
    }
    assert(false && "Can't find state");
    return nullptr;
}


void NpcBrainStateFactory::initStates()
{
    brainStateMap_.insert(
        BrainStateMap::value_type(stateIdle, new IdleNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateActive, new ActiveNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateAttacking, new AttackingNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateThinking, new ThinkingNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateEvading, new EvadingNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateResting, new RestingNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateDialog, new DialogNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateFleeing, new FleeingNpcBrainState));
    brainStateMap_.insert(
        BrainStateMap::value_type(stateSeekingAssist, new SeekingAssistNpcBrainState));
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
