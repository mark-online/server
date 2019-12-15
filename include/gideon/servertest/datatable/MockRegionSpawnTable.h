#pragma once

#include "MockRegionTable.h"
#include "MockNpcTable.h"
#include <gideon/cs/datatable/RegionSpawnTable.h>

namespace gideon { namespace servertest {

/**
* @class MockRegionSpawnTable
*/
class GIDEON_SERVER_API MockRegionSpawnTable : public gideon::datatable::RegionSpawnTable
{
    typedef sne::core::HashMap<RegionCode, Spawns> RegionSpawnMap;

public:
    MockRegionSpawnTable(MapCode /*mapCode*/) {
        fillNpcSpawns();
    }

private:
    virtual const Spawns* getSpawns(RegionCode regionCode) const;

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillNpcSpawns() {
        fillNormalNpcSpawn();
        fillRandomDungeonSpawn();
        fillRandomDungeonNpcSpawn();
		fillHarvestSpawn();
		fillTreasureSpawn();
        fillStoreNpcSpawn();
    }

    void fillNormalNpcSpawn();
    void fillStoreNpcSpawn();
    void fillRandomDungeonSpawn();
    void fillRandomDungeonNpcSpawn();
	void fillHarvestSpawn();
	void fillTreasureSpawn();

private:
    RegionSpawnMap regionSpawnMap_;
};

}} // namespace gideon { namespace servertest {
