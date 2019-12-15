#include "ZoneServerPCH.h"
#include "SpawnService.h"
#include "EntityPool.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/Harvest.h"
#include "../../model/gameobject/Obelisk.h"
#include "../../model/gameobject/Treasure.h"
#include "../../model/gameobject/RandomDungeon.h"
#include "../../model/gameobject/Device.h"
#include "../../model/gameobject/Anchor.h"
#include "../../model/state/CreatureState.h"
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/RandomDungeonTable.h>
#include <gideon/cs/datatable/DeviceTable.h>
#include <gideon/cs/datatable/EntityPathTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class EntitySpawnTask
 */
class EntitySpawnTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<EntitySpawnTask>
{
public:
    EntitySpawnTask(go::Entity& entity, WorldMap& worldMap, const ObjectPosition& spawnPosition) :
        entity_(entity),
        worldMap_(worldMap),
        spawnPosition_(spawnPosition) {}

private:
    virtual void run() {
        const ErrorCode errorCode = entity_.spawn(worldMap_, spawnPosition_);
        if (isFailed(errorCode)) {
            assert(false);
            SNE_LOG_ERROR("EntitySpawnTask::run() - Failed to spawn Entity(%u)[E%d]",
                entity_.getEntityCode(), errorCode);
        }
    }

private:
    go::Entity& entity_;
    WorldMap& worldMap_;
    const ObjectPosition spawnPosition_;
};

/**
 * @class EntityRespawnTask
 */
class EntityRespawnTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<EntityRespawnTask>
{
public:
    EntityRespawnTask(go::Entity& entity, WorldMap& worldMap) :
        entity_(entity),
        worldMap_(worldMap) {}

private:
    virtual void run() {
        const ErrorCode errorCode = entity_.respawn(worldMap_);
        if (isFailed(errorCode)) {
            SNE_LOG_ERROR("EntityRespawnTask::run() - Failed to respawn Entity(%u)[E%d]",
                entity_.getEntityCode(), errorCode);
        }
    }

private:
    go::Entity& entity_;
    WorldMap& worldMap_;
};


/**
 * @class EntityDespawnTask
 */
class EntityDespawnTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<EntityDespawnTask>
{
public:
    EntityDespawnTask(go::Entity& entity) :
        entity_(entity) {}

private:
    virtual void run() {
        if (! entity_.isValid()) {
            return;
        }
        const ErrorCode errorCode = entity_.despawn();
        if (isFailed(errorCode)) {
            SNE_LOG_ERROR("EntityDespawnTask::run() - Failed to despawn Entity(%u)[E%d]",
                entity_.getEntityCode(), errorCode);
        }
    }

private:
    go::Entity& entity_;
};


const size_t initialNpcSpawnCount = 20000;
const size_t initialHarvestSpawnCount = 10000;
const size_t initialObeliskSpawnCount = 10;
const size_t initialTreasureSpawnCount = 100;
const size_t initialRandomDungeonSpawnCount = 100;
const size_t initialDeviceSpawnCount = 100;
const size_t initialAnchorSpawnCount = 100;

} // namespace

// = SpawnService

SNE_DEFINE_SINGLETON(SpawnService);

SpawnService::SpawnService(go::EntityAllocator& npcAllocator,
    go::EntityAllocator& harvestAllocator,
    go::EntityAllocator& obeliskAllocator,
    go::EntityAllocator& treasureAllocator,
    go::EntityAllocator& randomDungeonAllocator,
    go::EntityAllocator& deviceAllocator,
    go::EntityAllocator& anchorAllocator) :
    npcPool_(std::make_unique<EntityPool>(npcAllocator, initialNpcSpawnCount)),
    harvestPool_(std::make_unique<EntityPool>(harvestAllocator, initialHarvestSpawnCount)),
    obeliskPool_(std::make_unique<EntityPool>(obeliskAllocator, initialObeliskSpawnCount)),
    treasurePool_(std::make_unique<EntityPool>(treasureAllocator, initialTreasureSpawnCount)),
    randomDungeonPool_(std::make_unique<EntityPool>(randomDungeonAllocator, initialRandomDungeonSpawnCount)),
    devicePool_(std::make_unique<EntityPool>(deviceAllocator, initialDeviceSpawnCount)),
    anchorPool_(std::make_unique<EntityPool>(anchorAllocator, initialAnchorSpawnCount))
{
}


