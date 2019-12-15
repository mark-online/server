#include "ZoneServerPCH.h"
#include "SpawnMapRegion.h"
#include "../../service/spawn/SpawnService.h"
#include "../../service/anchor/AnchorService.h"
#include "../../service/time/GameTimer.h"
#include "../../world/WorldMap.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/Harvest.h"
#include "../../model/gameobject/Treasure.h"
#include "../../model/gameobject/Obelisk.h"
#include "../../model/gameobject/Dungeon.h"
#include "../../model/gameobject/ability/Tickable.h"
#include "../../model/gameobject/ability/WorldEventable.h"
#include "../../model/gameobject/ability/Summonable.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/template/region_table.hxx>
#include <gideon/cs/datatable/RandomDungeonTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Logger.h>

namespace gideon {

const SpawnTemplate& getSpawnTemplate(const RegionSpawnTemplate* regionSpawnTemplate,
    const PositionSpawnTemplate* positionSpawnTemplate)
{
    assert((regionSpawnTemplate != nullptr) || (positionSpawnTemplate != nullptr));

    return (regionSpawnTemplate != nullptr) ?
        static_cast<const SpawnTemplate&>(*regionSpawnTemplate) :
        static_cast<const SpawnTemplate&>(*positionSpawnTemplate);
}

} // namespace gideon {

namespace gideon { namespace zoneserver {

// = SpawnMapRegion

SpawnMapRegion::SpawnMapRegion(WorldMap& worldMap,
    const datatable::RegionTemplate& regionTemplate,
    const datatable::RegionCoordinates::CoordinatesList& coordinates,
    const datatable::RegionSpawnTable::Spawns* regionSpawns,
    const datatable::PositionSpawnTable::Spawns* positionSpawns,
    WorldMapHelper& worldMapHelper) :
    MapRegion(worldMap, regionTemplate, coordinates),
    worldMapHelper_(worldMapHelper)
{
    assert(isSpawnable(getRegionType()));

    if (regionSpawns != nullptr) {
        regionSpawnTemplates_ = *regionSpawns;
    }

    if (positionSpawns != nullptr) {
        positionSpawnTemplates_ = *positionSpawns;
    }
}


SpawnMapRegion::~SpawnMapRegion()
{
}


bool SpawnMapRegion::initialize()
{
    std::unique_lock<LockType> lock(lock_);

    if (! MapRegion::initialize()) {
        return false;
    }

    if (SPAWN_SERVICE != nullptr) { // for Test
        if (! spawnAllAtRegion()) {
            return false;
        }
        if (! spawnAllAtPosition()) {
            return false;
        }
    }

    startAiTask();
    return true;
}


void SpawnMapRegion::prefinalize()
{
    std::unique_lock<LockType> lock(lock_);

    stopAiTask();
}


void SpawnMapRegion::finalize()
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
                const DataCode entityCode = entity->getEntityCode();				
                assert(isValidDataCode(entityCode));
                entity->finalize();
                // TODO: anchor
                //const CodeType ct = getCodeType(entityCode);
                //if (ct == ctAnchor) {
                //    ANCHOR_SERVICE->destroyAnchor(entityCode, *entity);
                //}
                //else {
                    SPAWN_SERVICE->destroyEntity(entityCode, *entity);
                //}
            }
        }
        allEntities_.clear();

        MapRegion::finalize();
    }
}


void SpawnMapRegion::entitySpawned(go::Entity& entity)
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


void SpawnMapRegion::entityDespawned(go::Entity& entity)
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


void SpawnMapRegion::finalizeForSpawnRegionTickable()
{
    stopAiTask();
    {
        std::unique_lock<LockType> lock(entityTickLock_);

        tickableEntities_.clear();
    }
}


go::Entity* SpawnMapRegion::getFirstEntity(ObjectType objectType, DataCode dataCode)
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


size_t SpawnMapRegion::getEntityCount(ObjectType objectType) const
{
    std::unique_lock<LockType> lock(lock_);

    return allEntities_[objectType].size();
}


