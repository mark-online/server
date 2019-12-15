#include "ZoneServerPCH.h"
#include "MapRegion.h"
#include "../WorldMap.h"
#include "esut/Random.h"
#include <gideon/cs/datatable/RegionTable.h>
#include "gideon/3d/3d.h"

namespace gideon { namespace zoneserver {

MapRegion::MapRegion(WorldMap& worldMap, const datatable::RegionTemplate& regionTemplate,
    const datatable::RegionCoordinates::CoordinatesList& coordinates) :
    worldMap_(worldMap),
    regionTemplate_(regionTemplate),
    coordinates_(coordinates)
{
}


MapRegion::~MapRegion()
{
}


bool MapRegion::initialize()
{
    if (coordinates_.empty()) {
        return false;
    }

    // FYI: 현재는 원만 제대로 지원!
    for (const datatable::CoordinatesTemplate& coord : coordinates_) {
        if (! coord.isCircle()) {
            return false;
        }
    }
    return true;
}


void MapRegion::prefinalize()
{
}


void MapRegion::finalize()
{
}


void MapRegion::normalizeToRegion(Position& position, const Position& homePosition) const
{
    const int regionIndex = getNearestRegionIndex(homePosition);
    assert((0 <= regionIndex) && (regionIndex < coordinates_.size()));
    const datatable::CoordinatesTemplate* coordinates = &coordinates_[regionIndex];

    if (coordinates->isCircle()) {
        Vector2 direction = asVector2(position) - asVector2(coordinates->center_);
        const float32_t distance = glm::length(direction);
        if (distance > coordinates->radius_) {
            normalize(direction, distance);
            position.x_ = coordinates->center_.x_ + (coordinates->radius_ * direction.x);
            position.y_ = coordinates->center_.y_ + (coordinates->radius_ * direction.y);
        }
    }
    else {
        assert(false);
        //clamp2d(position, getLeftTop(), getRightBottom());
    }
}


void MapRegion::normalizeToRegion(Position& position, int regionSequence) const
{
    const int regionIndex = (regionSequence % coordinates_.size());
    assert((0 <= regionIndex) && (regionIndex < coordinates_.size()));
    const datatable::CoordinatesTemplate* coordinates = &coordinates_[regionIndex];

    if (coordinates->isCircle()) {
        Vector2 direction = asVector2(position) - asVector2(coordinates->center_);
        const float32_t distance = glm::length(direction);
        if (distance > coordinates->radius_) {
            normalize(direction, distance);
            position.x_ = coordinates->center_.x_ + (coordinates->radius_ * direction.x);
            position.y_ = coordinates->center_.y_ + (coordinates->radius_ * direction.y);
        }
    }
    else {
        assert(false);
        //clamp2d(position, getLeftTop(), getRightBottom());
    }
}


bool MapRegion::isContained(const Position& position, float32_t radiusDelta) const
{
    for (const datatable::CoordinatesTemplate& coord : coordinates_) {
        if (coord.isContainedIn2d(position, radiusDelta)) {
            return true;
        }
    }
    return false;
}


ObjectPosition MapRegion::getRandomPosition() const
{
    return getRandomPosition((esut::random() % coordinates_.size()));
}


// FYI: NPC or Monster만 순차적으로 영역을 선택하도록 한다!
ObjectPosition MapRegion::getRandomPositionBy(const go::Entity& entity, int regionSequence) const
{
    if (! entity.isNpcOrMonster()) {
        return getRandomPosition();
    }

    return getRandomPosition(regionSequence % coordinates_.size());
}


int MapRegion::getNearestRegionIndex(const Position& position) const
{
    assert(! coordinates_.empty());
    if (coordinates_.size() == 1) {
        return 0;
    }

    int regionIndex = 0;
    float32_t nearestDistanceSq = (std::numeric_limits<float32_t>::max)();
    for (int i = 0; i < coordinates_.size(); ++i) {
        const datatable::CoordinatesTemplate* coord = &coordinates_.front();
        const float32_t distSq = get2dDistanceSqTo(coord->center_, position);
        if (distSq < nearestDistanceSq) {
            nearestDistanceSq = distSq;
            regionIndex = i;
        }
    }
    return regionIndex;
}


ObjectPosition MapRegion::getRandomPosition(int regionIndex) const
{
    assert((0 <= regionIndex) && (regionIndex < coordinates_.size()));

    const Heading heading = esut::random() % maxHeading;
    const datatable::CoordinatesTemplate& coord = coordinates_[regionIndex];
    if (coord.isCircle()) {
        const float32_t radius = coord.radius_ * 0.8f; // 영역 테두리에 놓이지 않도록 수정

        // 아래 코드는 중심에 몰리는 경향이 있음
        //const Vector2 direction = getDirection(esut::random() % maxHeading);
        //const float range = esut::randomExclusive() * radius;
        //const float x = coord.center_.x_ + (direction.x * range);
        //const float y = coord.center_.y_ + (direction.y * range);
        //return ObjectPosition(x, y, coord.center_.z_, heading);

        const float32_t x = esut::random(-radius, radius);
        const float32_t maxY = sqrtf((radius * radius) - (x * x));
        return ObjectPosition(x + coord.center_.x_,
            esut::random(-maxY, maxY) + coord.center_.y_,
            coord.center_.z_,
            heading);
    }
    else {
        const float x = esut::random(coord.leftTop_.x_, coord.rightBottom_.x_);
        const float y = esut::random(coord.leftTop_.y_, coord.rightBottom_.y_);
        return ObjectPosition(x, y, coord.leftTop_.z_, heading);
    }
}


}} // namespace gideon { namespace zoneserver {
