#include "ZoneServerPCH.h"
#include "WorldEventPhase.h"
#include "WorldEventMission.h"
#include "../../world/WorldMap.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/WorldEventCallback.h"
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionSpawnTable.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>


namespace gideon { namespace zoneserver {


WorldEventPhase::WorldEventPhase()
{
}


WorldEventPhase::~WorldEventPhase()
{
}


bool WorldEventPhase::initialize(const datatable::RegionTable& regionTable,
    const datatable::RegionCoordinates& regionCoordinates,
    const datatable::WorldEventMissionTable::WorldEventMissions& missions,
    WorldMap& worldMap, WorldMapHelper& worldMapHelper,
    WorldEventScoreCallback& scoreCallback)
{
    datatable::WorldEventMissionTable::WorldEventMissions::const_iterator pos = missions.begin();
    const datatable::WorldEventMissionTable::WorldEventMissions::const_iterator end = missions.end();
    for (; pos != end; ++pos) {
        const gdt::world_event_mission_t* mission = (*pos);
        WorldEventMission* eventMission = new WorldEventMission(worldMap, worldMapHelper,  scoreCallback, *mission);
        if (! eventMission->initialize(regionTable, regionCoordinates)) {
            return false;
        }
        missions_.push_back(eventMission);
    }

    return true;
}


void WorldEventPhase::ready()
{
    for (WorldEventMission* mission : missions_) {
        mission->ready();
    }
}


void WorldEventPhase::active()
{
    for (WorldEventMission* mission : missions_) {
        mission->activate();
    }
}


void WorldEventPhase::deactive()
{
    for (WorldEventMission* mission : missions_) {
        mission->deactivate();
    }
}



void WorldEventPhase::update()
{
    for (WorldEventMission* mission : missions_) {
        mission->update();
    }
}


void WorldEventPhase::setParty(WorldEventMissionCode missionCode, PartyId partyId)
{
    WorldEventMission* mission = getMission(missionCode);
    if (mission) {
        mission->setParty(partyId);
    }
}


void WorldEventPhase::setFailMission()
{
    for (WorldEventMission* mission : missions_) {
        mission->setFailMission();
    }
}


void WorldEventPhase::enter(WorldEventMissionCode missionCode, go::Entity& player)
{
    WorldEventMission* mission = getMission(missionCode);
    if (mission) {
        mission->enter(player);
    }
}


void WorldEventPhase::leave(WorldEventMissionCode missionCode, ObjectId playerId)
{
    WorldEventMission* mission = getMission(missionCode);
    if (mission) {
        mission->leave(playerId);
    }
}


void WorldEventPhase::leave(ObjectId playerId)
{
    for (WorldEventMission* mission : missions_) {
        mission->leave(playerId);
    }
}

void WorldEventPhase::fillWorldEventMissionInfo(WorldEventMissionInfos& phaseInfos) const
{
    for (const WorldEventMission* mission : missions_) {
        mission->fillWorldEventMissionInfo(phaseInfos);
    }
}


bool WorldEventPhase::isComplete() const
{
    for (const WorldEventMission* mission : missions_) {
        if (! mission->isCompleteMission()) {
            return false;
        }
    }
    return true;
}


bool WorldEventPhase::isFail() const
{
    for (const WorldEventMission* mission : missions_) {
        if (mission->isFailMission()) {
            return true;
        }
    }
    return false;
}


bool WorldEventPhase::isReady() const
{
    for (const WorldEventMission* mission : missions_) {
        if (! mission->hasParty()) {
            return false;
        }
    }
    return true;
}


bool WorldEventPhase::hasMission(WorldEventMissionCode missionCode) const
{
    return getMission(missionCode) != nullptr;
}


WorldEventMission* WorldEventPhase::getMission(WorldEventMissionCode missionCode)
{
    for (WorldEventMission* mission : missions_) {
        if (mission->getMissionCode() == missionCode) {
            return mission;
        }
    }
    return nullptr;
}


const WorldEventMission* WorldEventPhase::getMission(WorldEventMissionCode missionCode) const
{
    for (const WorldEventMission* mission : missions_) {
        if (mission->getMissionCode() == missionCode) {
            return mission;
        }
    }
    return nullptr;
}


}}  // namespace gideon { namespace zoneserver {