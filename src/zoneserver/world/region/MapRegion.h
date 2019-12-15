#pragma once

#include "../../zoneserver_export.h"
#include "../../model/gameobject/Entity.h"
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

class WorldMap;

namespace go {
class Entity;
} // namespace go {

/**
 * @class MapRegion
 * 맵 상의 특정 영역
 */
class ZoneServer_Export MapRegion : public boost::noncopyable
{
protected: // abstract class
    MapRegion(WorldMap& worldMap, const datatable::RegionTemplate& regionTemplate,
        const datatable::RegionCoordinates::CoordinatesList& coordinates);

public:
    virtual ~MapRegion();

public:
    virtual bool initialize();
    virtual void prefinalize();
    virtual void finalize();

public:
    virtual void entitySpawned(go::Entity& entity) = 0;
    virtual void entityDespawned(go::Entity& entity) = 0;

	/// 투기장을 위한 임시 코드
	/// TODO: 나중에 처리해야함
	virtual void finalizeForSpawnRegionTickable() = 0;

public:
    virtual go::Entity* getFirstEntity(ObjectType objectType, DataCode dataCode) = 0;
    virtual size_t getEntityCount(ObjectType objectType) const = 0;

public:
    /// homePosition이 포함된 지역에서 position을 보정한다
    void normalizeToRegion(Position& position, const Position& homePosition) const;

    void normalizeToRegion(Position& position, int regionSequence) const;

public:
    WorldMap& getWorldMap() const {
        return worldMap_;
    }

    RegionCode getRegionCode() const {
        return regionTemplate_.regionCode_;
    }

    RegionType getRegionType() const {
        return gideon::getRegionType(getRegionCode());
    }

    const datatable::RegionTemplate& getRegionTemplate() const {
        return regionTemplate_;
    }

public:
    ObjectPosition getRandomPosition() const;
    ObjectPosition getRandomPositionBy(const go::Entity& entity, int regionSequence) const;

    const Position& getCenterPosition() const {
        // FYI: 첫번째 영역의 중심점을 리턴
        return coordinates_[0].center_;
    }

    bool isContained(const Position& position, float32_t radiusDelta = 0.0f) const;

    bool canTeleportTo(MapCode targetMapCode, RegionCode targetRegionCode) const {
        if (! regionTemplate_.isPortalRegion()) {
            return false;
        }
        return (regionTemplate_.getTargetMapCode() == targetMapCode) &&
            (regionTemplate_.getTargetRegionCode() == targetRegionCode);
    }

public:
    //Position getLeftTop() const {
    //    if (isRect_) {
    //        return leftTop_;
    //    }
    //    return Position(center_.x_ - radius_, center_.y_ - radius_, center_.z_);
    //}

    //Position getRightBottom() const {
    //    if (isRect_) {
    //        return rightBottom_;
    //    }
    //    return Position(center_.x_ + radius_, center_.y_ + radius_, center_.z_);
    //}

private:
    int getNearestRegionIndex(const Position& position) const;
    ObjectPosition getRandomPosition(int regionIndex) const;

private:
    WorldMap& worldMap_;
    const datatable::RegionTemplate regionTemplate_;
    const datatable::RegionCoordinates::CoordinatesList coordinates_;
};

}} // namespace gideon { namespace zoneserver {
