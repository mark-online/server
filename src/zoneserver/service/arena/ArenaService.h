#pragma once

#include "../../zoneserver_export.h"
#include "ArenaServiceCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ArenaInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/core/utility/Singleton.h>
#include <sne/base/concurrent/Future.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

class WorldMap;
class WorldMapHelper;
class Arena;

/**
 * @struct ChallengerInfo
 */
struct ChallengerInfo
{
	sne::server::ServerId zoneServerId_;
	ObjectId playerId_;

	explicit ChallengerInfo(sne::server::ServerId zoneServerId, ObjectId playerId) :
		zoneServerId_(zoneServerId),
		playerId_(playerId) {}
};

/**
 * @struct ArenaMatchInfo
 */
struct ArenaMatchInfo
{
    enum { WaitingTime = 60000 };

	MapCode mapCode_;
	ObjectId arenaId_;
    GameTime waitoutTime_;

	explicit ArenaMatchInfo(MapCode mapCode, ObjectId arenaId, GameTime startWaitingTime) :
		mapCode_(mapCode),
		arenaId_(arenaId),
        waitoutTime_(startWaitingTime + WaitingTime) {}

    bool isWaitTimeout(GameTime now) const {
        return waitoutTime_ <= now;
    }
};

/**
 * @class ArenaService
 *
 * 투기장 서비스
 */
class ZoneServer_Export ArenaService : public ArenaServiceCallback
{
    SNE_DECLARE_SINGLETON(ArenaService);
    typedef std::mutex LockType;

	typedef sne::core::List<ChallengerInfo> ChallengerInfos;
	typedef sne::core::HashMap<MapCode, ChallengerInfos> ChallengeWaitingLists;
	typedef sne::core::List<ArenaMatchInfo> ArenaMatchInfos;
	typedef sne::core::HashMap<ObjectId, ArenaMatchInfos> MatchWaitingLists;
	typedef sne::core::HashMap<ObjectId, Arena*> Arenas;
	typedef sne::core::HashMap<MapCode, ObjectIdSet> ArenaObjectIds;
    typedef sne::core::HashMap<ObjectId, sec_t> DeserterMap;

public:
    ArenaService(WorldMapHelper& worldMapHelper);
	~ArenaService() {}

    bool initialize();

	ErrorCode challenge(sne::server::ServerId serverId, ObjectId playerId, MapCode mapCode);
	ErrorCode cancelChallenge(ObjectId playerId, MapCode mapCode);
	ErrorCode enterArena(ObjectPosition& spawnPosition, WorldMap*& arenaWorldMap, go::Entity& challenger,
        ObjectId arenaId, MapCode preGlobalMapCode);

    void removeChallengerAndWaitMatcher(ObjectId challengerId);
    void removeChallenger(ObjectId challengerId);
    void removeArena(ObjectId arenaId);
	void matching();

    // 치트키 탈영병 해제
    void releaseDeserter(ObjectId characterId);

private:
	void trespassTeamDeathMatch(MapCode mapCode, ChallengerInfos& challengers);
	void matchingVsMode(MapCode mapCode, ChallengerInfos& challengers);
	void matched(const ChallengerInfo& challengerInfo, ObjectId arenaId, MapCode mapCode);

	bool isExistChallenger(MapCode mapCode, ObjectId playerId) const;
    bool isExistWaitMatcher(MapCode mapCode, ObjectId playerId) const;

	void startMatching();
	void stopMatching();
	
    void removeChallengerAndWaitMatcher_i(ObjectId challengerId);
    void removeWaitMatcher_i(ObjectId challengerId);
    void removeChallenger_i(ObjectId challengerId);

    void removeTimeoutWaitMatcher();
    void removeEmptyArena();
    void removeDeserters();
    void removeArena_i(ObjectId arenaId);
    void stopArenas();
    void revivePlayerInArenas();
    void releaseDeserters();

private:
	Arena* createOrGetArena(ObjectId arenaId, MapCode mapCode);
	Arena* getArena(ObjectId arenaId);

    bool isExistDeserter(ObjectId playerId) const;

private:
    // = ArenaServiceCallback overriding
    virtual void registDeserter(ObjectId playerId);

private:
    WorldMapHelper& worldMapHelper_;

	ObjectId lastArenaId_;
	ChallengeWaitingLists challengeWaitingLists_;
	MatchWaitingLists matchWaitingLists_;

	ArenaObjectIds arenaObjectIds_;
	Arenas arenas_;
    DeserterMap deserterMap_;

	sne::base::Future::WeakRef matchingTask_;

    mutable LockType lock_;
	mutable LockType deserterLock_;
};

}} // namespace gideon { namespace zoneserver {

#define ARENA_SERVICE gideon::zoneserver::ArenaService::instance()
