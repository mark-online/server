#pragma once

#include <gideon/Common.h>
#include "MockRandomDungeonTable.h"
#include <gideon/cs/datatable/RegionTable.h>

namespace gideon { namespace servertest {

const RegionCode characterSpawnRegionCode = 201392129;
const RegionCode npcSpawnRegionCode = 201523201;
const RegionCode stroeNpcSpawnRegionCode = 201523202;
const RegionCode harvestRegionCode = makeRegionCode(rtHarvest, 1);
const RegionCode harvestRegionCode2 = makeRegionCode(rtHarvest, 2);
const RegionCode treasureRegionCode = makeRegionCode(rtTreasure, 1);
const RegionCode treasureRegionCode2 = makeRegionCode(rtTreasure, 2);
const RegionCode randomDungeonRegionCode = 201588740;
const RegionCode portalEntryRegionCode = makeRegionCode(rtPortal, 1);
const RegionCode portalExitRegionCode = makeRegionCode(rtPortal, 10);

/**
* @class MockRegionTable
*/
class GIDEON_SERVER_API MockRegionTable : public gideon::datatable::RegionTable
{
public:
    MockRegionTable(MapCode /*mapCode*/) {
        fillRegions();
    }

    virtual const gideon::datatable::RegionTemplate* getRegion(RegionCode code) const;

private:
    virtual const RegionMap& getRegionMap() const {
        return regionMap_;
    }

    virtual const gideon::datatable::RegionTemplate*
        getRegionByQuestMissionCode(QuestMissionCode /*missionCode*/) const {
        return nullptr;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillRegions() {
        fillCharacterSpawnRegion();
        fillNpcSpawnRegion();
        fillStoreNpcSpawnRegion();
		fillHarvestSpawnRegion();
        fillRandomDungeonRegion();
        fillPortalToRegion();
		fillTreasureSpawnRegion();
    }

    void fillCharacterSpawnRegion();
    void fillNpcSpawnRegion();
    void fillStoreNpcSpawnRegion();
	void fillHarvestSpawnRegion();
	void fillTreasureSpawnRegion();
    void fillRandomDungeonRegion();
    void fillPortalToRegion();

private:
    RegionMap regionMap_;
};

}} // namespace gideon { namespace servertest {
