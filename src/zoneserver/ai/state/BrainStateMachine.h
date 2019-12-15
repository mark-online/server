#pragma once

#include <sne/core/fsm/StateMachine.h>
#include <sne/core/container/Containers.h>

namespace gideon { namespace zoneserver { namespace ai {

class Brain;
class BrainState;

/**
 * @class BrainStateMachine
 */
class BrainStateMachine : public sne::core::fsm::StateMachine<Brain>
{
    typedef sne::core::fsm::StateMachine<Brain> Parent;

    typedef sne::core::HashMap<int, BrainState*> BrainStateMap;

public:
    BrainStateMachine(Brain& owner);
    virtual ~BrainStateMachine();

public:
    void registerBrainState(int id, BrainState& state) {
        brainStateMap_.emplace(id, &state);
    }

private:
    BrainStateMap brainStateMap_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