SpawnService::~SpawnService()
{
    SpawnService::instance(nullptr);
}


bool SpawnService::initialize()
{
    SpawnService::instance(this);
    return true;
}


ErrorCode SpawnService::createEntity(go::Entity*& entity, DataCode entityCode,
    WorldMapHelper& worldMapHelper)
{
    entity = nullptr;

    const CodeType ct = getCodeType(entityCode);
    switch (ct) {
    case ctNpc:
        return createNpc(entity, entityCode);
    case ctObelisk:
        return createObelisk(entity, entityCode);
    case ctHarvest:
        return createHarvest(entity, entityCode);
    case ctTreasure:
        return createTreasure(entity, entityCode);
    case ctMap: //ctDungeon:
        return createDungeon(entity, entityCode, worldMapHelper);
    case ctDevice:
        return createDevice(entity, entityCode);
    }

    assert(false);
    return ecEntityNotFound;
}


ErrorCode SpawnService::createEntity(go::Entity*& entity, DataCode entityCode,
    const ObjectPosition& position, go::Entity* summoner)
{
    const CodeType ct = getCodeType(entityCode);
    if (ctAnchor == ct) {
        return createAnchor(entity, entityCode, position, summoner);
    }

    return ecEntityNotFound;
}


void SpawnService::destroyEntity(DataCode entityCode, go::Entity& entity)
{
    const CodeType ct = getCodeType(entityCode);
    switch (ct) {
    case ctNpc:
        releaseNpc(getNpcType(entityCode), entity);
        break;
    case ctObelisk:
        obeliskPool_->releaseEntity(&entity);
        break;
    case ctHarvest:
        harvestPool_->releaseEntity(&entity);
        break;
    case ctTreasure:
        treasurePool_->releaseEntity(&entity);
        break;
    case ctMap: //ctDungeon:
        releaseDungeon(getMapType(entityCode), entity);
        break;
    case ctDevice:
        devicePool_->releaseEntity(&entity);
        break;
    case otAnchor:
        anchorPool_->releaseEntity(&entity);
        break;
    default:
        assert(false);
    }
}


void SpawnService::destroyEntity(ObjectType ot, go::Entity& entity)
{
    switch (ot) {
    case otNpc:
    case otMonster:
        npcPool_->releaseEntity(&entity);
        break;
    case otObelisk:
        obeliskPool_->releaseEntity(&entity);
        break;
    case otHarvest:
        harvestPool_->releaseEntity(&entity);
        break;
    case otTreasure:
        treasurePool_->releaseEntity(&entity);
        break;
    case otDungeon:
        randomDungeonPool_->releaseEntity(&entity);
        break;
    case otDevice:
        devicePool_->releaseEntity(&entity);
        break;
    case otAnchor:
        anchorPool_->releaseEntity(&entity);
        break;;
    case otGraveStone:
    case otBuilding:
        break;
    default:
        assert(false);
    }
}



ErrorCode SpawnService::createNpc(go::Entity*& entity, NpcCode npcCode)
{
    const datatable::NpcTemplate* npcTemplate = NPC_TABLE->getNpcTemplate(npcCode);
    if (! npcTemplate) {
        return ecWorldMapInvalidNpc;
    }

    const NpcType npcType = npcTemplate->getNpcType();
    go::Npc* npc = acquireNpc(npcType);
    if (! npc) {
        return ecWorldMapInvalidNpc;
    }

    const ObjectId npcId = generateNextObjectId(npcType);
    if (! npc->initialize(npcId, *npcTemplate)) {
        static_cast<go::Entity*>(npc)->finalize();
        releaseNpc(npcType, *npc);
        return ecWorldMapInvalidNpc;
    }

    entity = npc;
    return ecOk;
}


