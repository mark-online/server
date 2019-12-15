#pragma once

#include "MapRegion.h"
#include <gideon/cs/datatable/RegionSpawnTable.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>
#include <sne/base/concurrent/Runnable.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver {

namespace go {
class Harvest;
class Treasure;
class Obelisk;
class Dungeon;
} // namespace go {

class WorldMapHelper;

/**
 * @class SpawnMapRegion
 * Entity spawn을 담당할 map region
 */
class ZoneServer_Export SpawnMapRegion :
    public MapRegion,
    private sne::base::Runnable
{
protected:
    typedef sne::core::HashMap<ObjectType, go::EntityMap> ObjectEntityMap;
    typedef sne::core::Set<GameObjectInfo> GameObjectSet;

    typedef std::mutex LockType;

public:
    SpawnMapRegion(WorldMap& worldMap, const datatable::RegionTemplate& regionTemplate,
        const datatable::RegionCoordinates::CoordinatesList& coordinates,
        const datatable::RegionSpawnTable::Spawns* regionSpawns,
        const datatable::PositionSpawnTable::Spawns* positionSpawns,
        WorldMapHelper& worldMapHelper);
    virtual ~SpawnMapRegion();

public:
    virtual bool initialize();
    virtual void prefinalize();
    virtual void finalize();

protected:
    virtual void entitySpawned(go::Entity& entity);
    virtual void entityDespawned(go::Entity& entity);
	virtual void finalizeForSpawnRegionTickable();

protected:
    virtual go::Entity* getFirstEntity(ObjectType objectType, DataCode dataCode);
    virtual size_t getEntityCount(ObjectType objectType) const;

public:
    ErrorCode spawnEntity(go::Entity& entity, const PositionSpawnTemplate& spawnTemplate) {
        return spawn(entity, nullptr, &spawnTemplate, 0);
    }

    /// global map region에 NPC를 소환한다
    ErrorCode summonNpc(go::Entity& summoner, NpcCode npcCode,
        msec_t spawnDelay, msec_t duration, SpawnType spawnType, float32_t keepAwayDistance = 0.0f);

    /// 특정한 위치(spawnPosition)에 NPC 소환
    ErrorCode summonNpc(go::Entity*& summon, go::Entity& summoner, const ObjectPosition& spawnPosition,
        NpcCode npcCode, msec_t spawnDelay, msec_t duration, SpawnType spawnType,
        float32_t keepAwayDistance = 0.0f);

    /// global map region에 대형 멤버(NPC)을 소환한다
    ErrorCode summonFormationMember(go::Entity*& summon, NpcCode npcCode, go::Entity& summoner,
        float32_t keepAwayDistance = 0.0f);

    /// global map region에 대형 멤버(NPC)을 소환한다
    ErrorCode spawnAnchor(go::Entity*& summonAncor, AnchorCode anchorCode, const ObjectPosition& position,
        go::Entity* summoner = nullptr);

protected:
	const datatable::PositionSpawnTable::Spawns& getPositionSpawnTemplates() const {
		return positionSpawnTemplates_;
	}

    const datatable::RegionSpawnTable::Spawns& getRegionSpawnTemplates() const {
        return regionSpawnTemplates_;
    }

protected:
    LockType& getLock() {
        return lock_;
    }

    ObjectEntityMap& getObjectEntityMap() {
        return allEntities_;
    }

    const go::EntityMap& getEntityMap(ObjectType objectType) const {
        assert(isValid(objectType));
        return allEntities_[objectType];
    }

private:
    bool spawnAllAtRegion();
    bool spawnAllAtPosition(); // 빌딩은 제외한다.

    ErrorCode spawn(DataCode entityCode, const RegionSpawnTemplate* regionSpawnTemplate,
        const PositionSpawnTemplate* positionSpawnTemplate, int regionSequence);
    ErrorCode spawn(go::Entity& entity, const RegionSpawnTemplate* regionSpawnTemplate,
        const PositionSpawnTemplate* positionSpawnTemplate, int regionSequence);

    void startAiTask();
    void stopAiTask();

private:
    // = sne::base::Runnable overriding
    virtual void run();

private:
    datatable::RegionSpawnTable::Spawns regionSpawnTemplates_;
    datatable::PositionSpawnTable::Spawns positionSpawnTemplates_;
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
