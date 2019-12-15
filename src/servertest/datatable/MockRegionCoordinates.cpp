#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockRegionCoordinates.h>

namespace gideon { namespace servertest {

const datatable::RegionCoordinates::CoordinatesList*
MockRegionCoordinates::getCoordinates(RegionCode code) const
{
    const CoordinatesMap::const_iterator pos = coordinatesMap_.find(code);
    if (pos != coordinatesMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockRegionCoordinates::fillCharacterSpawnRegionCoordinates()
{
    coordinatesMap_[characterSpawnRegionCode].push_back(
        datatable::CoordinatesTemplate(characterSpawnRegionCode, Position(0, 0, 0), 0.5f));
}


void MockRegionCoordinates::fillNpcSpawnRegionCoordinates()
{
    coordinatesMap_[npcSpawnRegionCode].push_back(
        datatable::CoordinatesTemplate(characterSpawnRegionCode, Position(0, 0, 0), 0.5f));
}



void MockRegionCoordinates::fillStoreNpcSpawnRegionCoordinates()
{
    coordinatesMap_[stroeNpcSpawnRegionCode].push_back(
        datatable::CoordinatesTemplate(stroeNpcSpawnRegionCode, Position(0, 0, 0), 0.5f));
}



void MockRegionCoordinates::fillHarvestSpawnRegionCoordinates()
{
    coordinatesMap_[harvestRegionCode].push_back(
        datatable::CoordinatesTemplate(harvestRegionCode, Position(0, 0, 0), 0.5f));

    coordinatesMap_[harvestRegionCode2].push_back(
        datatable::CoordinatesTemplate(harvestRegionCode2, Position(0, 0, 0), 0.5f));
}


void MockRegionCoordinates::fillTreasureSpawnRegionCoordinates()
{
    coordinatesMap_[treasureRegionCode].push_back(
        datatable::CoordinatesTemplate(treasureRegionCode, Position(0, 0, 0), 0.5f));

    coordinatesMap_[treasureRegionCode2].push_back(
        datatable::CoordinatesTemplate(treasureRegionCode2, Position(0, 0, 0), 0.5f));
}


void MockRegionCoordinates::fillRandomDungeonRegionCoordinates()
{
    coordinatesMap_[randomDungeonRegionCode].push_back(
        datatable::CoordinatesTemplate(characterSpawnRegionCode, Position(0, 0, 0), 0.5f));
}


void MockRegionCoordinates::fillTeleportToRegionCoordinates()
{
    coordinatesMap_[portalEntryRegionCode].push_back(
        datatable::CoordinatesTemplate(characterSpawnRegionCode, Position(2.5f, 2.5f, 1.0f), 1.0f));
}

}} // namespace gideon { namespace servertest {
