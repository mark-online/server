#pragma once

#include "../zoneserver_export.h"
#include "SectorAddress.h"
#include "../model/gameobject/Entity.h"
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/RegionInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Time.h>
#include <boost/noncopyable.hpp>

namespace gideon {
struct RegionSpawnTemplate;
struct PositionSpawnTemplate;
} // namespace gideon {

namespace gideon { namespace datatable {
class EntityPathTable;
class PositionSpawnTable;
struct RegionTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} //namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

class WorldMapHelper;
class WorldMapCallback;
class Sector;
class MapRegion;
class RegionTriggerManager;

/**
 * @class WorldMap
 * World 내의 공간
 */
class ZoneServer_Export WorldMap : public boost::noncopyable
{
    using SectorMap = sne::core::HashMap<SectorAddress::SectorId, Sector*>;

    using LockType = std::mutex;

public:
    using MapRegionMap = sne::core::HashMap<RegionCode, MapRegion*>;
    using MapRegions = sne::core::Vector<MapRegion*>;
    
public:
    WorldMap(WorldMapHelper& helper, ObjectId mapId);
    ~WorldMap();

    bool initialize(const MapInfo& mapInfo);
    void finalize();

    /// 강제로 모든 엔티티를 despawn시킨다(관련 이벤트가 발생하지 않는다). 랜덤던전 전용
    void finalizeForReinitialize();

	/// 투기장을 위한 임시 코드
	/// TODO: 나중에 처리해야함
	void finalizeForSpawnRegionTickable();

    void setCallback(WorldMapCallback& callback) {
        callback_ = &callback;
    }

    /// CAUTION! 직접 호출하지 말고 Entity::spawn()을 호출해야 한다
    ErrorCode spawn(go::Entity& entity, const ObjectPosition& position);

    /// CAUTION! 직접 호출하지 말고 Entity::despawn()을 호출해야 한다
    ErrorCode despawn(go::Entity& entity);

    /// Entity가 source에서 destination으로 sector migration을 한다
    /// @return 성공할 경우 목적지(destination) sector.
    const Sector* migrate(go::Entity& entity, const ObjectPosition& source,
        const ObjectPosition& destination);

	void broadcast(go::EntityEvent::Ref event);

public:
    bool isInitialized() const;

    UnionEntityInfos toUnionEntityInfos(const go::EntityMap& creatures,
        const go::Entity* exceptEntity) const;

    /// 플레이어의 초기 스폰 위치를 리턴한다
    WorldPosition getCharacterSpawnPosition() const;

    /// 가장 가까운 무덤(리스폰) 위치를 리턴한다
    ObjectPosition getNearestGravePosition(const Position& position, bool isCao) const;

    /// 가장 가까운 무덤(리스폰) 지역 코드를 리턴한다
    RegionCode getNearestGraveRegionCode(const Position& position, bool isCao) const;

    /// source에서 destination으로 이동할 수 있는가?
    bool canMove(const ObjectPosition& source,
        const ObjectPosition& destination) const {
        return (getSector(source) != nullptr) &&
            (getSector(destination) != nullptr);
    }

    ObjectPosition adjustPosition(const ObjectPosition& destination) const;

    ErrorCode selectTarget(const UnionEntityInfo*& unionEntityInfo,
        const GameObjectInfo& entityInfo);

    bool isInMap(const Position& position) const;

    bool isInSafeRegion(const Position& position) const;

    ErrorCode checkRespawn(go::Entity& entity, permil_t downPer, bool skipTimeCheck = false) const;

public:
    ObjectId getMapId() const {
        return mapId_;
    }

    MapCode getMapCode() const {
        return mapInfo_.mapCode_;
    }

    MapType getMapType() const {
        const MapCode mapCode = getMapCode();
        if (isValidMapCode(mapCode)) {
            return gideon::getMapType(mapCode);
        }
        return mtUnknown;
    }

    MapRegionMap& getMapRegions() {
        return mapRegions_;
    }

    const MapRegionMap& getMapRegions() const {
        return mapRegions_;
    }

    const MapRegion& getGlobalMapRegion() const {
        assert(globalMapRegion_ != nullptr);
        return *globalMapRegion_;
    }

    MapRegion& getGlobalMapRegion() {
        assert(globalMapRegion_ != nullptr);
        return *globalMapRegion_;
    }

    const MapRegion* getMapRegion(RegionCode regionCode) const {
        const MapRegionMap::const_iterator pos = mapRegions_.find(regionCode);
        if (pos == mapRegions_.end()) {
            return nullptr;
        }
        return (*pos).second;
    }

    const MapRegion* getTeleportMapRegion(MapCode targetMapCode,
        RegionCode targetRegionCode) const;

	const MapRegion* getMapRegion(RegionType regionType) const;

    const gdt::entity_path_t* getEntityPath(
        const RegionSpawnTemplate* regionSpawnTemplate,
        const PositionSpawnTemplate* positionSpawnTemplate) const;
    const gdt::entity_path_t* getEntityPath(EntityPathCode entityPathCode) const;
    const gdt::entity_path_t* getNearestEntityPath(const go::Entity& entity) const;

    const RegionTriggerManager& getRegionTriggerManager() const {
        return *regionTriggerManager_;
    }

    const datatable::PositionSpawnTable* getPositionSpawnTable() const;

public:
    bool isGlobalWorldMap() const {
        return getMapType() == mtGlobalWorld;
    }

    bool isDungeon() const {
        return gideon::isDungeon(getMapType());
    }

public:
    const Sector* getSector(const SectorAddress& address) const;
    Sector* getSector(const SectorAddress& address);

