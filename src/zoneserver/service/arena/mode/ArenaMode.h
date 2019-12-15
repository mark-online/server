#pragma once

#include "../../party/PartyPlayerHelper.h"
#include "../../../model/gameobject/EntityEvent.h"
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/ArenaInfo.h>
#include <gideon/cs/shared/data/RegionInfo.h>
#include <gideon/cs/shared/data/Chatting.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
class EntityEvent;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {

class WorldMap;
class MapRegion;

/**
 * @class ArenaMode
 */
class ArenaMode : public PartyPlayerHelper
{
    typedef sne::core::HashMap<ObjectId, MapCode> PreGlobalMapCodes;
    typedef sne::core::HashMap<ObjectId, go::Entity*> Players;
    typedef sne::core::HashMap<ObjectId, const MapRegion*> MapRegions;
public:
    ArenaMode(ArenaModeType arenaModeType, uint32_t minChallenger,
        uint32_t maxChallenger, sec_t playingTime, WorldMap& worldMap);
    virtual ~ArenaMode();

    virtual bool initialize() = 0;
    virtual void finalize() = 0;

    virtual bool addWaitMatcher(ObjectId matcherId) = 0;
    virtual bool addWaitMatchers(const ObjectIdSet& matcherIds) = 0;
    virtual void removeWaitMatcher(ObjectId matcherId) = 0;
    virtual bool arrangePosition(ObjectPosition& position, go::Entity& player, MapCode preGlobalMapCode) = 0;
    virtual void readyToPlay(go::Entity& player) = 0;
    virtual void killed(go::Entity& killer, go::Entity& victim) = 0;
    virtual void stop() = 0;
    virtual void leave(ObjectId playerId);
    virtual void say(ObjectId playerId, const ChatMessage& message);
    virtual void setWaypoint(ObjectId playerId, const Waypoint& waypoint) = 0;
    virtual void notifyMoved(ObjectId playerId, float32_t x, float32_t y) = 0;
    
    virtual bool shouldDestroy() const = 0;
    virtual bool canJoin() const = 0;
    virtual bool shouldStop() const = 0;
    virtual bool isEnoughValidPlayTime() const = 0;

    virtual bool isSameTeam(ObjectId sourceId, ObjectId targetId) const = 0;
    
public:
    MapCode getPreGlobalMapCode(ObjectId playerId) const;

    bool isPlaying() const;
    bool canMoving(ObjectId playerId, const Position& movePosition) const;
    
    virtual void revivePlayers();

protected:
    ArenaModeType getArenaModeType() const {
        return arenaModeType_;
    }

    uint32_t getMinChallenger() const {
        return minChallenger_;
    }

    uint32_t getMaxChallenger() const {
        return maxChallenger_;
    }

    sec_t getPlayingTime() const {
        return playingTime_;
    }

    WorldMap& getWorldMap() {
        return worldMap_;
    }

    const WorldMap& getWorldMap() const {
        return worldMap_;
    }

    const MapRegion* getPlayerSpawnRegion(ObjectId playerId) const;
    RegionCode getPlayerRegionCode(ObjectId playerId) const;
    sec_t getReviveLeftTime() const;

    ObjectPosition getSpawnPosition(const MapRegion& region) const;

protected:
    // = PartyPlayerHelper overriding
    go::Entity* getPlayer(ObjectId playerId);
    const go::Entity* getPlayer(ObjectId playerId) const;

protected:
    void addPlayer(go::Entity* player, const MapRegion& mapRegion, MapCode preGlobalMapCode);
    void removePlayer(ObjectId playerId);
    void setArenaState(ArenaState state) {
        state_ = state;
    }

private:
    void addPreGlobalMapCode(ObjectId matcherId, MapCode preGlobalMapCode);
    void removePreGlobalMapCode(ObjectId playerId);
    
protected:
    void notifyEvent(go::EntityEvent::Ref event, ObjectId exceptPlayerId = invalidObjectId);
    
private:
    WorldMap& worldMap_;
    ArenaState state_;
    ArenaModeType arenaModeType_;
    uint32_t minChallenger_;
    uint32_t maxChallenger_;
    sec_t playingTime_;
    PreGlobalMapCodes preGlobalMapCodes_;
    Players players_;
    MapRegions mapRegions_;
    sec_t nextReviveTime_;
};


}} // namespace gideon { namespace zoneserver {