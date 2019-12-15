#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockWorldMapTable.h>

namespace gideon { namespace servertest {

const gdt::map_t* MockWorldMapTable::getMap(MapCode code) const
{
    const WorldMapMap::const_iterator pos = worldMapMap_.find(code);
    if (pos != worldMapMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


void MockWorldMapTable::fillInitialWorldMap()
{
    const size_t sectorLength = 5;
    const size_t xSectorCount = 10 + 1;
    const size_t ySectorCount = 10 + 1;
    const size_t zSectorCount = 10 + 1;

    gdt::map_t* map = new gdt::map_t;
    map->map_code(globalWorldMapCode);
    map->x_sector_count(xSectorCount);
    map->y_sector_count(ySectorCount);
    map->z_sector_count(zSectorCount);
    map->sector_length(sectorLength);
    worldMapMap_.emplace(map->map_code(), map);
}


void MockWorldMapTable::destroyMaps()
{
    for (const WorldMapMap::value_type& value : worldMapMap_) {
        delete value.second;
    }
}

}} // namespace gideon { namespace servertest {
