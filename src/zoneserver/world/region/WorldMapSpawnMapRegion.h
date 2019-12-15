#pragma once

#include "SpawnMapRegion.h"
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class WorldMapSpawnMapRegion
 * ThreadSafeMemoryPoolMixin을 사용하기 위한 leaf class
 */
class ZoneServer_Export WorldMapSpawnMapRegion :
    public SpawnMapRegion,
    public sne::core::ThreadSafeMemoryPoolMixin<WorldMapSpawnMapRegion>
{
public:
    WorldMapSpawnMapRegion(WorldMap& worldMap, const datatable::RegionTemplate& regionTemplate,
        const datatable::RegionCoordinates::CoordinatesList& coordinates,
        const datatable::RegionSpawnTable::Spawns* regionSpawns,
        const datatable::PositionSpawnTable::Spawns* positionSpawns,
        WorldMapHelper& worldMapHelper) :
        SpawnMapRegion(worldMap, regionTemplate, coordinates, regionSpawns, positionSpawns,
            worldMapHelper) {}
	// 빌딩은 디비에서 읽기 때문에 초기화 시기를 다르게 한다.
	bool spawnBuildings();

};

}} // namespace gideon { namespace zoneserver {
