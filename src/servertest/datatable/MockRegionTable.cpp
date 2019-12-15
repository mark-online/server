#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockRegionTable.h>
#include <gideon/servertest/datatable/MockWorldMapTable.h>

namespace gideon { namespace servertest {

const gideon::datatable::RegionTemplate* MockRegionTable::getRegion(RegionCode code) const
{
    const RegionMap::const_iterator pos = regionMap_.find(code);
    if (pos != regionMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockRegionTable::fillCharacterSpawnRegion()
{
    const gideon::datatable::RegionTemplate region(characterSpawnRegionCode);
    regionMap_.emplace(region.regionCode_, region);
}


void MockRegionTable::fillNpcSpawnRegion()
{
    const gideon::datatable::RegionTemplate region(npcSpawnRegionCode);
    regionMap_.emplace(region.regionCode_, region);
}


void MockRegionTable::fillStoreNpcSpawnRegion()
{
    const gideon::datatable::RegionTemplate region(stroeNpcSpawnRegionCode);
    regionMap_.emplace(region.regionCode_, region);
}


void MockRegionTable::fillHarvestSpawnRegion()
{
    const gideon::datatable::RegionTemplate region(harvestRegionCode);
    regionMap_.emplace(region.regionCode_, region);

    const gideon::datatable::RegionTemplate region2(harvestRegionCode2);
    regionMap_.emplace(region2.regionCode_, region2);
}


void MockRegionTable::fillTreasureSpawnRegion()
{
    const gideon::datatable::RegionTemplate region(treasureRegionCode);
    regionMap_.emplace(region.regionCode_, region);

    const gideon::datatable::RegionTemplate region2(treasureRegionCode2);
    regionMap_.emplace(region2.regionCode_, region2);
}


void MockRegionTable::fillRandomDungeonRegion()
{
    const gideon::datatable::RegionTemplate region(randomDungeonRegionCode, firstRandomDungeonMapCode);
    regionMap_.emplace(region.regionCode_, region);
}


void MockRegionTable::fillPortalToRegion()
{
    const gideon::datatable::RegionTemplate region(portalEntryRegionCode,
        otherWorldMapCode, portalExitRegionCode);
    regionMap_.emplace(region.regionCode_, region);
}

}} // namespace gideon { namespace servertest {