ErrorCode SpawnMapRegion::summonNpc(go::Entity& summoner, NpcCode npcCode,
    msec_t spawnDelay, msec_t duration, SpawnType spawnType, float32_t keepAwayDistance)
{
    go::Entity* entity = nullptr;
    ErrorCode errorCode = SPAWN_SERVICE->createEntity(entity, npcCode, worldMapHelper_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    PositionSpawnTemplate spawnTemplate;
    spawnTemplate.spawnType_ = spawnType;
    spawnTemplate.minSpawnDelay_ = spawnDelay;
    spawnTemplate.maxSpawnDelay_ = spawnDelay;
    spawnTemplate.minDespawnInterval_ = duration;
    spawnTemplate.maxDespawnInterval_ = duration;
    spawnTemplate.entityCode_ = npcCode;
    spawnTemplate.position_ = summoner.getPosition();
    if (keepAwayDistance > 0.0f) {
        keepAway(spawnTemplate.position_, keepAwayDistance);
    }

    errorCode = spawn(*entity, nullptr, &spawnTemplate, 0);
    if (isFailed(errorCode)) {
        SPAWN_SERVICE->destroyEntity(npcCode, *entity);
        return errorCode;
    }

    go::Summonable* summonerSummonable = summoner.querySummonable();
    if (summonerSummonable != nullptr) {
        summonerSummonable->addSummon(entity->getGameObjectInfo());
    }
    go::Summonable* summoneeSummonable = entity->querySummonable();
    if (summoneeSummonable != nullptr) {
        summoneeSummonable->setSummoner(summoner, spawnType);
    }

    go::WorldEventable* worldEventable = summoner.queryWorldEventable();
    if (worldEventable) {
        WorldEventMissionCallback* callback = worldEventable->getWorldEventMissionCallback();
        if (callback) {
            entity->queryWorldEventable()->setWorldEventMissionCallback(*callback, false, 0.0f);
        }
    }
    return ecOk;
}


ErrorCode SpawnMapRegion::summonNpc(go::Entity*& summon, go::Entity& summoner,
    const ObjectPosition& spawnPosition, NpcCode npcCode,
    msec_t spawnDelay, msec_t duration, SpawnType spawnType, float32_t keepAwayDistance)
{
    summon = nullptr;
    go::Entity* entity = nullptr;
    ErrorCode errorCode = SPAWN_SERVICE->createEntity(entity, npcCode, worldMapHelper_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    PositionSpawnTemplate spawnTemplate;
    spawnTemplate.spawnType_ = spawnType;
    spawnTemplate.minSpawnDelay_ = spawnDelay;
    spawnTemplate.maxSpawnDelay_ = spawnDelay;
    spawnTemplate.minDespawnInterval_ = duration;
    spawnTemplate.maxDespawnInterval_ = duration;
    spawnTemplate.entityCode_ = npcCode;
    spawnTemplate.position_ = spawnPosition;
    if (keepAwayDistance > 0.0f) {
        keepAway(spawnTemplate.position_, keepAwayDistance);
    }

    errorCode = spawn(*entity, nullptr, &spawnTemplate, 0);
    if (isFailed(errorCode)) {
        SPAWN_SERVICE->destroyEntity(npcCode, *entity);
        return errorCode;
    }

    go::Summonable* summonerSummonable = summoner.querySummonable();
    if (summonerSummonable != nullptr) {
        summonerSummonable->addSummon(entity->getGameObjectInfo());
    }
    go::Summonable* summoneeSummonable = entity->querySummonable();
    if (summoneeSummonable != nullptr) {
        summoneeSummonable->setSummoner(summoner, spawnType);
    }

    summon = entity;
    return ecOk;
}


ErrorCode SpawnMapRegion::summonFormationMember(go::Entity*& summon, NpcCode npcCode, go::Entity& summoner,
    float32_t keepAwayDistance)
{
    summon = nullptr;
    go::Entity* entity = nullptr;
    ErrorCode errorCode = SPAWN_SERVICE->createEntity(entity, npcCode, worldMapHelper_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    PositionSpawnTemplate spawnTemplate;
    spawnTemplate.entityCode_ = npcCode;
    spawnTemplate.position_ = summoner.getPosition();
    if (keepAwayDistance > 0.0f) {
        keepAway(spawnTemplate.position_, keepAwayDistance);
    }

    errorCode = spawn(*entity, nullptr, &spawnTemplate, 0);
    if (isFailed(errorCode)) {
        SPAWN_SERVICE->destroyEntity(npcCode, *entity);
        return errorCode;
    }

    summon = entity;
    return ecOk;
}


ErrorCode SpawnMapRegion::spawnAnchor(go::Entity*& summonAncor, AnchorCode anchorCode, const ObjectPosition& position,
    go::Entity* summoner)
{
    go::Entity* entity = nullptr;
    ErrorCode errorCode = SPAWN_SERVICE->createEntity(entity, anchorCode, position, summoner);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    PositionSpawnTemplate spawnTemplate;
    spawnTemplate.entityCode_ = anchorCode;
    spawnTemplate.position_ = position;
    
    entity->setWorldMap(getWorldMap());
    entity->setMapRegion(*this);
    entity->setSpawnTemplate(spawnTemplate);

    errorCode = spawnEntity(*entity, spawnTemplate);
    if (isFailed(errorCode)) {
        SPAWN_SERVICE->destroyEntity(anchorCode, *entity);
        return errorCode;
    }

    summonAncor = entity;

    return ecOk;

}


bool SpawnMapRegion::spawnAllAtRegion()
{
    for (const RegionSpawnTemplate& spawnTemplate : regionSpawnTemplates_) {
        for (uint16_t i = 0; i < spawnTemplate.pool_; ++i) {
            const DataCode entityCode = spawnTemplate.entityCode_;
            const ErrorCode errorCode = spawn(entityCode, &spawnTemplate, nullptr, i);
            if (isFailed(errorCode)) {
                SNE_LOG_ERROR("Failed to spawn entity(%u) in region(%u)[E:%d].",
                    entityCode, spawnTemplate.regionCode_, errorCode);
                return false;
            }
        }
    }

    return true;
}


bool SpawnMapRegion::spawnAllAtPosition()
{
    for (const PositionSpawnTemplate& spawnTemplate :  positionSpawnTemplates_) {
        const DataCode entityCode = spawnTemplate.entityCode_;
        const ErrorCode errorCode = spawn(entityCode, nullptr, &spawnTemplate, 0);
        if (isFailed(errorCode)) {
            SNE_LOG_ERROR("Failed to spawn entity(%u) in position[E:%d].", entityCode, errorCode);
            return false;
        }
    }

    return true;
}


ErrorCode SpawnMapRegion::spawn(DataCode entityCode,
    const RegionSpawnTemplate* regionSpawnTemplate,
    const PositionSpawnTemplate* positionSpawnTemplate, int regionSequence)
{
    if ((! regionSpawnTemplate) && (! positionSpawnTemplate)) {
        assert(false);
        return ecServerInternalError;
    }

    if (isValidBuildingCode(entityCode)) {
        return ecOk;
    }

    go::Entity* entity = nullptr;
    if (isValidAnchorCode(entityCode)) {
        ObjectPosition position = regionSpawnTemplate != nullptr ? getRandomPosition() : positionSpawnTemplate->position_;
        return spawnAnchor(entity, entityCode, position);
    }

    const ErrorCode errorCode =
        SPAWN_SERVICE->createEntity(entity, entityCode, worldMapHelper_);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    return spawn(*entity, regionSpawnTemplate, positionSpawnTemplate, regionSequence);    
}


ErrorCode SpawnMapRegion::spawn(go::Entity& entity,
    const RegionSpawnTemplate* regionSpawnTemplate,
    const PositionSpawnTemplate* positionSpawnTemplate, int regionSequence)
{
    if ((! regionSpawnTemplate) && (! positionSpawnTemplate)) {
        assert(false);
        return ecServerInternalError;
    }

    entity.setWorldMap(getWorldMap());
    entity.setMapRegion(*this, regionSequence);
    entity.setPath(getWorldMap().getEntityPath(regionSpawnTemplate, positionSpawnTemplate));
    entity.setSpawnTemplate(getSpawnTemplate(regionSpawnTemplate, positionSpawnTemplate));

    const ObjectPosition spawnPosition = (positionSpawnTemplate != nullptr) ?
        positionSpawnTemplate->position_ : getRandomPositionBy(entity, regionSequence);
    (void)SPAWN_SERVICE->scheduleSpawn(entity, spawnPosition);
    return ecOk;
}


void SpawnMapRegion::startAiTask()
{
    lastUpdateTime_ = GAME_TIMER->msec();

    const msec_t thinkInterval = 100; //1 * 1000;
    aiTask_ = TASK_SCHEDULER->schedule(*this, 0, thinkInterval);
}


void SpawnMapRegion::stopAiTask()
{
    sne::base::Future::Ref aiTask = aiTask_.lock();
    if (aiTask.get() != nullptr) {
        aiTask->cancel();
        aiTask_.reset();
    }
}

// = sne::base::Runnable overriding

void SpawnMapRegion::run()
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
