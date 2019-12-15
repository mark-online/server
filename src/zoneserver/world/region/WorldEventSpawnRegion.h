#pragma once

#include "MapRegion.h"
#include "../../service/world_event/callback/WorldEventMissionCallback.h"
#include <gideon/cs/datatable/WorldEventInvaderSpawnTable.h>
#include <gideon/cs/datatable/WorldEventMissionSpawnTable.h>
#include <sne/base/concurrent/Runnable.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver {

class WorldEvent;
class WorldMapHelper;

/**
 * @class WorldEventSpawnMapRegion
 * Public quest의 Entity spawn을 담당할 map region
 */
class ZoneServer_Export WorldEventSpawnMapRegion :
    public MapRegion,
    private sne::base::Runnable
{
    typedef sne::core::HashMap<ObjectType, go::EntityMap> ObjectEntityMap;
    typedef sne::core::Set<GameObjectInfo> GameObjectSet;

    typedef std::mutex LockType;
public:
    WorldEventSpawnMapRegion(WorldMap& worldMap, WorldMapHelper& worldMapHelper,
        const datatable::RegionTemplate& regionTemplate,
        const datatable::RegionCoordinates::CoordinatesList& coordinates);
    virtual ~WorldEventSpawnMapRegion();

    virtual bool initialize();
    virtual void prefinalize();
    virtual void finalize();

    void spawnEntities(WorldEventEntityInfoMap& worldEventEntityInfoMap,
        WorldEventMissionCallback& worldEventMissionCallback,
        const datatable::WorldEventMissionSpawnTable::Spawns& spawns);
    void spawnInvaders(WorldEventMissionCallback& worldEventMissionCallback,
        const gdt::world_event_invader_spawn_t& spawns);

	void despawnNpcs();
    void startAiTask();
    void stopAiTask();

private:
    ErrorCode spawnEntity(WorldEventEntityInfoMap& worldEventEntityInfoMap,
        WorldEventMissionCallback& worldEventMissionCallback,
        const RegionSpawnTemplate& regionSpawnTemplate, EntityPathCode entityPathCode,
        bool shouldNotifyMovement, float32_t joinDistance);
    ErrorCode spawnEntity(WorldEventEntityInfoMap& worldEventEntityInfoMap,
        WorldEventMissionCallback& worldEventMissionCallback,
        const PositionSpawnTemplate& positionSpawnTemplate, bool shouldNotifyMovement, float32_t joinDistance);
    ErrorCode spawnInvader(WorldEventMissionCallback& worldEventMissionCallback,
        const PositionSpawnTemplate& positionSpawnTemplate,
        bool shouldNotifyMovement);

    ErrorCode spawn(go::Entity& entity, const RegionSpawnTemplate& regionSpawnTemplate,
        EntityPathCode entityPathCode);
    ErrorCode spawn(go::Entity& entity, const PositionSpawnTemplate& positionSpawnTemplate);

private:
    // = MapRegion overriding
    virtual void entitySpawned(go::Entity& entity);
    virtual void entityDespawned(go::Entity& entity);
    virtual void finalizeForSpawnRegionTickable();

    virtual go::Entity* getFirstEntity(ObjectType objectType, DataCode dataCode);
    virtual size_t getEntityCount(ObjectType objectType) const;

private:
    // = sne::base::Runnable overriding
    virtual void run();

private:
    WorldMapHelper& worldMapHelper_;

    mutable ObjectEntityMap allEntities_;
    go::EntityMap tickableEntities_;
    go::EntityMap newTickableEntities_;
    GameObjectSet oldTickableEntities_;

    sne::base::Future::WeakRef aiTask_;

    GameTime lastUpdateTime_;

    mutable LockType lock_;
    mutable LockType tickableLock_;
    mutable LockType entityTickLock_;
};

}} // namespace gideon { namespace zoneserver {
