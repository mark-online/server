#pragma once

#include "MapRegion.h"
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class NormalMapRegion
 * 일반 영역
 */
class ZoneServer_Export NormalMapRegion :
    public MapRegion,
    public sne::core::ThreadSafeMemoryPoolMixin<NormalMapRegion>
{
public:
    NormalMapRegion(WorldMap& worldMap, const datatable::RegionTemplate& regionTemplate,
        const datatable::RegionCoordinates::CoordinatesList& coordinates) :
        MapRegion(worldMap, regionTemplate, coordinates) {}

private:
    virtual void entitySpawned(go::Entity& entity);
    virtual void entityDespawned(go::Entity& entity);
	virtual void finalizeForSpawnRegionTickable() {}

private:
    virtual go::Entity* getFirstEntity(ObjectType objectType, DataCode dataCode);
    virtual size_t getEntityCount(ObjectType objectType) const;
};

}} // namespace gideon { namespace zoneserver {
