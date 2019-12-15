#include "ZoneServerPCH.h"
#include "WorldMap.h"
#include "WorldMapHelper.h"
#include "WorldMapCallback.h"
#include "Sector.h"
#include "region/WorldMapSpawnMapRegion.h"
#include "region/NormalMapRegion.h"
#include "region/RegionTriggerManager.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/status/CreatureStatus.h"
#include "../model/gameobject/ability/Liveable.h"
#include "../model/gameobject/EntityEvent.h"
#include "../controller/EntityController.h"
#include "../service/time/GameTimer.h"
#include "../service/spawn/SpawnService.h"
#include <gideon/serverbase/datatable/DataTableLoader.h>
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/cs/datatable/EntityPathTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class DestroyMapRegionTask
 * (Spawn)MapRegion을 바로 finalize()할 경우 스폰된 entity들의 task가
 * 나중에 호출되어 잘못된 메모리 참조 오류를 일으키므로 우회하기 위해 만듦.
 */
class DestroyMapRegionTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<DestroyMapRegionTask>
{
public:
    DestroyMapRegionTask(MapRegion* memory) :
        mapRegion_(memory) {}

private:
    void run() override {
        mapRegion_->finalize();
        boost::checked_delete(mapRegion_);
    }

private:
    MapRegion* mapRegion_;
};


std::unique_ptr<MapRegion> createMapRegion(WorldMap& worldMap, WorldMapHelper& worldMapHelper,
    const datatable::RegionTemplate& regionTemplate,
    const datatable::RegionCoordinates::CoordinatesList& coordinates,
    const datatable::RegionSpawnTable::Spawns* regionSpawns,
    const datatable::PositionSpawnTable::Spawns* positionSpawns)
{
    const RegionCode regionCode = regionTemplate.regionCode_;
    const RegionType regionType = getRegionType(regionCode);
    if (gideon::isSpawnable(regionType)) {
        return std::make_unique<WorldMapSpawnMapRegion>(worldMap, regionTemplate, coordinates,
            regionSpawns, positionSpawns, worldMapHelper);
    }

    return std::make_unique<NormalMapRegion>(worldMap, regionTemplate, coordinates);
}


inline void adjustCoordinate(float32_t& p, float32_t minP, float32_t maxP)
{
    if (p < minP) {
        p = minP;
    }
    else if (p > maxP) {
        p = maxP;
    }
}

} // namespace

// = WorldMap

WorldMap::WorldMap(WorldMapHelper& helper, ObjectId mapId) :
    mapId_(mapId),
    worldMapHelper_(helper),
    callback_(nullptr),
    minX_(0.0f),
    maxX_(0.0f),
    minY_(0.0f),
    maxY_(0.0f),
    minZ_(0.0f),
    maxZ_(0.0f),
    globalMapRegion_(nullptr),
    characterSpawnRegion_(nullptr),
    minPlayerReviveTime_(
        GIDEON_PROPERTY_TABLE->getPropertyValue<GameTime>(L"player_min_revive_time"))
{
}


WorldMap::~WorldMap()
{
}


bool WorldMap::initialize(const MapInfo& mapInfo)
{
    assert(mapInfo.isValid());

    SNE_LOG_INFO("WorldMap(<%u,%u>,%u,%u) is initializing...",
        mapInfo.xSectorCount_, mapInfo.ySectorCount_,
        mapInfo.sectorLength_, sizeof(Sector));

    {
        std::unique_lock<LockType> lock(lock_);

        mapInfo_ = mapInfo;

        assert(allEntities_.empty());

        if (! initSectors()) {
            return false;
        }

        if (isGlobalWorldMap()) {
            if (! initGlobalSectors()) {
                return false;
            }
        }

        // TODO: 지형 데이터 로딩

        if (! createAllMapRegions()) {
            return false;
        }
    }

    regionTriggerManager_ = std::make_unique<RegionTriggerManager>(
        worldMapHelper_.getRegionTable(mapInfo.mapCode_),
        worldMapHelper_.getRegionCoordinates(mapInfo.mapCode_));

    return true;
}


