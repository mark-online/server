#pragma once

#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/utility/Singleton.h>

namespace gideon { namespace zoneserver {

/**
 * @class DistanceChecker
 */
class DistanceChecker
{
public:
    SNE_DECLARE_SINGLETON(DistanceChecker);

public:
	DistanceChecker() {}

	bool initialize();

    bool checkNpcDistance(const Position& src, const Position& dest) const {
        return checkDistance(npcDistanceSq_, src, dest);
    }
	bool checkHarvestDistance(const Position& src, const Position& dest) const {
        return checkDistance(harvestDistanceSq_, src, dest);
    }
    bool checkTreasureDistance(const Position& src, const Position& dest) const {
        return checkDistance(treasureDistanceSq_, src, dest);
    }
    bool checkDeviceDistance(const Position& src, const Position& dest) const {
        return checkDistance(deviceDistanceSq_, src, dest);
    }
    bool checkGravestoneDistance(const Position& src, const Position& dest) const {
        return checkDistance(gravestoneDistanceSq_, src, dest);
    }
    bool checkTradeDistance(const Position& src, const Position& dest) const {
        return checkDistance(tradeDistanceSq_, src, dest);
    }
    bool checkDungeonDistance(const Position& src, const Position& dest) const {
        return checkDistance(dungeonDistanceSq_, src, dest);
    }
    bool checkBuildingDistance(const Position& src, const Position& dest) const {
        return checkDistance(buildingDistanceSq_, src, dest);
    }

private:
    bool checkDistance(float32_t distanceSq, const Position& src, const Position& dest) const;

private:
    float32_t npcDistanceSq_;
	float32_t harvestDistanceSq_;
	float32_t treasureDistanceSq_;
	float32_t deviceDistanceSq_;
    float32_t gravestoneDistanceSq_;
    float32_t tradeDistanceSq_;
    float32_t dungeonDistanceSq_;
    float32_t buildingDistanceSq_;
};

}} // namespace gideon { namespace zoneserver {

#define DISTANCE_CHECKER gideon::zoneserver::DistanceChecker::instance()
