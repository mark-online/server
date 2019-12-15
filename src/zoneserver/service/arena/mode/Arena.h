#pragma once

#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <gideon/cs/shared/data/AccountId.h>

namespace gdt {
class arena_t;
} // namespace gdt {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

class WorldMapHelper;
class WorldMap;
class ArenaMode;
class ArenaServiceCallback;

/**
 * @class Arena
 */
class Arena : public sne::core::ThreadSafeMemoryPoolMixin<Arena>
{
	typedef std::mutex LockType;

public:
	Arena(ObjectId arenaId, ArenaServiceCallback& serviceCallback,
        WorldMapHelper& worldMapHelper);
	~Arena();

	bool initialize(MapCode mapCode);
    void finalize();

public:
	// 팀을 배정하고, 스폰 위치를 리턴
	ObjectPosition arrangePosition(go::Entity& player, MapCode preGlobalMapCode);
    void readyToPlay(go::Entity& player);
    bool addWaitMatcher(ObjectId challengerId);
    void removeWaitMatcher(ObjectId challengerId);
    
    void despawn(go::Entity& player);
    void leave(AccountId accountId, ObjectId playerId);
    void revivePlayers();    

    MapCode getPreGlobalMapCode(ObjectId playerId) const;
    MapCode getMapCode() const {
        return mapCode_;
    }
    
    void killed(go::Entity& killer, go::Entity& victim);
    void stop();
    void say(ObjectId playerId, const ChatMessage& message);
    bool isSameTeam(ObjectId sourceId, ObjectId targetId) const;
    bool canMoving(ObjectId playerId, const Position& movePosition) const;
    void setWaypoint(ObjectId playerId, const Waypoint& waypoint);
    void notifyMoved(ObjectId playerId, float32_t x, float32_t y);
        
public:
	WorldMap& getWorldMap();
	ObjectId getArenaId() const {
		return arenaId_;
	}

	bool canJoin() const;
    bool shouldDestroy() const;
    bool shouldStop() const;
    bool isPlaying() const;
    bool canMountVehicle() const;
    bool canMonutGlider() const;
    bool isEnoughValidPlayTime() const;

private:
	const ObjectId arenaId_;
    ArenaServiceCallback& serviceCallback_;
    WorldMapHelper& worldMapHelper_;

    MapCode mapCode_;
    std::unique_ptr<WorldMap> worldMap_;
    std::unique_ptr<ArenaMode> arenaMode_;

    const gdt::arena_t* arenaTempate_;

    mutable LockType lock_;
};


}} // namespace gideon { namespace zoneserver {