void WorldMap::finalize()
{
    std::unique_lock<LockType> lock(lock_);

    destroyMapRegions();
    clearSectors();

    allEntities_.clear();

    mapInfo_.reset();
}


void WorldMap::finalizeForReinitialize()
{
    std::unique_lock<LockType> lock(lock_);

    for (SectorMap::value_type& value : sectors_) {
        Sector* sector = value.second;
        sector->reset();
    }

    allEntities_.clear();

    destroyMapRegionsForReinitialize();

    mapInfo_.reset();
}


void WorldMap::finalizeForSpawnRegionTickable()
{
    for (MapRegionMap::value_type& value : mapRegions_) {
        value.second->finalizeForSpawnRegionTickable();
    }
}


bool WorldMap::initSectors()
{
    SNE_ASSERT((mapInfo_.xSectorCount_ % 2) != 0 && "the size of sector must be odd");
    SNE_ASSERT((mapInfo_.ySectorCount_ % 2) != 0 && "the size of sector must be odd");

    clearSectors();
    const auto halfX = static_cast<int16_t>(mapInfo_.xSectorCount_ / 2);
    assert(halfX > 0);
    const auto halfY = static_cast<int16_t>(mapInfo_.ySectorCount_ / 2);
    assert(halfY > 0);
    //const int halfZ = static_cast<int>(mapInfo_.zSectorCount_ / 2);
    //assert(halfZ > 0);
    for (int16_t r = -halfY; r <= halfY; ++r) {
        for (int16_t c = -halfX; c <= halfX; ++c) {
            const SectorAddress address(c, r);
            if (! registerSector(address, false)) {
                return false;
            }
        }
    }

    const float xHalfSize = mapInfo_.xSectorCount_ * mapInfo_.sectorLength_ / 2.0f;
    const float yHalfSize = mapInfo_.ySectorCount_ * mapInfo_.sectorLength_ / 2.0f;
    const float zHalfSize = mapInfo_.zSectorCount_ * mapInfo_.sectorLength_ / 2.0f;

    minX_ = mapInfo_.originX_ - xHalfSize;
    maxX_ = mapInfo_.originX_ + xHalfSize;
    minY_ = mapInfo_.originY_ - yHalfSize;
    maxY_ = mapInfo_.originY_ + yHalfSize;
    minZ_ = mapInfo_.originZ_ - zHalfSize;
    maxZ_ = mapInfo_.originZ_ + zHalfSize;

    SNE_LOG_INFO("WorldMap(M%X, [%u,%u,%u])'s sectors are initialized.",
        mapInfo_.mapCode_, mapInfo_.originX_, mapInfo_.originY_, mapInfo_.originZ_);
    return true;
}


bool WorldMap::initGlobalSectors()
{
    SNE_ASSERT((mapInfo_.xSectorCount_ % 2) != 0 && "the size of sector must be odd");
    SNE_ASSERT((mapInfo_.ySectorCount_ % 2) != 0 && "the size of sector must be odd");

    clearGlobalSectors();
    const uint32_t sectorSize = 3;
    globalMapInfo_ = mapInfo_;
    globalMapInfo_.xSectorCount_ = mapInfo_.xSectorCount_ / sectorSize; 
    globalMapInfo_.ySectorCount_ = mapInfo_.ySectorCount_ / sectorSize;
    globalMapInfo_.zSectorCount_ = mapInfo_.zSectorCount_ / sectorSize;
    globalMapInfo_.sectorLength_ = mapInfo_.sectorLength_ * sectorSize;


    const auto halfX = static_cast<int16_t>(globalMapInfo_.xSectorCount_ / 2);
    assert(halfX > 0);
    const auto halfY = static_cast<int16_t>(globalMapInfo_.ySectorCount_ / 2);
    assert(halfY > 0);
    //const int16_t halfZ = static_cast<int16_t>(mapInfo_.zSectorCount_ / 2);
    //assert(halfZ > 0);
    for (int16_t r = -halfY; r <= halfY; ++r) {
        for (int16_t c = -halfX; c <= halfX; ++c) {
            const SectorAddress address(c, r);
            if (! registerSector(address, true)) {
                return false;
            }
        }
    }
    return true;
}