ErrorCode SpawnService::createHarvest(go::Entity*& entity, HarvestCode harvestCode)
{
    go::Harvest* harvest = harvestPool_->acquireEntity<go::Harvest>();
    if (! harvest) {
        return ecWorldMapInvalidHarvest;
    }

    const ObjectId harvestId = harvestPool_->generateObjectId();
    if (! harvest->initialize(harvestId, harvestCode)) {
        harvestPool_->releaseEntity(harvest);
        return ecWorldMapInvalidHarvest;
    }

    entity = harvest;
    return ecOk;
}


ErrorCode SpawnService::createObelisk(go::Entity*& entity, ObeliskCode obeliskCode)
{
    obeliskCode;
    // TODO: ObeliskTemplate 추가 (비용, 기능 등을 차등두기 위해)

    go::Obelisk* obelisk = obeliskPool_->acquireEntity<go::Obelisk>();
    if (! obelisk) {
        return ecWorldMapInvalidObelisk;
    }

    const ObjectId obeliskId = obeliskPool_->generateObjectId();
    if (! obelisk->initialize(obeliskId)) {
        obeliskPool_->releaseEntity(obelisk);
        return ecWorldMapInvalidObelisk;
    }

    entity = obelisk;
    return ecOk;
}


ErrorCode SpawnService::createTreasure(go::Entity*& entity, TreasureCode treasureCode)
{
    go::Treasure* treasure = treasurePool_->acquireEntity<go::Treasure>();
    if (! treasure) {
        return ecWorldMapInvalidTreasure;
    }

    const ObjectId treasureId = treasurePool_->generateObjectId();
    if (! treasure->initialize(treasureId, treasureCode)) {
        treasurePool_->releaseEntity(treasure);
        return ecWorldMapInvalidTreasure;
    }

    entity = treasure;
    return ecOk;
}


ErrorCode SpawnService::createDevice(go::Entity*& entity, DeviceCode deviceCode)
{
    const datatable::DeviceTemplate* deviceTemplate = 
        DEVICE_TABLE->getDeviceTemplate(deviceCode);
    if (! deviceTemplate) {
        return ecWorldMapInvalidDevice;
    }

    go::Device* object = devicePool_->acquireEntity<go::Device>();
    if (! object) {
        return ecWorldMapInvalidDevice;
    }
    const ObjectId deviceId = devicePool_->generateObjectId();
    if (! object->initialize(deviceId, *deviceTemplate)) {
        devicePool_->releaseEntity(object);
        return ecWorldMapInvalidDevice;
    }


    entity = object;
    return ecOk;
}


ErrorCode SpawnService::createDungeon(go::Entity*& entity, MapCode dungeonMapCode,
    WorldMapHelper& worldMapHelper)
{
    const MapType mapType = getMapType(dungeonMapCode);

    go::Dungeon* dungeon = acquireDungeon(mapType);
    if (! dungeon) {
        return ecDungeonInvalid;
    }

    const ErrorCode errorCode = initializeDungeon(*dungeon, dungeonMapCode, worldMapHelper);
    if (isFailed(errorCode)) {
        releaseDungeon(mapType, *dungeon);
        return errorCode;
    }

    entity = dungeon;
    return ecOk;
}


ErrorCode SpawnService::createAnchor(go::Entity*& entity, AnchorCode anchorCode,
    const ObjectPosition& position, go::Entity* owner)
{
    go::Anchor* anchor = anchorPool_->acquireEntity<go::Anchor>();
    if (! anchor) {
        return ecWorldMapInvalidHarvest;
    }

    const ObjectId anchorId = anchorPool_->generateObjectId();
    if (! anchor->initialize(anchorCode, anchorId, position, owner)) {
        anchorPool_->releaseEntity(anchor);
        return ecWorldMapInvalidHarvest;
    }

    entity = anchor;
    return ecOk;
}


