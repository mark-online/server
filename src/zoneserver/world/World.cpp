#include "ZoneServerPCH.h"
#include "World.h"
#include "WorldCallback.h"
#include "WorldMap.h"
#include "WorldMapHelper.h"
#include "WorldClock.h"
#include "region/MapRegion.h"
#include "../model/gameobject/AbstractAnchor.h"
#include "../model/gameobject/Creature.h"
#include "../model/gameobject/StaticObject.h"
#include "../model/gameobject/ability/Achievementable.h"
#include "../model/gameobject/ability/CharacterTitleable.h"
#include "../model/gameobject/status/StaticObjectStatus.h"
#include "../ZoneService.h"
#include "../s2s/ZoneCommunityServerProxy.h"
#include "../service/arena/ArenaService.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/3d/3d.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver {

namespace {
/**
 * @class DatabaseCallbackResisterTask
 */
class DatabaseCallbackResisterTask : public sne::database::DatabaseTask
{
public:
    DatabaseCallbackResisterTask(serverbase::DBQueryPlayerCallback& callback) :
        callback_(callback) {}

    virtual void run(sne::database::Database& database) {
        serverbase::ProxyGameDatabase& db = 
            static_cast<serverbase::ProxyGameDatabase&>(database);
        db.registDBQueryPlayerCallback(callback_);
    }

private:
    serverbase::DBQueryPlayerCallback& callback_;
};

}

SNE_DEFINE_SINGLETON(World);

World::World(const ZoneInfo& zoneInfo, WorldMapHelper& worldMapHelper,
    WorldCallback& callback) :
    zoneInfo_(zoneInfo),
    worldMapHelper_(worldMapHelper),
    callback_(callback)
{
    World::instance(this);
}


World::~World()
{
}


bool World::initialize()
{
    sne::server::Profiler profiler(__FUNCTION__);

    DatabaseCallbackResisterTask callbackTask(*this);
    SNE_DATABASE_MANAGER->enumerate(callbackTask);

    auto worldMap = std::make_unique<WorldMap>(worldMapHelper_, worldMapHelper_.generateMapId());
    if (!worldMap->initialize(zoneInfo_.globalMapInfo_)) {
        SNE_LOG_ERROR("Failed to initialize Global World Map.");
        return false;
    }
    addWorldMap(*worldMap);

    globalWorldMap_ = std::move(worldMap);

    initWorldClock();

    return true;
}


void World::finalize()
{
    std::lock_guard<LockType> lock(lock_);

    worldCock_.reset();

    if (globalWorldMap_) {
        globalWorldMap_->finalize();
    }

    for (DungeonWorldMaps::value_type& value : randomDungeonWorldMaps_) {
        WorldMap* worldMap = value.second;
        worldMap->finalize();
    }
}


ErrorCode World::enterWorld(ObjectPosition& spawnPosition, go::Entity& player,
    RegionCode spawnRegionCode, ObjectId arenaId, MapCode preGlobalMapCode,
    ZoneId lastZoneId, const Position& position)
{
    WorldMap* spawnWorldMap = nullptr;
    if (isValidObjectId(arenaId)) {
        const ErrorCode errorCode = ARENA_SERVICE->enterArena(spawnPosition, spawnWorldMap, player, arenaId, preGlobalMapCode);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }
    else {
        spawnWorldMap = &WORLD->getGlobalWorldMap();

        if (isValidRegionCode(spawnRegionCode)) {
            const MapRegion* mapRegion = spawnWorldMap->getMapRegion(spawnRegionCode);
            if (! mapRegion) {
                return ecZoneInvalidSpawnPosition;;
            }
            if (mapRegion->getRegionType() != rtPortal) {
                return ecZoneInvalidSpawnPosition;;
            }
            spawnPosition = mapRegion->getRandomPosition();
        }
        else if (lastZoneId != WORLD->getZoneId()) {
            if (!isZero(asVector3(position))) {
                static_cast<Position&>(spawnPosition) = position;
            }
            else {
                spawnPosition = spawnWorldMap->getCharacterSpawnPosition();
            }
        }
        else {
            spawnPosition = player.getPosition();
        }
    }

    assert(spawnWorldMap != nullptr);
    player.setWorldMap(*spawnWorldMap);
    player.setMapRegion(spawnWorldMap->getGlobalMapRegion());
    return ecOk;
}