bool WorldMap::createAllMapRegions()
{
    if (! createGlobalMapRegion()) {
        SNE_LOG_ERROR("Failed to create global map region.");
        return false;
    }

    if (! createMapRegions()) {
        SNE_LOG_ERROR("Failed to create map regions.");
        return false;
    }

    return true;
}


bool WorldMap::createGlobalMapRegion()
{
    const datatable::PositionSpawnTable* positionSpawnTable = 
        worldMapHelper_.getPositionSpawnTable(mapInfo_.mapCode_);
    if (! positionSpawnTable) {
        // Arena
        //SNE_LOG_WARNING("Position spawn table(M%u) not found.", mapInfo_.mapCode_);
    }

    const datatable::PositionSpawnTable::Spawns* positionSpawns =
        (positionSpawnTable != nullptr) ?
            positionSpawnTable->getSpawns(globalRegionCode) : nullptr;

    const float32_t lenX = float32_t(mapInfo_.xSectorCount_ * mapInfo_.sectorLength_);
    const float32_t lenY = float32_t(mapInfo_.ySectorCount_ * mapInfo_.sectorLength_);
    const float32_t regionRadius = glm::length(Vector2(lenX, lenY));
    const datatable::RegionTemplate regionTemplate(globalRegionCode);
    const datatable::CoordinatesTemplate regionCoordinates(globalRegionCode,
        Position(float32_t(mapInfo_.originX_), float32_t(mapInfo_.originY_), float32_t(mapInfo_.originZ_)),
        regionRadius);
    datatable::RegionCoordinates::CoordinatesList coordinatesList;
    coordinatesList.push_back(regionCoordinates);

    std::unique_ptr<MapRegion> mapRegion = createMapRegion(*this, worldMapHelper_, regionTemplate,
        coordinatesList, nullptr, positionSpawns);
    if ((!mapRegion) || (!mapRegion->initialize())) {
        return false;
    }

    globalMapRegion_ = mapRegion.release();
    mapRegions_.emplace(globalRegionCode, globalMapRegion_);
    return true;
}


bool WorldMap::createMapRegions()
{
    const datatable::RegionTable* regionTable = worldMapHelper_.getRegionTable(mapInfo_.mapCode_);
    if (! regionTable) {
        SNE_LOG_WARNING("Region table(M%u) not found.", mapInfo_.mapCode_);
        return true;
    }

    const datatable::RegionCoordinates* regionCoordinates =
        worldMapHelper_.getRegionCoordinates(mapInfo_.mapCode_);
    if (! regionCoordinates) {
        SNE_LOG_ERROR("Can't load region coordinates(M%u).", mapInfo_.mapCode_);
        return false;
    }

    const datatable::RegionSpawnTable* regionSpawnTable = worldMapHelper_.getRegionSpawnTable(mapInfo_.mapCode_);
    if (! regionSpawnTable) {
        SNE_LOG_WARNING("Can't load region spawn table(M%u).", mapInfo_.mapCode_);
        //return false;
    }

    const datatable::RegionTable::RegionMap& regionMap = regionTable->getRegionMap();
    SNE_LOG_DEBUG("WorldMap(%X)::initMapRegions() - %u regsions",
        mapInfo_.mapCode_, regionMap.size());

    for (const datatable::RegionTable::RegionMap::value_type& value : regionMap) {
        const RegionCode regionCode = value.first;
        const datatable::RegionTemplate& regionTemplate = value.second;
        const RegionType regionType = getRegionType(regionCode);
        
        if (rtHelpTrigger == regionType) {
            continue;
        }
        if (rtWorldEvent == regionType) {
            continue;
        }

        const datatable::RegionCoordinates::CoordinatesList* coordinates =
            regionCoordinates->getCoordinates(regionCode);
        if (! coordinates) {
            SNE_LOG_ERROR("Can't load region coordinate(R%u).", regionCode);
            return false;
        }

        const datatable::RegionSpawnTable::Spawns* regionSpawns=
            (regionSpawnTable != nullptr) ? regionSpawnTable->getSpawns(regionCode) : nullptr;       

        std::unique_ptr<MapRegion> mapRegion = createMapRegion(*this, worldMapHelper_,
            regionTemplate, *coordinates, regionSpawns, nullptr);
        if ((! mapRegion) || (! mapRegion->initialize())) {
            SNE_LOG_ERROR("Failed to create MapRegion(R%u).", regionCode);
            return false;
        }

        MapRegion* releasedMapRegion = mapRegion.release();
        mapRegions_.emplace(regionCode, releasedMapRegion);

        if (rtCharacterSpawn == regionType) {
            characterSpawnRegion_ = releasedMapRegion;
        }

        if (rtPortal == regionType) {
            teleportMapRegions_.push_back(releasedMapRegion);
        }

        if (isNoAttackableRegion(regionType)) {
            safeMapRegions_.push_back(releasedMapRegion);
        }
    }

    return true;
}


