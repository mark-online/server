#pragma once

#include "NpcBrainEventDef.h"
#include "../BrainEvent.h"
#include <sne/core/container/Containers.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class NpcBrainEventFactory
 * - Flyweight pattern
 */
class NpcBrainEventFactory : public boost::noncopyable
{
    typedef sne::core::HashMap<NpcBrainEventId, BrainEvent*> BrainEventMap;

    SNE_DECLARE_SINGLETON(NpcBrainEventFactory);

public:
    NpcBrainEventFactory();
    ~NpcBrainEventFactory();

    BrainEvent* getBrainEvent(NpcBrainEventId id);

private:
    void initEvents();

private:
    BrainEventMap brainEventMap_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {

#define NPC_BRAIN_EVENT_FACTORY \
    gideon::zoneserver::ai::NpcBrainEventFactory::instance()