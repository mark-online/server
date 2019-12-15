#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/NpcInfo.h>
#include <gideon/cs/shared/data/HarvestInfo.h>
#include <gideon/cs/shared/data/ObeliskInfo.h>
#include <gideon/cs/shared/data/TreasureInfo.h>
#include <gideon/cs/shared/data/DeviceInfo.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/EntityPathInfo.h>
#include <gideon/cs/shared/data/AnchorInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/base/concurrent/Future.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
class Npc;
class Harvest;
class Obelisk;
class Treasure;
class Dungeon;
class Device;
class EntityAllocator;
} // namespace go {

class EntityPool;
class WorldMap;
class WorldMapHelper;

/**
 * @class SpawnService
 *
 * Entity spawn 서비스
 */
class ZoneServer_Export SpawnService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(SpawnService);

public:
    SpawnService(go::EntityAllocator& npcAllocator,
        go::EntityAllocator& harvestAllocator,
        go::EntityAllocator& obeliskAllocator,
        go::EntityAllocator& treasureAllocator,
        go::EntityAllocator& randomDungeonAllocator,
        go::EntityAllocator& deviceAllocator,
        go::EntityAllocator& anchorAllocator);
    ~SpawnService();

    bool initialize();

public:
    ErrorCode createEntity(go::Entity*& entity, DataCode entityCode,
        WorldMapHelper& worldMapHelper);
    ErrorCode createEntity(go::Entity*& entity, DataCode entityCode,
        const ObjectPosition& position, go::Entity* summoner);

    void destroyEntity(DataCode entityCode, go::Entity& entity);
	void destroyEntity(ObjectType ot, go::Entity& entity);

private:
    ErrorCode createNpc(go::Entity*& entity, NpcCode npcCode);

    ErrorCode createHarvest(go::Entity*& entity, HarvestCode harvestCode);

    ErrorCode createObelisk(go::Entity*& entity, ObeliskCode obeliskCode);

    ErrorCode createTreasure(go::Entity*& entity, TreasureCode treasureCode);

    ErrorCode createDevice(go::Entity*& entity, DeviceCode deviceCode);

    ErrorCode createDungeon(go::Entity*& entity, MapCode dungeonMapCode,
        WorldMapHelper& worldMapHelper);

    ErrorCode createAnchor(go::Entity*& entity, AnchorCode anchorCode,
        const ObjectPosition& position, go::Entity* owner = nullptr);

public:
    sne::base::Future::Ref scheduleSpawn(go::Entity& entity, const ObjectPosition& spawnPosition);
    sne::base::Future::Ref scheduleRespawn(go::Entity& entity, WorldMap& worldMap);
    sne::base::Future::Ref scheduleDespawn(go::Entity& entity, msec_t delay);

private:
    go::Npc* acquireNpc(NpcType npcType);
    void releaseNpc(NpcType npcType, go::Entity& entity);
    ObjectId generateNextObjectId(NpcType npcType);

    go::Dungeon* acquireDungeon(MapType mapType);
    void releaseDungeon(MapType mapType, go::Entity& entity);
    ObjectId generateNextObjectId(MapType mapType);

    ErrorCode initializeDungeon(go::Dungeon& dungeon,
        MapCode dungeonMapCode, WorldMapHelper& worldMapHelper);

private:
    std::unique_ptr<EntityPool> npcPool_;
    std::unique_ptr<EntityPool> harvestPool_;
    std::unique_ptr<EntityPool> obeliskPool_;
    std::unique_ptr<EntityPool> treasurePool_;
    std::unique_ptr<EntityPool> randomDungeonPool_;
    std::unique_ptr<EntityPool> devicePool_;
    std::unique_ptr<EntityPool> anchorPool_;
};

}} // namespace gideon { namespace zoneserver {

#define SPAWN_SERVICE gideon::zoneserver::SpawnService::instance()