void WorldMap::destroyMapRegions()
{
    for (MapRegionMap::value_type& value : mapRegions_) {
        MapRegion* mapRegion = value.second;
        mapRegion->finalize();
        boost::checked_delete(mapRegion);
    }
    mapRegions_.clear();
    safeMapRegions_.clear();
    teleportMapRegions_.clear();

    characterSpawnRegion_ = nullptr;
}


void WorldMap::destroyMapRegionsForReinitialize()
{
    const msec_t pendingPeriod = 60 * 1000;

    for (MapRegionMap::value_type& value : mapRegions_) {
        MapRegion* mapRegion = value.second;
        mapRegion->prefinalize();
        TASK_SCHEDULER->schedule(
            std::make_unique<DestroyMapRegionTask>(mapRegion),
            pendingPeriod);
    }
    mapRegions_.clear();

    destroyMapRegions();
}


void WorldMap::clearSectors()
{
    for (SectorMap::value_type& value : sectors_) {
        Sector* sector = value.second;
        boost::checked_delete(sector);
    }
    sectors_.clear();
}


void WorldMap::clearGlobalSectors()
{
    for (SectorMap::value_type& value : globalSectors_) {
        Sector* sector = value.second;
        boost::checked_delete(sector);
    }
    globalSectors_.clear();
}


ErrorCode WorldMap::spawn(go::Entity& entity, const ObjectPosition& position)
{
    if (isEntered(entity.getGameObjectInfo())) {
        return ecWorldMapAlreadyEntered;
    }

    if (isGlobalWorldMap()) {
        Sector* globalSector = getGlobalSector(position);
        if (globalSector) {
            if (! globalSector->enter(entity, position)) {
                // TODO: 어떤 처리를 해야 하나?
            }
        }
        else {
            // TODO: 어떤 처리를 해야 하나?
            SNE_LOG_ERROR("Entity(E%" PRIu64 ")'s spawning position global(%2.1f,%2.1f,%2.1f) is not valid.",
                entity.getObjectId(), position.x_, position.y_, position.z_);
        }	
    }


    if (! entity.isBuilding()) {
        Sector* sector = getSector(position);
        if (! sector) {
            // TODO: 어떤 처리를 해야 하나?
            SNE_LOG_ERROR("Entity(E%" PRIu64 ")'s spawning position(%2.1f,%2.1f,%2.1f) is not valid.",
                entity.getObjectId(), position.x_, position.y_, position.z_);
            return ecZoneInvalidSpawnPosition;
        }

        if (! sector->enter(entity, position)) {
            return ecWorldMapAlreadyEntered;
        }
    }
    

    addEntity(entity);
    entity.getMapRegion().entitySpawned(entity);

    entity.getController().spawned(*this);

    return ecOk;
}


ErrorCode WorldMap::despawn(go::Entity& entity)
{
    ErrorCode errorCode = ecOk;

    if (isGlobalWorldMap()) {
        Sector* globalSector = getGlobalSector(entity.getPosition());
        if (globalSector != nullptr) {
            globalSector->leave(entity);
        }
        else {
            // TODO: 어떤 처리?
            if (entity.isPlayer()) {
                SectorAddress address = entity.getGlobalSectorAddress();
                address;
            }
        }
    }

    if (! entity.isBuilding()) {
        Sector* sector = getSector(entity.getSectorAddress());
        if (sector != nullptr) {
            sector->leave(entity);
        }
        else {
            errorCode = ecWorldMapNotEntered;
        }
    }   	

    if (isEntered(entity.getGameObjectInfo())) {
        removeEntity(entity);
        entity.getMapRegion().entityDespawned(entity);
    }
    else {
        errorCode = ecWorldMapNotEntered;
    }

    entity.getController().despawned(*this);

    return errorCode;
}


const Sector* WorldMap::migrate(go::Entity& entity,
    const ObjectPosition& source, const ObjectPosition& destination)
{
    if (isGlobalWorldMap()) {
        Sector* destinSector = getGlobalSector(destination);		
        Sector* sourceSector = getGlobalSector(source);
        if ((sourceSector && destinSector) && (sourceSector != destinSector)) {
            bool result = sourceSector->isEntered(entity);
            if (result) {
                result = (! destinSector->isEntered(entity));
            }
            if (result) {
                sourceSector->migrate(entity, *destinSector, destination);
            }
        }
    }

    Sector* destinSector = getSector(destination);
    if (! destinSector) {
        return nullptr;
    }

    Sector* sourceSector = getSector(source);
    if (! sourceSector) {
        return nullptr;
    }

    if (sourceSector == destinSector) {
        return nullptr;
    }

    if (! sourceSector->isEntered(entity)) {
        return nullptr;
    }

    if (destinSector->isEntered(entity)) {
        return nullptr;
    }

    if (! sourceSector->migrate(entity, *destinSector, destination)) {
        return nullptr;
    }	
    
    return destinSector;
}


void WorldMap::broadcast(go::EntityEvent::Ref event)
{
    std::lock_guard<LockType> lock(lock_);

    for (const go::EntityMap::value_type& value : allEntities_) {
        go::Entity* entity = value.second;
        if (! entity->isValid()) {
            continue;
        }

        (void)TASK_SCHEDULER->schedule(
            std::make_unique<go::EventCallTask>(*entity, event));
    }
}


bool WorldMap::isInitialized() const
{
    std::lock_guard<LockType> lock(lock_);

    return mapInfo_.isValid();
}


UnionEntityInfos WorldMap::toUnionEntityInfos(const go::EntityMap& creatures,
    const go::Entity* exceptEntity) const
{
    UnionEntityInfos unionEntityInfos;
    unionEntityInfos.reserve(creatures.size());

    for (const go::EntityMap::value_type& value : creatures) {
        //const ObjectId entityId = value.first;
        const go::Entity* entity = value.second;
        if ((exceptEntity != nullptr) && exceptEntity->isSame(*entity)) {
            continue;
        }
        if (entity->isValid()) {
            const UnionEntityInfo& entityInfo = entity->getUnionEntityInfo();
            assert(entityInfo.isValid());
            if (entityInfo.isValid()) {
                unionEntityInfos.push_back(entityInfo);
            }            
        }        
    }

    return unionEntityInfos;
}


WorldPosition WorldMap::getCharacterSpawnPosition() const
{
    std::unique_lock<LockType> lock(lock_);

    if (! characterSpawnRegion_) {
        assert(false && "character spawn region is not defined.");
        return WorldPosition();
    }

    return WorldPosition(adjustPosition(characterSpawnRegion_->getRandomPosition()),
        mapInfo_.mapCode_);
}


ObjectPosition WorldMap::getNearestGravePosition(const Position& position, bool isCao) const
{
    const MapRegion* nearestMapRegion = getNearestGraveMapRegion(position, isCao);
    if (! nearestMapRegion) {
        return ObjectPosition();
    }
    return adjustPosition(nearestMapRegion->getRandomPosition());
}


RegionCode WorldMap::getNearestGraveRegionCode(const Position& position, bool isCao) const
{
    const MapRegion* nearestMapRegion = getNearestGraveMapRegion(position, isCao);
    if (! nearestMapRegion) {
        return invalidRegionCode;
    }
    return nearestMapRegion->getRegionCode();
}


const MapRegion* WorldMap::getNearestGraveMapRegion(const Position& position, bool isCao) const
{
    std::unique_lock<LockType> lock(lock_);

    const Vector2& source = asVector2(position);

    const MapRegion* nearestMapRegion = nullptr;
    float32_t nearestDistanceSq = (std::numeric_limits<float32_t>::max)();
    for (const MapRegionMap::value_type& value : mapRegions_) {
        const MapRegion* mapRegion = value.second;
        if (isCao) {
            if (mapRegion->getRegionType() != rtCaoGrave) {
                continue;
            }
        }
        else {
            if (mapRegion->getRegionType() != rtGrave) {
                continue;
            }
        }


        const Vector2 destin = asVector2(mapRegion->getCenterPosition());
        const float32_t distanceSq = squaredLength(destin - source);
        if (distanceSq < nearestDistanceSq) {
            nearestMapRegion = mapRegion;
            nearestDistanceSq = distanceSq;
        }
    }

    return nearestMapRegion;
}


void WorldMap::addEntity(go::Entity& entity)
{
    size_t entityCount = 0;
    {
        std::unique_lock<LockType> lock(lock_);

        allEntities_.insertEntity(entity);

        entityCount = allEntities_.size();
    }

    SNE_LOG_INFO("WorldMap(%X): entity(E%" PRIu64 ") entered. total = %u",
        mapInfo_.mapCode_, entity.getObjectId(), entityCount);

    if (callback_ != nullptr) {
        callback_->entitySpawned(mapId_, mapInfo_.mapCode_, entity);
    }
}


void WorldMap::removeEntity(go::Entity& entity)
{
    size_t entityCount = 0;
    {
        std::unique_lock<LockType> lock(lock_);

        allEntities_.erase(entity.getGameObjectInfo());

        entityCount = allEntities_.size();
    }

    SNE_LOG_INFO("WorldMap(%X): entity(E%" PRIu64 ") left. total = %u",
        mapInfo_.mapCode_, entity.getObjectId(), entityCount);

    if (callback_ != nullptr) {
        callback_->entityDespawned(entity, mapInfo_.mapCode_);
    }
}


bool WorldMap::isEntered(const GameObjectInfo& entityInfo) const
{
    std::unique_lock<LockType> lock(lock_);

    return isEntered_i(entityInfo);
}


go::Entity* WorldMap::getEntity(const GameObjectInfo& entityInfo)
{
    std::unique_lock<LockType> lock(lock_);

    return getEntity_i(entityInfo);
}


const go::Entity* WorldMap::getEntity(const GameObjectInfo& entityInfo) const 
{
    std::unique_lock<LockType> lock(lock_);

    return getEntity_i(entityInfo);
}


size_t WorldMap::getEntityCount() const
{
    std::unique_lock<LockType> lock(lock_);

    return allEntities_.size();
}


size_t WorldMap::getEntityCount(ObjectType objectType) const
{
    // TODO: 최적화
    size_t count = 0;

    std::unique_lock<LockType> lock(lock_);

    for (const go::EntityMap::value_type& value : allEntities_) {
        const go::Entity* entity = value.second;
        if (entity->getObjectType() == objectType) {
            ++count;
        }
    }
    return count;
}


ObjectPosition WorldMap::adjustPosition(const ObjectPosition& destination) const
{
    // TODO: 위치의 보정 범위가 허용 수준을 넘을 경우 로깅
    ObjectPosition newPosition = destination;
    adjustCoordinate(newPosition.x_, minX_, maxX_);
    adjustCoordinate(newPosition.y_, minY_, maxY_);
    adjustCoordinate(newPosition.z_, minZ_, maxZ_);
    newPosition.heading_ = fixHeading(newPosition.heading_);
    return newPosition;
}


ErrorCode WorldMap::selectTarget(const UnionEntityInfo*& unionEntityInfo,
    const GameObjectInfo& entityInfo)
{
    unionEntityInfo = nullptr;

    // TODO: 현재는 Creature만 지원함
    if (! entityInfo.isCreature()) {
        return ecEntityNotFound;
    }

    go::Entity* entity = getEntity(entityInfo);
    if (! entity) {
        return ecEntityNotFound;
    }

    unionEntityInfo = &entity->getUnionEntityInfo();
    return ecOk;
}


bool WorldMap::isInMap(const Position& position) const
{
    std::unique_lock<LockType> lock(lock_);

    if (position.x_ < minX_) {
        return false;
    }
    if (position.x_ > maxX_) {
        return false;
    }

    if (position.y_ < minY_) {
        return false;
    }
    if (position.y_ > maxY_) {
        return false;
    }

    if (position.z_ < minZ_) {
        return false;
    }
    if (position.z_ > maxZ_) {
        return false;
    }
    return true;
}


bool WorldMap::isInSafeRegion(const Position& position) const
{
    std::lock_guard<LockType> lock(lock_);

    for (MapRegion* mapRegion : safeMapRegions_) {
        assert(isNoAttackableRegion(mapRegion->getRegionType()));

        if (mapRegion->isContained(position)) {
            return true;
        }
    }

    return false;
}


ErrorCode WorldMap::checkRespawn(go::Entity& entity, permil_t downPer, bool skipTimeCheck) const
{
    if (! entity.isPlayer()) {
        return ecOk;
    }

    go::Liveable* liveable = entity.queryLiveable();
    if (! liveable ) {
        return ecOk;
    }

    if (! skipTimeCheck) {
        const GameTime lastDeadTime = liveable->getCreatureStatus().getLastDeadTime();
        GameTime downTime = 0;
        if (0 < downPer) {
            downTime = minPlayerReviveTime_ * downPer / 1000;
        }
        if ((GAME_TIMER->msec() - lastDeadTime) < minPlayerReviveTime_ - downTime) {
            return ecCharacterNotEnoughReviveTime;
        }
    }

    return ecOk;
}


const MapRegion* WorldMap::getTeleportMapRegion(MapCode targetMapCode,
    RegionCode targetRegionCode) const
{
    std::lock_guard<LockType> lock(lock_);

    for (const MapRegion* mapRegion : teleportMapRegions_) {
        assert(mapRegion->getRegionType() == rtPortal);
        if (mapRegion->canTeleportTo(targetMapCode, targetRegionCode)) {
            return mapRegion;
        }
    }

    return nullptr;
}


const MapRegion* WorldMap::getMapRegion(RegionType regionType) const
{
    std::lock_guard<LockType> lock(lock_);

    for (const MapRegionMap::value_type& value : mapRegions_) {
        const MapRegion* mapRegion = value.second;
        if (mapRegion->getRegionType() == regionType) {
            return mapRegion;
        }
    }

    return nullptr;
}


const gdt::entity_path_t* WorldMap::getEntityPath(
    const RegionSpawnTemplate* regionSpawnTemplate,
    const PositionSpawnTemplate* positionSpawnTemplate) const
{
    const EntityPathCode entityPathCode = (regionSpawnTemplate != nullptr) ?
        invalidEntityPathCode : positionSpawnTemplate->entityPathCode_;
    return getEntityPath(entityPathCode);
}


