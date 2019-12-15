#pragma once

#include "SpawnMapRegion.h"
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

class PublicQuest;

/**
 * @class PublicQuestSpawnMapRegion
 * Public quest의 Entity spawn을 담당할 map region
 */
class ZoneServer_Export PublicQuestSpawnMapRegion :
    public SpawnMapRegion,
    public sne::core::ThreadSafeMemoryPoolMixin<PublicQuestSpawnMapRegion>
{
    typedef SpawnMapRegion::LockType LockType;

public:
    PublicQuestSpawnMapRegion(WorldMap& worldMap, PublicQuest& publicQuest,
        const datatable::RegionTemplate& regionTemplate,
        const datatable::RegionCoordinates::CoordinatesList& coordinates,
        const datatable::RegionSpawnTable::Spawns& regionSpawns,
        const datatable::PositionSpawnTable::Spawns& positionSpawns,
        WorldMapHelper& worldMapHelper);
    virtual ~PublicQuestSpawnMapRegion();

    virtual bool initialize();
    virtual void finalize();

	void despawnNpcs();

public:
    const PublicQuestGoalInfoMap& getPublicQuestGoalInfoMap() const {
        return goalInfo_;
    }

    uint32_t getScore(CreatureLevel level) const {
        return (10000 * level) / totalLevel_;
    }

private:
    bool updateGoalInfo(NpcCode npcCode);

private:
    // = SpawnMapRegion overriding
    virtual void entitySpawned(go::Entity& entity);

private:
    PublicQuest& publicQuest_;
    uint32_t totalLevel_;
    PublicQuestGoalInfoMap goalInfo_;
};

}} // namespace gideon { namespace zoneserver {
