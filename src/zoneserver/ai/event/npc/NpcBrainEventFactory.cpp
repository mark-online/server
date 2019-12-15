#include "ZoneServerPCH.h"
#include "NpcBrainEventFactory.h"
#include "ActiveNpcBrainEvent.h"
#include "NothingTodoNpcBrainEvent.h"
#include "AttackedNpcBrainEvent.h"
#include "TiredNpcBrainEvent.h"
#include "ReturnedNpcBrainEvent.h"
#include "AggressiveNpcBrainEvent.h"
#include "ThreatedNpcBrainEvent.h"
#include "DialogRequestedNpcBrainEvent.h"
#include "FearedNpcBrainEvent.h"
#include "SeekAssistNpcBrainEvent.h"

namespace gideon { namespace zoneserver { namespace ai {

SNE_DEFINE_SINGLETON(NpcBrainEventFactory);

NpcBrainEventFactory s_npcBrainEventFactory;

NpcBrainEventFactory::NpcBrainEventFactory()
{
    initEvents();

    NpcBrainEventFactory::instance(this);
}


NpcBrainEventFactory::~NpcBrainEventFactory()
{
    for (BrainEventMap::value_type& value : brainEventMap_) {
        BrainEvent* state = value.second;
        delete state;
    }
}


BrainEvent* NpcBrainEventFactory::getBrainEvent(NpcBrainEventId id)
{
    if (! isValid(id)){
        return nullptr;
    }

    const BrainEventMap::iterator pos = brainEventMap_.find(id);
    if (pos != brainEventMap_.end()) {
        return (*pos).second;
    }
    assert(false && "Can't find event");
    return nullptr;
}


void NpcBrainEventFactory::initEvents()
{
    brainEventMap_.insert(
        BrainEventMap::value_type(eventActive,
            new ActiveNpcBrainEvent(eventActive)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventNothingToDo,
            new NothingTodoNpcBrainEvent(eventNothingToDo)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventAttacked,
            new AttackedNpcBrainEvent(eventAttacked)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventTired,
            new TiredNpcBrainEvent(eventTired)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventReturned,
            new ReturnedNpcBrainEvent(eventReturned)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventAggressive,
            new AggressiveNpcBrainEvent(eventAggressive)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventThreated,
            new ThreatedNpcBrainEvent(eventThreated)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventDialogRequested,
            new DialogRequestedNpcBrainEvent(eventDialogRequested)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventFeared,
            new FearedNpcBrainEvent(eventFeared)));
    brainEventMap_.insert(
        BrainEventMap::value_type(eventSeekAssist,
            new SeekAssistNpcBrainEvent(eventSeekAssist)));
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
