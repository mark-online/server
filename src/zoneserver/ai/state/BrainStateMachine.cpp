#include "ZoneServerPCH.h"
#include "BrainStateMachine.h"

namespace gideon { namespace zoneserver { namespace ai {

BrainStateMachine::BrainStateMachine(Brain& owner) :
    Parent(owner)
{
}


BrainStateMachine::~BrainStateMachine()
{
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