void World::broadcast(go::EntityEvent::Ref event)
{
    globalWorldMap_->broadcast(event);

    {
        std::lock_guard<LockType> lock(lock_);

        for (DungeonWorldMaps::value_type& value : randomDungeonWorldMaps_) {
            WorldMap* worldMap = value.second;
            worldMap->broadcast(event);
        }
    }
}


void World::addWorldMap(WorldMap& worldMap)
{
    const ObjectId mapId = worldMap.getMapId();
    assert(isValidObjectId(mapId));
    const MapCode mapCode = worldMap.getMapCode();
    assert(! getWorldMap(mapId));

    worldMap.setCallback(*this);

    {
        std::lock_guard<LockType> lock(lock_);

        const MapType mapType = getMapType(mapCode);
        if (mtRandomDundeon == mapType) {
            randomDungeonWorldMaps_.emplace(mapId, &worldMap);
        }
    }

    callback_.worldMapOpened(mapCode, mapId);
}


void World::removeWorldMap(const WorldMap& worldMap)
{
    const ObjectId mapId = worldMap.getMapId();
    const MapCode mapCode = worldMap.getMapCode();

    {
        std::lock_guard<LockType> lock(lock_);

        if (! worldMap.isInitialized()) {
            return;
        }

        assert(isValidObjectId(mapId));
        assert(isValidMapCode(mapCode));
        const MapType mapType = getMapType(mapCode);
        if (mtRandomDundeon == mapType) {
            randomDungeonWorldMaps_.erase(mapId);
        }
        else {
            assert(false);
        }
    }

    callback_.worldMapClosed(mapCode, mapId);
}


void World::notifyOpenedWorldMaps()
{
    assert(globalWorldMap_);
    ZoneCommunityServerProxy& communityServerProxy =
        ZONE_SERVICE->getCommunityServerProxy();
    communityServerProxy.z2m_worldMapOpened(
        globalWorldMap_->getMapCode(), globalWorldMap_->getMapId());

    std::lock_guard<LockType> lock(lock_);

    for (DungeonWorldMaps::value_type& value : randomDungeonWorldMaps_) {
        const ObjectId mapId = value.first;
        WorldMap* worldMap = value.second;
        communityServerProxy.z2m_worldMapOpened(
            worldMap->getMapCode(), mapId);
    }
}


WorldMap* World::getWorldMap(ObjectId mapId)
{
    std::lock_guard<LockType> lock(lock_);

    return getWorldMap_i(mapId);
}


