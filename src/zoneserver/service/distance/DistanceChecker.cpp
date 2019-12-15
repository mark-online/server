#include "ZoneServerPCH.h"
#include "DistanceChecker.h"
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/cs/shared/data/Rate.h>
#include <gideon/3d/3d.h>

namespace gideon { namespace zoneserver {

SNE_DEFINE_SINGLETON(DistanceChecker)

bool DistanceChecker::initialize()
{
	if (! GIDEON_PROPERTY_TABLE) {
		return false;
	}
	
    npcDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"npc_distance"));
	harvestDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"harvest_distance"));
	treasureDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"treasure_distance"));
	deviceDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"device_distance"));
    gravestoneDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"gravestone_distance"));
    tradeDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"trade_distance"));
    dungeonDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"dungeon_distance"));
    buildingDistanceSq_ = square(GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"building_distance"));

	DistanceChecker::instance(this);
	return true;
}


bool DistanceChecker::checkDistance(float32_t distanceSq,
    const Position& src, const Position& dest) const
{
    const Vector2& source = asVector2(src);
    const Vector2& destin = asVector2(dest);
    return squaredLength(destin - source) <= distanceSq;
}

}} // namespace gideon { namespace zoneserver {