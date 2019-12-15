#pragma once

#include <gideon/cs/shared/data/WorldEventInfo.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/datatable/WorldEventMissionTable.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace datatable {
class RegionTable;
class RegionCoordinates;
class RegionSpawnTable;
class PositionSpawnTable;
}} // 


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}}

namespace gideon { namespace zoneserver {


class WorldMap;
class WorldMapHelper;
class WorldEventMission;
class WorldEventSpawnMapRegion;
class WorldEventScoreCallback;

const int32_t invalidMission = 0;
const int32_t startMission = 1;

/***
 * @class WorldEventPhase
 ***/
class WorldEventPhase : public sne::core::ThreadSafeMemoryPoolMixin<WorldEventPhase>
{
    typedef std::mutex LockType;
    typedef sne::core::Vector<WorldEventMission*> WorldEventMissions;


public:
    WorldEventPhase();
    ~WorldEventPhase();

    bool initialize(const datatable::RegionTable& regionTable,
        const datatable::RegionCoordinates& regionCoordinates,
        const datatable::WorldEventMissionTable::WorldEventMissions& missions,
        WorldMap& worldMap, WorldMapHelper& worldMapHelper,
        WorldEventScoreCallback& scoreCallback);

    void ready();
    void active();
    void deactive();
    void update();
    void destoryPhase();

    void setParty(WorldEventMissionCode missionCode, PartyId partyId);
    void setFailMission();

    void enter(WorldEventMissionCode missionCode, go::Entity& player);
    void leave(WorldEventMissionCode missionCode, ObjectId playerId);
    void leave(ObjectId playerId);

    void fillWorldEventMissionInfo(WorldEventMissionInfos& missionInfos) const;

    bool isComplete() const;
    bool isFail() const;
    bool isReady() const;
    bool hasMission(WorldEventMissionCode missionCode) const;

private:
    WorldEventMission* getMission(WorldEventMissionCode missionCode);
    const WorldEventMission* getMission(WorldEventMissionCode missionCode) const;

private:
    WorldEventMissions missions_;    
};

}} // namespace gideon { namespace zoneserver {