go::Entity* World::getPlayer(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    go::EntityMap::iterator pos = playerMaps_.find(GameObjectInfo(otPc, playerId));
    if (pos != playerMaps_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


go::Entity* World::getPlayer(const Nickname& nickname)
{
    std::lock_guard<LockType> lock(lock_);

    NicknameMap::iterator pos = nicknameMap_.find(nickname);
    if (pos != nicknameMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


ZoneUserInfos World::getZoneUserInfos() const
{
    ZoneUserInfos infos;

    std::lock_guard<LockType> lock(lock_);

    for (const go::EntityMap::value_type& value : playerMaps_) {
        const go::Entity* player = value.second;
        infos.push_back(ZoneUserInfo(player->getObjectId(), player->getNickname()));
    }
    return infos;
}


EntityStatusInfo World::getEntityStatusInfo(const go::Entity* entity) const
{
    if (! entity) {
        return EntityStatusInfo();
    }

    EntityStatusInfo statusInfo(entity->getGameObjectInfo());
    // TODO: anchor
/*    if (entity->isAnchorOrBuilding()) {
        const go::AbstractAnchor& anchor = static_cast<const go::AbstractAnchor&>(*entity);
        statusInfo.set(anchor.getCurrentPoints(), anchor.getAnchorOwnerInfo());
    } 
    else */if (entity->isCreature()) {
        const go::Creature& creature = static_cast<const go::Creature&>(*entity);
        statusInfo.set(creature.getCreatureStatus().getHitPoints(), creature.getDebuffBuffEffectInfoSet(),
            creature.getCreatureStatus().getCreatureShields());
    }
    else if (entity->isStaticObject()) {
        const go::StaticObject& so = static_cast<const go::StaticObject&>(*entity);
        statusInfo.set(so.getStaticObjectStatus().getHitPoints());
    }
    return statusInfo;
}


bool World::isAggressive(int sourceLevel, int targetLevel) const
{
    const int aggressiveLevelDiff = globalWorldMap_->getAggressiveLevelDiff();
    if (aggressiveLevelDiff <= 0) {
        return true;
    }

    const int levelDiff = targetLevel - sourceLevel;
    if (levelDiff <= 0) {
        return true;
    }

    return levelDiff < globalWorldMap_->getAggressiveLevelDiff();
}


WorldMap* World::getWorldMap_i(ObjectId mapId)
{
    if (globalWorldMap_ && (globalWorldMap_->getMapId() == mapId)) {
        return globalWorldMap_.get();
    }

    const DungeonWorldMaps::iterator pos = randomDungeonWorldMaps_.find(mapId);
    if (pos != randomDungeonWorldMaps_.end()) {
        WorldMap* worldMap = (*pos).second;
        return worldMap;
    }

    return nullptr;
}


void World::initWorldClock()
{
    WorldTime currentWorldTime = minWorldTime;
    {
        sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

        sne::base::Future::Ref future = db->getWorldTime(zoneInfo_.zoneId_);
        if ((future.get() != nullptr) && future->waitForDone()) {
            const serverbase::GetWorldTimeRequestFuture& requestFuture =
                static_cast<const serverbase::GetWorldTimeRequestFuture&>(*future);
            if (isSucceeded(requestFuture.errorCode_)) {
                currentWorldTime = requestFuture.worldTime_;
            }
            else {
                // 오류 무시
            }
        }
        else {
            // 오류 무시
        }
    }

    worldCock_ = std::make_unique<WorldClock>(zoneInfo_.zoneId_, currentWorldTime);
}

// = WorldMapCallback overriding

void World::entitySpawned(ObjectId mapId, MapCode mapCode, go::Entity& entity)
{
    if (! entity.isPlayer()) {
        return;
    }

    go::Entity& player = static_cast<go::Entity&>(entity);
    {
        std::lock_guard<LockType> lock(lock_);

        playerMaps_.emplace(GameObjectInfo(otPc, player.getObjectId()), &player);
        nicknameMap_.emplace(player.getNickname(), &player);
    }

    if (isValidMapCode(mapCode) && hasChatChannel(getMapType(mapCode))) {
        ZONE_SERVICE->getCommunityServerProxy().z2m_worldMapEntered(mapId,
            player.getAccountId(), player.getObjectId(), player.getPosition());
    }
}


void World::entityDespawned(go::Entity& entity, MapCode mapCode)
{
    if (! entity.isPlayer()) {
        return;
    }

    go::Entity& player = static_cast<go::Entity&>(entity);
    {
        std::lock_guard<LockType> lock(lock_);

        playerMaps_.erase(GameObjectInfo(otPc, player.getObjectId()));
        nicknameMap_.erase(player.getNickname());
    }


    if (hasChatChannel(getMapType(mapCode))) {
        ZONE_SERVICE->getCommunityServerProxy().z2m_worldMapLeft(player.getAccountId(),
            player.getObjectId());
    }
}


void World::registPlayer(go::Entity& player)
{
    if (! player.isPlayer()) {
        return;
    }

    {
        std::lock_guard<LockType> lock(lock_);

        requestQueryPlayerMap_.emplace(player.getObjectId(), &player);
    }
}


void World::unregistPlayer(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    requestQueryPlayerMap_.erase(playerId);
}


void World::onQueryAchievements(ErrorCode errorCode, ObjectId playerId, const ProcessAchievementInfoMap& processInfoMap,
    const CompleteAchievementInfoMap& completeInfoMap)
{
    go::Entity* player = getRequestQueryPlayer(playerId);
    if (player) {
        player->queryAchievementable()->initializeAchievement(errorCode, processInfoMap, completeInfoMap);
    }
}


void World::onQueryCharacterTitles(ErrorCode errorCode, ObjectId playerId, const CharacterTitleCodeSet& titleCodeSet)
{
    go::Entity* player = getRequestQueryPlayer(playerId);
    if (player) {
        player->queryCharacterTitleable()->initializeCharacterTitle(errorCode, titleCodeSet);
    }
}


go::Entity* World::getRequestQueryPlayer(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    RequestQueryPlayerMap::iterator pos = requestQueryPlayerMap_.find(playerId);
    if (pos != requestQueryPlayerMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}

}} // namespace gideon { namespace zoneserver {
