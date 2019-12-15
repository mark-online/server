#pragma once

#include <gideon/Common.h>
#include "MockRegionTable.h"
#include <gideon/cs/datatable/RegionCoordinates.h>

namespace gideon { namespace servertest {

/**
* @class MockRegionCoordinates
*/
class GIDEON_SERVER_API MockRegionCoordinates : public datatable::RegionCoordinates
{
public:
    MockRegionCoordinates(MapCode /*mapCode*/) {
        fillCoordinates();
    }

    virtual const CoordinatesList* getCoordinates(RegionCode code) const;

private:
    virtual const CoordinatesMap& getCoordinatesMap() const {
        return coordinatesMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillCoordinates() {
        fillCharacterSpawnRegionCoordinates();
        fillNpcSpawnRegionCoordinates();
		fillHarvestSpawnRegionCoordinates();
        fillRandomDungeonRegionCoordinates();
        fillTeleportToRegionCoordinates();
		fillTreasureSpawnRegionCoordinates();
        fillStoreNpcSpawnRegionCoordinates();
    }

    void fillCharacterSpawnRegionCoordinates();
    void fillNpcSpawnRegionCoordinates();
	void fillHarvestSpawnRegionCoordinates();
    void fillRandomDungeonRegionCoordinates();
    void fillTeleportToRegionCoordinates();
	void fillTreasureSpawnRegionCoordinates();
    void fillStoreNpcSpawnRegionCoordinates();

    void destroyCoordinates();

private:
    CoordinatesMap coordinatesMap_;
};

}} // namespace gideon { namespace servertest {