    const Sector* getSector(const ObjectPosition& position) const {
        return getSector(position.x_, position.y_);
    }

    Sector* getSector(const ObjectPosition& position) {
        return getSector(position.x_, position.y_);
    }

    const Sector* getSector(float32_t x, float32_t y) const {
        return getSector(getSectorAddress(x, y));
    }

    Sector* getSector(float32_t x, float32_t y) {
        return getSector(getSectorAddress(x, y));
    }

public:
	const Sector* getGlobalSector(const SectorAddress& address) const;
	Sector* getGlobalSector(const SectorAddress& address);

	const Sector* getGlobalSector(const ObjectPosition& position) const {
		return getGlobalSector(position.x_, position.y_);
	}

	Sector* getGlobalSector(const ObjectPosition& position) {
		return getGlobalSector(position.x_, position.y_);
	}

	const Sector* getGlobalSector(float32_t x, float32_t y) const {
		return getGlobalSector(getGlobalSectorAddress(x, y));
	}

	Sector* getGlobalSector(float32_t x, float32_t y) {
		return getGlobalSector(getGlobalSectorAddress(x, y));
	}

public:
    size_t getSectorCount() const;

public:
    bool isEntered(const GameObjectInfo& entityInfo) const;

    go::Entity* getEntity(const GameObjectInfo& entityInfo);

	const go::Entity* getEntity(const GameObjectInfo& entityInfo) const;

    size_t getEntityCount() const;

    size_t getEntityCount(ObjectType objectType) const;

public:
    uint8_t getAggressiveLevelDiff() const {
        return mapInfo_.aggressiveLevelDiff_;
    }

private:
    bool initSectors();
	bool initGlobalSectors();
    bool createAllMapRegions();
    bool createMapRegions();
    bool createGlobalMapRegion();
    void destroyMapRegions();
    void destroyMapRegionsForReinitialize();
    void clearSectors();
	void clearGlobalSectors();

    bool registerSector(const SectorAddress& address, bool isGlobalSector);

    void addEntity(go::Entity& entity);
    void removeEntity(go::Entity& entity);

private:
    const MapRegion* getNearestGraveMapRegion(const Position& position, bool isCao) const;

    Sector* getSector_i(const SectorAddress& address) {
        const SectorMap::iterator pos = sectors_.find(address.getSectorId());
        if (pos != sectors_.end()) {
            return (*pos).second;
        }
        return nullptr;
    }

    const Sector* getSector_i(const SectorAddress& address) const {
        const SectorMap::const_iterator pos = sectors_.find(address.getSectorId());
        if (pos != sectors_.end()) {
            return (*pos).second;
        }
        return nullptr;
    }

	Sector* getGlobalSector_i(const SectorAddress& address) {
		const SectorMap::iterator pos = globalSectors_.find(address.getSectorId());
		if (pos != globalSectors_.end()) {
			return (*pos).second;
		}
		return nullptr;
	}

	const Sector* getGlobalSector_i(const SectorAddress& address) const {
		const SectorMap::const_iterator pos = globalSectors_.find(address.getSectorId());
		if (pos != globalSectors_.end()) {
			return (*pos).second;
		}
		return nullptr;
	}

    go::Entity* getEntity_i(const GameObjectInfo& entityInfo) {
        const go::EntityMap::iterator pos = allEntities_.find(entityInfo);
        if (pos == allEntities_.end()) {
            return nullptr;
        }
        return (*pos).second;
    }

	const go::Entity* getEntity_i(const GameObjectInfo& entityInfo) const {
		const go::EntityMap::const_iterator pos = allEntities_.find(entityInfo);
		if (pos == allEntities_.end()) {
			return nullptr;
		}
		return (*pos).second;
	}

    bool isEntered_i(const GameObjectInfo& entityInfo) const {
        const go::EntityMap::const_iterator pos = allEntities_.find(entityInfo);
        return pos != allEntities_.end();
    }

private:
    SectorAddress getSectorAddress(float32_t x, float32_t y) const {
        const auto column =
            SectorAddress::Value((x - mapInfo_.originX_) / mapInfo_.sectorLength_);
        const auto row =
            SectorAddress::Value((y - mapInfo_.originY_) / mapInfo_.sectorLength_);
        return SectorAddress(column, row);
    }

	SectorAddress getGlobalSectorAddress(float32_t x, float32_t y) const {
		const auto column =
			SectorAddress::Value((x - globalMapInfo_.originX_) / globalMapInfo_.sectorLength_);
		const auto row =
			SectorAddress::Value((y - globalMapInfo_.originY_) / globalMapInfo_.sectorLength_);
		return SectorAddress(column, row);
	}

private:
    const GameTime minPlayerReviveTime_;
    const ObjectId mapId_;

    WorldMapHelper& worldMapHelper_;
    WorldMapCallback* callback_;

    MapInfo mapInfo_;
	MapInfo globalMapInfo_; // 임시로 

    float32_t minX_;
    float32_t maxX_;
    float32_t minY_;
    float32_t maxY_;
    float32_t minZ_;
    float32_t maxZ_;

    Position centerWorldCoordinate_;

    SectorMap sectors_;
	SectorMap globalSectors_;

    MapRegionMap mapRegions_;
    MapRegions safeMapRegions_;
    MapRegions teleportMapRegions_;
    MapRegion* globalMapRegion_;
    MapRegion* characterSpawnRegion_;
    std::unique_ptr<RegionTriggerManager> regionTriggerManager_;

    go::EntityMap allEntities_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {
