#include "ZoneServerPCH.h"
#include "WorldEventSpawnRegion.h"
#include "../WorldMap.h"
#include "../../service/spawn/SpawnService.h"
#include "../../service/time/GameTimer.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Tickable.h"
#include "../../model/gameobject/ability/WorldEventable.h"
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/WorldEventInvaderSpawnTable.h>
#include <gideon/cs/datatable/WorldEventMissionSpawnTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

WorldEventSpawnMapRegion::WorldEventSpawnMapRegion(WorldMap& worldMap,
    WorldMapHelper& worldMapHelper, const datatable::RegionTemplate& regionTemplate,
    const datatable::RegionCoordinates::CoordinatesList& coordinates) :
    MapRegion(worldMap, regionTemplate, coordinates),
    worldMapHelper_(worldMapHelper)
{
    assert(rtWorldEvent == getRegionType());
}


WorldEventSpawnMapRegion::~WorldEventSpawnMapRegion()
{
}


bool WorldEventSpawnMapRegion::initialize()
{
    if (! MapRegion::initialize()) {
        return false;
    }

    startAiTask();
    return true;
}


void WorldEventSpawnMapRegion::prefinalize()
{
    std::unique_lock<LockType> lock(lock_);

    stopAiTask();
}


void WorldEventSpawnMapRegion::finalize()
{
    stopAiTask();

    {
        std::unique_lock<LockType> lock(tickableLock_);

        newTickableEntities_.clear();
        oldTickableEntities_.clear();
    }

    {
        std::unique_lock<LockType> lock(entityTickLock_);

        tickableEntities_.clear();
    }

    {
        std::unique_lock<LockType> lock(lock_);

        for (ObjectEntityMap::value_type& value : allEntities_) {
            for (go::EntityMap::value_type& value2 : value.second) {
                const GameObjectInfo& goInfo = value2.first;
                if (goInfo.isPlayer() || goInfo.isGraveStone()) {
                    continue;
                }

                go::Entity* entity = value2.second;
                assert(isValidDataCode(entity->getEntityCode()));
                entity->finalize();
            }
        }
        allEntities_.clear();

        MapRegion::finalize();
    }
}


void WorldEventSpawnMapRegion::despawnNpcs()
{
    ObjectEntityMap allEntities;
    {
        std::unique_lock<LockType> lock(entityTickLock_);

        allEntities = allEntities_;
    }

    ObjectEntityMap::iterator pos1 = allEntities.find(otNpc);
    if (pos1 != allEntities.end()) {
        go::EntityMap& npcMap = (*pos1).second;
        for (const go::EntityMap::value_type& value : npcMap) {
            go::Entity* entity = value.second;
            const ErrorCode ec = entity->despawn();
            if (isFailed(ec)) {
                assert(false);
            }
        }
    }

    ObjectEntityMap::iterator pos2 = allEntities.find(otMonster);
    if (pos2 != allEntities.end()) {
        go::EntityMap& monsterMap = (*pos2).second;
        for (const go::EntityMap::value_type& value : monsterMap) {
            go::Entity* entity = value.second;
            const ErrorCode ec = entity->despawn();
            if (isFailed(ec)) {
                assert(false);
            }
        }
    }
}


void WorldEventSpawnMapRegion::spawnEntities(WorldEventEntityInfoMap& worldEventEntityInfoMap,
    WorldEventMissionCallback& worldEventMissionCallback,
    const datatable::WorldEventMissionSpawnTable::Spawns& spawns)
{
    for (size_t i = 0; i < spawns.size(); ++i) {
        const bool shouldNotifyMovement = spawns[i]->should_notify_movement() != 0;
        const EntityPathCode entityPathCode = spawns[i]->entity_path_code();
        const float32_t joinDistance = spawns[i]->join_react_distance() / 100.0f;
        const int pool = spawns[i]->pool();

        if (pool == 1) { // 하나짜리는 보스급이라 가정하고 영역 중앙에서 스폰
            PositionSpawnTemplate positionSpawnTemplate;
            positionSpawnTemplate.entityCode_ = spawns[i]->entity_code();
            positionSpawnTemplate.position_ = ObjectPosition(getCenterPosition(), esut::random() % maxHeading);
            positionSpawnTemplate.entityPathCode_ = entityPathCode;
            spawnEntity(worldEventEntityInfoMap,  worldEventMissionCallback, 
                positionSpawnTemplate, shouldNotifyMovement, joinDistance);
        }
        else {
            RegionSpawnTemplate regionSpawnTemplate;
            regionSpawnTemplate.entityCode_ = spawns[i]->entity_code();
            regionSpawnTemplate.pool_ = uint16_t(pool);

            for (int j = 0; j < pool; ++j) {
                spawnEntity(worldEventEntityInfoMap,  worldEventMissionCallback, 
                    regionSpawnTemplate, entityPathCode, shouldNotifyMovement, joinDistance);
            }
        }
    }
}


void WorldEventSpawnMapRegion::spawnInvaders(WorldEventMissionCallback& worldEventMissionCallback,
    const gdt::world_event_invader_spawn_t& spawnTemplate)
{
    PositionSpawnTemplate positionSpawnTemplate;
    positionSpawnTemplate.entityCode_ = spawnTemplate.entity_code();
    positionSpawnTemplate.position_ = ObjectPosition(getCenterPosition(), esut::random() % maxHeading);
    positionSpawnTemplate.entityPathCode_ = spawnTemplate.entity_path_code();

    const bool shouldNotifyMovement = spawnTemplate.should_notify_movement();
    for (uint32_t i = 0; i < spawnTemplate.pool(); ++i) {
        spawnInvader(worldEventMissionCallback, positionSpawnTemplate, shouldNotifyMovement);
    }
}


void WorldEventSpawnMapRegion::startAiTask()
{
    lastUpdateTime_ = GAME_TIMER->msec();

    const msec_t thinkInterval = 100; //1 * 1000;
    aiTask_ = TASK_SCHEDULER->schedule(*this, 0, thinkInterval);
}


void WorldEventSpawnMapRegion::stopAiTask()
{
    sne::base::Future::Ref aiTask = aiTask_.lock();
    if (aiTask.get() != nullptr) {
        aiTask->cancel();
        aiTask_.reset();
    }
}


ErrorCode WorldEventSpawnMapRegion::spawnEntity(WorldEventEntityInfoMap& worldEventEntityInfoMap,
    WorldEventMissionCallback& worldEventMissionCallback, const RegionSpawnTemplate& regionSpawnTemplate,
    EntityPathCode entityPathCode, bool shouldNotifyMovement, float32_t joinDistance)
{
    go::Entity* entity = nullptr;
    ErrorCode errorCode = SPAWN_SERVICE->createEntity(entity, regionSpawnTemplate.entityCode_, worldMapHelper_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    errorCode = spawn(*entity, regionSpawnTemplate, entityPathCode);
    if (isSucceeded(errorCode)) {
        worldEventEntityInfoMap.insert(
            WorldEventEntityInfoMap::value_type(entity->getGameObjectInfo(), regionSpawnTemplate.entityCode_));
        go::WorldEventable* worldEventable = entity->queryWorldEventable();
        if (worldEventable) {
            worldEventable->setWorldEventMissionCallback(worldEventMissionCallback, shouldNotifyMovement, joinDistance);
        }
    }

    return errorCode;
}


ErrorCode WorldEventSpawnMapRegion::spawnEntity(WorldEventEntityInfoMap& worldEventEntityInfoMap,
    WorldEventMissionCallback& worldEventMissionCallback,
    const PositionSpawnTemplate& positionSpawnTemplate, bool shouldNotifyMovement, float32_t joinDistance)
{
    go::Entity* entity = nullptr;
    ErrorCode errorCode = SPAWN_SERVICE->createEntity(entity, positionSpawnTemplate.entityCode_, worldMapHelper_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    errorCode = spawn(*entity, positionSpawnTemplate);
    if (isSucceeded(errorCode)) {
        worldEventEntityInfoMap.insert(
            WorldEventEntityInfoMap::value_type(entity->getGameObjectInfo(), positionSpawnTemplate.entityCode_));
        go::WorldEventable* worldEventable = entity->queryWorldEventable();
        if (worldEventable) {
            worldEventable->setWorldEventMissionCallback(worldEventMissionCallback, shouldNotifyMovement, joinDistance);
        }
    }

    return errorCode;
}


ErrorCode WorldEventSpawnMapRegion::spawnInvader(WorldEventMissionCallback& worldEventMissionCallback,
    const PositionSpawnTemplate& positionSpawnTemplate, bool shouldNotifyMovement)
{
    go::Entity* entity = nullptr;
    ErrorCode errorCode =
        SPAWN_SERVICE->createEntity(entity, positionSpawnTemplate.entityCode_, worldMapHelper_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    errorCode = spawn(*entity, positionSpawnTemplate);
    if (isSucceeded(errorCode)) {
        go::WorldEventable* worldEventable = entity->queryWorldEventable();
        if (worldEventable) {
            worldEventable->setWorldEventMissionCallback(worldEventMissionCallback, shouldNotifyMovement, 0.0f);
        }
    }
    return errorCode;
}


ErrorCode WorldEventSpawnMapRegion::spawn(go::Entity& entity,
    const RegionSpawnTemplate& regionSpawnTemplate, EntityPathCode entityPathCode)
{    
    entity.setWorldMap(getWorldMap());
    entity.setMapRegion(*this);
    entity.setPath(getWorldMap().getEntityPath(entityPathCode));
    entity.setSpawnTemplate(regionSpawnTemplate);

    const ObjectPosition spawnPosition = getRandomPositionBy(entity, 0);
    (void)SPAWN_SERVICE->scheduleSpawn(entity, spawnPosition);
    return ecOk;
}


ErrorCode WorldEventSpawnMapRegion::spawn(go::Entity& entity,
    const PositionSpawnTemplate& positionSpawnTemplate)
{    
    entity.setWorldMap(getWorldMap());
    entity.setMapRegion(*this);
    entity.setPath(getWorldMap().getEntityPath(positionSpawnTemplate.entityPathCode_));
    entity.setSpawnTemplate(positionSpawnTemplate);

    (void)SPAWN_SERVICE->scheduleSpawn(entity, positionSpawnTemplate.position_);
    return ecOk;
}

// = MapRegion overriding

void WorldEventSpawnMapRegion::entitySpawned(go::Entity& entity)
{
    assert(entity.isValid());

    {
        std::unique_lock<LockType> lock(lock_);

        allEntities_[entity.getObjectType()].insertEntity(entity);
    }

    if (entity.queryTickable() != nullptr) {
        std::unique_lock<LockType> lock(tickableLock_);

        newTickableEntities_.insertEntity(entity);
        oldTickableEntities_.erase(entity.getGameObjectInfo());
    }
}


void WorldEventSpawnMapRegion::entityDespawned(go::Entity& entity)
{
    {
        std::unique_lock<LockType> lock(lock_);

        go::EntityMap& entityMap = allEntities_[entity.getObjectType()];
        entityMap.removeEntity(entity);
        if (entityMap.empty()) {
            allEntities_.erase(entity.getObjectType());
        }
    }

    if (entity.queryTickable() != nullptr) {
        std::unique_lock<LockType> lock(tickableLock_);

        newTickableEntities_.removeEntity(entity);
        oldTickableEntities_.insert(entity.getGameObjectInfo());
    }
}


void WorldEventSpawnMapRegion::finalizeForSpawnRegionTickable()
{
    stopAiTask();
    {
        std::unique_lock<LockType> lock(entityTickLock_);

        tickableEntities_.clear();
    }
}


go::Entity* WorldEventSpawnMapRegion::getFirstEntity(ObjectType objectType, DataCode dataCode)
{
    if (! isValid(objectType)) {
        return nullptr;
    }

    std::unique_lock<LockType> lock(lock_);

    for (go::EntityMap::value_type& value : allEntities_[objectType]) {
        go::Entity* entity = value.second;
        if (entity->getEntityCode() == dataCode) {
            return entity;
        }
    }
    return nullptr;
}


size_t WorldEventSpawnMapRegion::getEntityCount(ObjectType objectType) const
{
    std::unique_lock<LockType> lock(lock_);

    return allEntities_[objectType].size();
}


void WorldEventSpawnMapRegion::run()
{
    sne::base::Future::Ref aiTask = aiTask_.lock();
    if (! aiTask.get()) {
        return;
    }

    go::EntityMap newTickableEntities;
    GameObjectSet oldTickableEntities;
    {
        std::unique_lock<LockType> lock(tickableLock_);

        newTickableEntities = newTickableEntities_;
        newTickableEntities_.clear();
        oldTickableEntities = oldTickableEntities_;
        oldTickableEntities_.clear();
    }

    const GameTime currentTime = GAME_TIMER->msec();

    {
        std::unique_lock<LockType> lock(entityTickLock_);

        for (const GameObjectInfo& entityInfo : oldTickableEntities) {
            tickableEntities_.erase(entityInfo);
        }
        tickableEntities_.insert(newTickableEntities.begin(), newTickableEntities.end());

        const GameTime diff = getGameTimeDiff(lastUpdateTime_, currentTime);
        lastUpdateTime_ = currentTime;

        for (go::EntityMap::value_type& value : tickableEntities_) {
            go::Entity* entity = value.second;

            go::Tickable* tickable = entity->queryTickable();
            if (tickable != nullptr) {
                tickable->tick(diff);
            }
        }
    }
}

}} // namespace gideon { namespace zoneserver {
