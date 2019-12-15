#include "ZoneServerPCH.h"
#include "ArenaMode.h"
#include "ArenaEvents.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/ability/Liveable.h"
#include "../../../model/gameobject/ability/ArenaMatchable.h"
#include "../../../world/region/MapRegion.h"
#include "../../../model/gameobject/status/CreatureStatus.h"
#include <gideon/3d/3d.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver {


ArenaMode::ArenaMode(ArenaModeType arenaModeType, uint32_t minChallenger,
    uint32_t maxChallenger, sec_t playingTime, WorldMap& worldMap) :
    state_(asWaiting),
    arenaModeType_(arenaModeType),
    minChallenger_(minChallenger),
    maxChallenger_(maxChallenger),
    playingTime_(playingTime),
    worldMap_(worldMap),
    nextReviveTime_(0)
{
}


ArenaMode::~ArenaMode()
{
}


void ArenaMode::leave(ObjectId playerId)
{
    removePreGlobalMapCode(playerId);
    removePlayer(playerId);
}


void ArenaMode::say(ObjectId playerId, const ChatMessage& message)
{
    auto event = std::make_shared<ArenaPlayerSayEvent>(playerId, message);
    notifyEvent(event);
}


MapCode ArenaMode::getPreGlobalMapCode(ObjectId playerId) const
{
    const PreGlobalMapCodes::const_iterator pos = preGlobalMapCodes_.find(playerId);
    if (pos != preGlobalMapCodes_.end()) {
        return (*pos).second;
    }
    return invalidMapCode;
}


bool ArenaMode::isPlaying() const
{
    return asPlaying == state_;
}


bool ArenaMode::canMoving(ObjectId playerId, const Position& movePosition) const
{
    if (state_ == asStop) {
        return false;
    }
    if (state_ == asWaiting || state_ == asCountdown) {
        const MapRegion* region = getPlayerSpawnRegion(playerId);
        if (! region) {
            return false;
        }
        return region->isContained(movePosition);
    }
    return true;
}


void ArenaMode::revivePlayers()
{   
    const sec_t nextCallTime = 20;
    const sec_t now = getTime();
    if (nextReviveTime_ > now) {
        return;
    }
    nextReviveTime_ = getTime() + nextCallTime;
    for (Players::value_type& value : players_) {
        go::Entity* player = value.second;
        if (! player) {
            continue;
        }

        if (! player->queryLiveable()->getCreatureStatus().isDied()) {
            continue;
        }

        const MapRegion* region = getPlayerSpawnRegion(player->getObjectId());
        if (region) {
            const ObjectPosition spawnPosition = getSpawnPosition(*region);
            const ErrorCode errorCode = player->queryArenaMatchable()->reviveInArena(spawnPosition);
            if (isSucceeded(errorCode)) {
                auto event = std::make_shared<ArenaPlayerReviveEvent>(player->getObjectId(),
                    spawnPosition);
                notifyEvent(event);
            }
        }
    }
}


sec_t ArenaMode::getReviveLeftTime() const
{
    const sec_t now = getTime();
    if (nextReviveTime_ >= now) {
        return nextReviveTime_ - now;
    }
    return 0;
}


ObjectPosition ArenaMode::getSpawnPosition(const MapRegion& region) const
{
    ObjectPosition position = region.getRandomPosition();

    // 맵 중앙을 바라보도록 한다
    position.heading_ = getHeading(Position(), position);
    return position;
}


const MapRegion* ArenaMode::getPlayerSpawnRegion(ObjectId playerId) const
{
    const MapRegions::const_iterator pos = mapRegions_.find(playerId);
    if (pos != mapRegions_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


RegionCode ArenaMode::getPlayerRegionCode(ObjectId playerId) const
{
    const MapRegion* region = getPlayerSpawnRegion(playerId);
    if (region) {
        return region->getRegionCode();
    }
    return invalidRegionCode;
}


go::Entity* ArenaMode::getPlayer(ObjectId playerId)
{
    const Players::iterator pos = players_.find(playerId);
    if (pos != players_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


const go::Entity* ArenaMode::getPlayer(ObjectId playerId) const
{
    const Players::const_iterator pos = players_.find(playerId);
    if (pos != players_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


void ArenaMode::addPlayer(go::Entity* player, const MapRegion& mapRegion, MapCode preGlobalMapCode)
{
    players_.emplace(player->getObjectId(), player);
    mapRegions_.emplace(player->getObjectId(), &mapRegion);
    addPreGlobalMapCode(player->getObjectId(), preGlobalMapCode);
}


void ArenaMode::removePlayer(ObjectId playerId)
{
    players_.erase(playerId);
    mapRegions_.erase(playerId);
    removePreGlobalMapCode(playerId);
}


void ArenaMode::addPreGlobalMapCode(ObjectId matcherId, MapCode preGlobalMapCode)
{
    preGlobalMapCodes_.emplace(matcherId, preGlobalMapCode);
}


void ArenaMode::removePreGlobalMapCode(ObjectId matcherId)
{
    preGlobalMapCodes_.erase(matcherId);
}


void ArenaMode::notifyEvent(go::EntityEvent::Ref event, ObjectId exceptPlayerId)
{
    for (const Players::value_type& value : players_) {
        go::Entity* player = value.second;        
        if (player) {
            if (! player->isValid()) {
                continue;
            }
            if (isValidObjectId(exceptPlayerId)) {
                if (player->getObjectId() == exceptPlayerId) {
                    continue;
                }
            }
            if (player->isValid()) {
                (void)TASK_SCHEDULER->schedule(std::make_unique<go::EventCallTask>(*player, event));
            }
        }
    }
}

}} // namespace gideon { namespace zoneserver {