const gdt::entity_path_t* WorldMap::getEntityPath(EntityPathCode entityPathCode) const
{
    if (isValidEntityPathCode(entityPathCode)) {
        const datatable::EntityPathTable* entityPathTable =
            worldMapHelper_.getEntityPathTable(mapInfo_.mapCode_);
        assert(entityPathTable != nullptr);
        if (entityPathTable != nullptr) {
            return entityPathTable->getEntityPath(entityPathCode);
        }
    }
    return nullptr;
}


const gdt::entity_path_t* WorldMap::getNearestEntityPath(const go::Entity& entity) const
{
    const datatable::EntityPathTable* entityPathTable =
        worldMapHelper_.getEntityPathTable(mapInfo_.mapCode_);
    assert(entityPathTable != nullptr);
    if (! entityPathTable) {
        return nullptr;
    }

    const gdt::entity_path_t* nearestEntityPath = nullptr;
    float32_t nearestPathDistanceSq = (std::numeric_limits<float32_t>::max)();

    const ObjectPosition entityPosition = entity.getPosition();
    for (const datatable::EntityPathTable::EntityPathMap::value_type& value :
        entityPathTable->getEntityPathMap()) {
        const gdt::entity_path_t* entityPath = value.second;
        const gdt::entity_path_t::path_node_sequence& pathNodes = entityPath->path_node();
        const gdt::path_node_t& pathNode = pathNodes[0];
        const Position nodePosition(pathNode.x(), pathNode.y(), pathNode.z());
        const float32_t distanceSq = get2dDistanceSqTo(entityPosition, nodePosition);
        if (distanceSq < nearestPathDistanceSq) {
            nearestPathDistanceSq = distanceSq;
            nearestEntityPath = entityPath;
        }
    }

    return nearestEntityPath;
}


const datatable::PositionSpawnTable* WorldMap::getPositionSpawnTable() const
{
    return worldMapHelper_.getPositionSpawnTable(mapInfo_.mapCode_);
}


const Sector* WorldMap::getSector(const SectorAddress& address) const
{
    std::lock_guard<LockType> lock(lock_);

    return getSector_i(address);
}


Sector* WorldMap::getSector(const SectorAddress& address)
{
    std::lock_guard<LockType> lock(lock_);

    return getSector_i(address);
}


const Sector* WorldMap::getGlobalSector(const SectorAddress& address) const
{
    std::lock_guard<LockType> lock(lock_);

    return getGlobalSector_i(address);
}


Sector* WorldMap::getGlobalSector(const SectorAddress& address)
{
    std::lock_guard<LockType> lock(lock_);

    return getGlobalSector_i(address);
}


size_t WorldMap::getSectorCount() const
{
    std::lock_guard<LockType> lock(lock_);

    return sectors_.size();
}


bool WorldMap::registerSector(const SectorAddress& address, bool isGlobalSector)
{
    if (isGlobalSector) {
        if (getGlobalSector_i(address) != nullptr) {
            SNE_ASSERT(false && "already registered global sector");
            return false;
        }
    }
    else {
        if (getSector_i(address) != nullptr) {
            SNE_ASSERT(false && "already registered sector");
            return false;
        }
    }
    
    
    const auto sectorLength =
        float32_t(isGlobalSector ? mapInfo_.sectorLength_ * 4 : mapInfo_.sectorLength_);
    auto* sector = new Sector(address, sectorLength, isGlobalSector);
    if (isGlobalSector) {
        globalSectors_.emplace(address.getSectorId(), sector);
    }
    else {
        sectors_.emplace(address.getSectorId(), sector);
    }

    for (int i = 0; i < dirCount; ++i) {
        const auto direction = static_cast<SectorDirection>(i);
        const SectorAddress neighborAddress(address.getNeighbor(direction));
        Sector* neighbor = isGlobalSector ? getGlobalSector_i(neighborAddress) : getSector_i(neighborAddress);
        if (neighbor != nullptr) {
            sector->setNeighbor(direction, neighbor);
            neighbor->setNeighbor(getOpposite(direction), sector);
        }
    }

    return true;
}

}} // namespace gideon { namespace zoneserver {
