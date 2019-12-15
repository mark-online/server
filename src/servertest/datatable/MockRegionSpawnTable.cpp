#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockRegionSpawnTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const MockRegionSpawnTable::Spawns* MockRegionSpawnTable::getSpawns(RegionCode regionCode) const
{
    if (regionCode == globalRegionCode) {
        regionCode = invalidRegionCode;
    }
    const RegionSpawnMap::const_iterator pos =
        regionSpawnMap_.find(regionCode);
    if (pos != regionSpawnMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockRegionSpawnTable::fillNormalNpcSpawn()
{
    RegionSpawnTemplate spawn;
    spawn.regionCode_ = npcSpawnRegionCode;
    spawn.entityCode_ = normalNpcCode;
    spawn.pool_ = 2;
    spawn.minSpawnInterval_ = 10000;
    spawn.maxSpawnInterval_ = 10000;
    regionSpawnMap_[spawn.regionCode_].push_back(spawn);
}


void MockRegionSpawnTable::fillStoreNpcSpawn()
{
    RegionSpawnTemplate spawn;
    spawn.regionCode_ = stroeNpcSpawnRegionCode;
    spawn.entityCode_ = storeNpcCode;
    spawn.pool_ = 1;
    spawn.minSpawnInterval_ = 10000;
    spawn.maxSpawnInterval_ = 10000;
    regionSpawnMap_[spawn.regionCode_].push_back(spawn);
}


void MockRegionSpawnTable::fillRandomDungeonSpawn()
{
    RegionSpawnTemplate spawn;
    spawn.regionCode_ = randomDungeonRegionCode;
    spawn.entityCode_ = firstRandomDungeonMapCode;
    spawn.pool_ = 1;
    regionSpawnMap_[spawn.regionCode_].push_back(spawn);
}


void MockRegionSpawnTable::fillRandomDungeonNpcSpawn()
{
    RegionSpawnTemplate spawn;
    spawn.regionCode_ = invalidMapCode;
    spawn.entityCode_ = normalNpcCode;
    spawn.pool_ = 1;
    regionSpawnMap_[spawn.regionCode_].push_back(spawn);
}


void MockRegionSpawnTable::fillHarvestSpawn()
{
    RegionSpawnTemplate spawn;
    spawn.regionCode_ = harvestRegionCode;
    spawn.entityCode_ = harvestCode1;
    spawn.pool_ = 1;
    spawn.minSpawnInterval_ = 10000;
    spawn.maxSpawnInterval_ = 10000;
    regionSpawnMap_[spawn.regionCode_].push_back(spawn);

    RegionSpawnTemplate spawn2;
    spawn2.regionCode_ = harvestRegionCode2;
    spawn2.entityCode_ = harvestCode2;
    spawn2.pool_ = 1;
    spawn2.minSpawnInterval_ = 10000;
    spawn2.maxSpawnInterval_ = 10000;
    regionSpawnMap_[spawn2.regionCode_].push_back(spawn2);
}


void MockRegionSpawnTable::fillTreasureSpawn()
{
    RegionSpawnTemplate spawn;
    spawn.regionCode_ = treasureRegionCode;
    spawn.entityCode_ = treasureCode1;
    spawn.pool_ = 1;
    spawn.minSpawnInterval_ = 10000;
    spawn.maxSpawnInterval_ = 10000;
    regionSpawnMap_[spawn.regionCode_].push_back(spawn);

    RegionSpawnTemplate spawn2;
    spawn2.regionCode_ = treasureRegionCode2;
    spawn2.entityCode_ = treasureCode2;
    spawn2.pool_ = 1;
    spawn2.minSpawnInterval_ = 10000;
    spawn2.maxSpawnInterval_ = 10000;
    regionSpawnMap_[spawn2.regionCode_].push_back(spawn2);
}
}} // namespace gideon { namespace servertest {
