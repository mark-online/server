#pragma once

#include "../zoneserver_export.h"
#include "WorldMapCallback.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/EntityEvent.h"
#include <gideon/serverbase/database/callback/DBQueryPlayerCallback.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/CheatInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/RegionInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/EntityStatusInfo.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace go {
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

class WorldCallback;
class WorldMap;
class WorldMapHelper;
class WorldClock;

/**
 * @class World
 * 전체 월드를 담당한다
 */
class ZoneServer_Export World :
    private WorldMapCallback,
    private serverbase::DBQueryPlayerCallback,
    public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(World);

    using DungeonWorldMaps = sne::core::HashMap<ObjectId, WorldMap*>;
    using NicknameMap = sne::core::HashMap<Nickname, go::Entity*>;
    
    using RequestQueryPlayerMap = sne::core::HashMap<ObjectId, go::Entity*>;

    using LockType = std::mutex;

public:
    World(const ZoneInfo& zoneInfo, WorldMapHelper& worldMapHelper, WorldCallback& callback);
    ~World();

    bool initialize();
    void finalize();

    serverbase::DBQueryPlayerCallback* queryDBQueryPlayerCallback() {
        return this;
    }

public:
	ErrorCode enterWorld(ObjectPosition& spawnPosition, go::Entity& player,
		RegionCode spawnRegionCode, ObjectId arenaId, MapCode preGlobalMapCode,
        ZoneId lastZoneId, const Position& position);

    void broadcast(go::EntityEvent::Ref event);

public:
    void addWorldMap(WorldMap& worldMap);
    void removeWorldMap(const WorldMap& worldMap);

public:
    void notifyOpenedWorldMaps();

public:
    ShardId getShardId() const {
        return zoneInfo_.shardId_;
    }

    ZoneId getZoneId() const {
        return zoneInfo_.zoneId_;
    }

    MapCode getWorldMapCode() const {
        return zoneInfo_.globalMapInfo_.mapCode_;
    }

    bool isFirstZone() const {
        return zoneInfo_.isFirstZone_;
    }

    const WorldMap& getGlobalWorldMap() const {
        assert(globalWorldMap_.get() != nullptr);
        return *globalWorldMap_;
    }

    WorldMap& getGlobalWorldMap() {
        assert(globalWorldMap_.get() != nullptr);
        return *globalWorldMap_;
    }

    WorldMap* getWorldMap(ObjectId mapId);

    const WorldClock& getWorldClock() const {
        return *worldCock_;
    }

public:
    go::Entity* getPlayer(ObjectId playerId);
    go::Entity* getPlayer(const Nickname& nickname);

    ZoneUserInfos getZoneUserInfos() const;

    EntityStatusInfo getEntityStatusInfo(const go::Entity* entity) const;

public:
    bool isAggressive(int sourceLevel, int targetLevel) const;

private:
    void initWorldClock();

private:
    WorldMap* getWorldMap_i(ObjectId mapId);

private:
    // = WorldMapCallback overriding
    void entitySpawned(ObjectId mapId, MapCode mapCode, go::Entity& entity) override;
    void entityDespawned(go::Entity& entity, MapCode mapCode) override;

private:
    // = DBQueryPlayerCallback overrding
    void registPlayer(go::Entity& player) override;
    void unregistPlayer(ObjectId playerId) override;
    void onQueryAchievements(ErrorCode errorCode, ObjectId playerId, const ProcessAchievementInfoMap& processInfoMap,
        const CompleteAchievementInfoMap& completeInfoMap) override;
    void onQueryCharacterTitles(ErrorCode errorCode, ObjectId playerId, const CharacterTitleCodeSet& titleCodeSet) override;

    go::Entity* getRequestQueryPlayer(ObjectId playerId);
private:
    const ZoneInfo zoneInfo_;
    WorldMapHelper& worldMapHelper_;
    WorldCallback& callback_;

    std::unique_ptr<WorldMap> globalWorldMap_;
    DungeonWorldMaps randomDungeonWorldMaps_;
    go::EntityMap playerMaps_;
    NicknameMap nicknameMap_;
    
    RequestQueryPlayerMap requestQueryPlayerMap_;

    std::unique_ptr<WorldClock> worldCock_;

    mutable LockType lock_;
    mutable LockType playerLock_;
    mutable LockType queryLock_;
};

}} // namespace gideon { namespace zoneserver {

#define WORLD gideon::zoneserver::World::instance()
