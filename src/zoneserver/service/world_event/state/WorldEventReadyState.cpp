#include "ZoneServerPCH.h"
#include "WorldEventReadyState.h"
#include "../WorldEventPhase.h"
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/datatable/WorldEventTable.h>

namespace gideon { namespace zoneserver {



WorldEventReadyState::WorldEventReadyState(WorldEvent::EventPhaseMap& eventPhaseMap) :
    eventPhaseMap_(eventPhaseMap)
{
}

    
bool WorldEventReadyState::shouldNextState() const
{
    WorldEvent::EventPhaseMap::iterator pos = eventPhaseMap_.begin();
    WorldEvent::EventPhaseMap::iterator end = eventPhaseMap_.end();
    for (; pos != end; ++pos) {
        WorldEventPhase* eventPhase = (*pos).second;
        if (! eventPhase->isReady()) {
            return false;
        }
    }
    return true;
}


void WorldEventReadyState::reInitialize()
{
    WorldEvent::EventPhaseMap::iterator pos = eventPhaseMap_.begin();
    WorldEvent::EventPhaseMap::iterator end = eventPhaseMap_.end();
    for (; pos != end; ++pos) {
        WorldEventPhase* phase = (*pos).second;
        phase->ready();        
    }
}


void WorldEventReadyState::setParty(WorldEventMissionCode missionCode, PartyId partyId)
{
    WorldEventPhase* phase = getWorldEventPhase(missionCode);
    if (phase) {
        phase->setParty(missionCode, partyId);
    }
}


WorldEventPhase* WorldEventReadyState::getWorldEventPhase(WorldEventMissionCode missionCode)
{
    WorldEvent::EventPhaseMap::iterator pos = eventPhaseMap_.begin();
    WorldEvent::EventPhaseMap::iterator end = eventPhaseMap_.end();

    for (pos; pos != end; ++pos) {
        WorldEventPhase* currentPhases = (*pos).second;

        if (currentPhases->hasMission(missionCode)) {
            return currentPhases;        
        }
    }

    return nullptr;
}


}} // namespace gideon { namespace zoneserver {