sne::base::Future::Ref SpawnService::scheduleSpawn(go::Entity& entity,
    const ObjectPosition& spawnPosition)
{
    WorldMap* worldMap = entity.getCurrentWorldMap();
    assert(worldMap != nullptr);
    const msec_t delay = entity.getSpawnTemplate().getSpawnDelay();
    // delay가 0일 경우에도 데드락을 피하기 위해 우회 호출하도록 함
    return TASK_SCHEDULER->schedule(
        std::make_unique<EntitySpawnTask>(entity, *worldMap, spawnPosition),
        delay);
}


sne::base::Future::Ref SpawnService::scheduleRespawn(go::Entity& entity,
    WorldMap& worldMap)
{
    const msec_t spawnDelay = entity.getSpawnTemplate().getSpawnInterval();
    if (spawnDelay <= 0) {
        return sne::base::Future::Ref();
    }

    return TASK_SCHEDULER->schedule(
        std::make_unique<EntityRespawnTask>(entity, worldMap),
        spawnDelay);
}


sne::base::Future::Ref SpawnService::scheduleDespawn(go::Entity& entity, msec_t delay)
{
    return TASK_SCHEDULER->schedule(std::make_unique<EntityDespawnTask>(entity), delay);
}


go::Npc* SpawnService::acquireNpc(NpcType npcType)
{
    if (isValid(npcType)) {
        return npcPool_->acquireEntity<go::Npc>();
    }
    else {
        assert(false && "invalid NpcType");
        return nullptr;
    }
}


void SpawnService::releaseNpc(NpcType npcType, go::Entity& entity)
{
    if (isValid(npcType)) {
        npcPool_->releaseEntity(&entity);
    }
    else {
        assert(false && "invalid NpcType");
    }
}


ObjectId SpawnService::generateNextObjectId(NpcType npcType)
{
    if (isValid(npcType)) {
        return npcPool_->generateObjectId();
    }
    else {
        assert(false && "invalid NpcType");
        return invalidObjectId;
    }
}


go::Dungeon* SpawnService::acquireDungeon(MapType mapType)
{
    switch (mapType) {
    case mtRandomDundeon:
        return randomDungeonPool_->acquireEntity<go::Dungeon>();
    }
    assert(false && "invalid MapType");
    return nullptr;
}


void SpawnService::releaseDungeon(MapType mapType, go::Entity& entity)
{
    switch (mapType) {
    case mtRandomDundeon:
        randomDungeonPool_->releaseEntity(&entity);
        break;
    default:
        assert(false && "invalid MapType");
    }
}


ObjectId SpawnService::generateNextObjectId(MapType mapType)
{
    switch (mapType) {
    case mtRandomDundeon:
        return randomDungeonPool_->generateObjectId();
    }
    assert(false && "invalid MapType");
    return invalidObjectId;
}


ErrorCode SpawnService::initializeDungeon(go::Dungeon& dungeon,
    MapCode dungeonMapCode, WorldMapHelper& worldMapHelper)
{
    assert(isValidMapCode(dungeonMapCode));

    const MapType mapType = getMapType(dungeonMapCode);
    if (mapType == mtRandomDundeon) {
        go::RandomDungeon& randomDungeon = static_cast<go::RandomDungeon&>(dungeon);
        const gdt::random_dungeon_t* dungeonTemplate = 
           RANDOM_DUNGEON_TABLE->getRandomDungeonTemplate(dungeonMapCode)->getInfo();
        if (! dungeonTemplate) {
            assert(false);
            return ecDungeonInvalid;
        }

        const ObjectId dungeonId = generateNextObjectId(mapType);
        if (! randomDungeon.initialize(dungeonId, dungeonMapCode, *dungeonTemplate,
            worldMapHelper)) {
            return ecDungeonInvalid;
        }

        return ecOk;
    }
    assert(false && "invalid MapType");
    return ecDungeonInvalid;
}

}} // namespace gideon { namespace zoneserver {
