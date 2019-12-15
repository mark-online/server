#pragma once

#include "NpcBrainStateDef.h"
#include <sne/core/container/Containers.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace ai {

class BrainState;
class GlobalBrainState;

/**
 * @class NpcBrainStateFactory
 * - Flyweight pattern
 */
class NpcBrainStateFactory : public boost::noncopyable
{
    typedef sne::core::HashMap<NpcBrainStateId, BrainState*> BrainStateMap;

public:
    NpcBrainStateFactory();
    ~NpcBrainStateFactory();

    std::unique_ptr<GlobalBrainState> createGlobalBrainState();

    BrainState* getBrainState(NpcBrainStateId id);

private:
    void initStates();

private:
    BrainStateMap brainStateMap_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
