#pragma once

#include "../Brain.h"
#include <sne/core/fsm/State.h>

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class BrainState
 */
class BrainState : public sne::core::fsm::State<Brain>
{
    typedef sne::core::fsm::State<Brain> Parent;

protected:
    // = sne::core::fsm::State overriding
    virtual void entry(Brain& /*brain*/) {}
    virtual void exit(Brain& /*brain*/) {}
    virtual void update(Brain& /*brain*/, msec_t /*diff*/) {}

    virtual bool handleEvent(Brain& /*brain*/,
        sne::core::fsm::Event& /*event*/) const {
        return false;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
