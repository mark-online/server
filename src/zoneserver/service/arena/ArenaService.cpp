#include "ZoneServerPCH.h"
#include "ArenaService.h"
#include "mode/Arena.h"
#include "../time/GameTimer.h"
#include "../../s2s/arena/ArenaServerSideProxyManager.h"
#include "../../model/gameobject/Entity.h"
#include <gideon/cs/datatable/ArenaTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver {

namespace {


/**
 * @class ArenaDestoryTask
 */
class ArenaDestoryTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaDestoryTask>
{
public:
    ArenaDestoryTask(Arena& arena) :
        arena_(arena) {}

private:
    virtual void run() {
        arena_.finalize();
        boost::checked_delete(&arena_);
    }

private:
    Arena& arena_;
};


/**
 * @class MatchingTask
 */
class MatchingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<MatchingTask>
{
public:
    MatchingTask() {}

private:
    virtual void run() {
        ARENA_SERVICE->matching();
    }
};


} // namespace {

// = ArenaService;

SNE_DEFINE_SINGLETON(ArenaService)

ArenaService::ArenaService(WorldMapHelper& worldMapHelper) :
    worldMapHelper_(worldMapHelper),
    lastArenaId_(invalidObjectId)
{
}


bool ArenaService::initialize()
{
    if (! ARENA_TABLE) {
        return false;
    }

    const datatable::ArenaTable::ArenaMap& arenaMap = ARENA_TABLE->getArenaMap();

    for (const datatable::ArenaTable::ArenaMap::value_type& value : arenaMap) {
        challengeWaitingLists_.emplace(value.first, ChallengerInfos());
        arenaObjectIds_.emplace(value.first, ObjectIdSet());
    }
    
    ArenaService::instance(this);

    startMatching();

    return true;
}


ErrorCode ArenaService::challenge(sne::server::ServerId serverId, ObjectId playerId, MapCode mapCode)
{
    std::lock_guard<LockType> lock(lock_);

    if (! isValidMapCode(mapCode)) {
        return ecArenaInvalidArena;
    }

    const gdt::arena_t* arenaTempate = ARENA_TABLE->getArenaTemplate(mapCode);
    if (! arenaTempate) {
        return ecArenaInvalidArena;
    }

    if (isExistChallenger(mapCode, playerId)) {
        return ecArenaAlreayChallenge;
    }

    if (isExistWaitMatcher(mapCode, playerId)) {
        return ecArenaMatcher;
    }

    if (isExistDeserter(playerId)) {
        return ecArenaNotChallengDeserter;
    }

    ChallengeWaitingLists::iterator challengePos = challengeWaitingLists_.find(mapCode);
    if (challengePos != challengeWaitingLists_.end()) {
        ChallengerInfos& challengers = (*challengePos).second;
        challengers.push_back(ChallengerInfo(serverId, playerId));
    }

    return ecOk;
}


ErrorCode ArenaService::cancelChallenge(ObjectId playerId, MapCode mapCode)
{
    std::lock_guard<LockType> lock(lock_);

    ChallengeWaitingLists::iterator challengerPos = challengeWaitingLists_.find(mapCode);
    if (challengerPos != challengeWaitingLists_.end()) {
        ChallengerInfos& challengers = (*challengerPos).second;
        ChallengerInfos::iterator pos = challengers.begin();
        ChallengerInfos::iterator end = challengers.end();

        for (; pos != end; ++pos) {
            const ChallengerInfo& challengerInfo = (*pos);
            if (challengerInfo.playerId_ == playerId) {
                challengers.erase(pos);
                break;
            }
        }
    }

    MatchWaitingLists::iterator matchPos = matchWaitingLists_.find(playerId);
    if (matchPos != matchWaitingLists_.end()) {
        ObjectId challengerId = (*matchPos).first;
        ArenaMatchInfos& matchInfos = (*matchPos).second;
        ArenaMatchInfos::iterator pos = matchInfos.begin();
        ArenaMatchInfos::iterator end = matchInfos.end();

        for (; pos != end; ++pos) {
            const ArenaMatchInfo& info = (*pos);
            if (info.mapCode_ == mapCode) {
                Arena* arena = getArena(info.arenaId_);
                if (arena) {
                    arena->removeWaitMatcher(challengerId);
                    if (arena->shouldDestroy()) {
                        removeArena_i(info.arenaId_);
                    }
                }

                matchInfos.erase(pos);
                if (matchInfos.empty()) {
                    matchWaitingLists_.erase(playerId);
                }
                break;
            }
        }
    }

    return ecOk;
}


ErrorCode ArenaService::enterArena(ObjectPosition& spawnPosition, WorldMap*& arenaWorldMap,
    go::Entity& challenger, ObjectId arenaId, MapCode preGlobalMapCode)
{
    std::lock_guard<LockType> lock(lock_);

    MapCode mapCode = invalidMapCode;
    MatchWaitingLists::iterator matchPos = matchWaitingLists_.find(challenger.getObjectId());
    if (matchPos != matchWaitingLists_.end()) {
        ArenaMatchInfos& matchInfos = (*matchPos).second;
        ArenaMatchInfos::iterator pos = matchInfos.begin();
        ArenaMatchInfos::iterator end = matchInfos.end();

        for (; pos != end; ++pos) {
            const ArenaMatchInfo& matchInfo = (*pos);
            if (matchInfo.arenaId_ == arenaId) {
                mapCode = matchInfo.mapCode_;
                break;
            }
        }
    }
    
    if (! isValidMapCode(mapCode)) {
        return ecArenaInvalidArena;
    }
    
    Arena* arena = createOrGetArena(arenaId, mapCode);
    if (! arena) {
        return ecArenaInvalidArena;
    }
    
    spawnPosition = arena->arrangePosition(challenger, preGlobalMapCode);

    arenaWorldMap = &arena->getWorldMap();

    removeChallengerAndWaitMatcher_i(challenger.getObjectId());

    return ecOk;
}


void ArenaService::removeChallengerAndWaitMatcher(ObjectId challengerId)
{
    std::lock_guard<LockType> lock(lock_);

    removeChallengerAndWaitMatcher_i(challengerId);
}


void ArenaService::removeChallenger(ObjectId challengerId)
{
    std::lock_guard<LockType> lock(lock_);

    removeChallenger_i(challengerId);
}


void ArenaService::removeArena(ObjectId arenaId)
{
    std::lock_guard<LockType> lock(lock_);

    removeArena_i(arenaId);
}


void ArenaService::matching()
{
    {
        std::lock_guard<LockType> lock(lock_);

        for (ChallengeWaitingLists::value_type& value : challengeWaitingLists_) {
            ChallengerInfos& challengers = value.second;
            MapCode mapCode = value.first;
            const gdt::arena_t* arenaTempate = ARENA_TABLE->getArenaTemplate(mapCode);
            if (! arenaTempate) {
                assert(false);
            }
            ArenaModeType arenaModeType = toArenaModeType(arenaTempate->arena_mode_type());
            if (arenaModeType == amtTeamDeathMatch) {
                trespassTeamDeathMatch(mapCode, challengers);
            }	
            matchingVsMode(mapCode, challengers);
        }

        revivePlayerInArenas();
        removeTimeoutWaitMatcher();
        stopArenas();
        removeEmptyArena();
    }
    releaseDeserters();
}


void ArenaService::releaseDeserter(ObjectId playerId)
{
    {
        std::lock_guard<LockType> lock(deserterLock_);

        deserterMap_.erase(playerId);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateDeserterExpireTime(playerId, 0);
    }
}


void ArenaService::trespassTeamDeathMatch(MapCode mapCode, ChallengerInfos& challengers)
{
    ArenaObjectIds::iterator arenaPos = arenaObjectIds_.find(mapCode);
    if (arenaPos != arenaObjectIds_.end()) {
        ObjectIdSet arenaIds = (*arenaPos).second;
        for (const ObjectIdSet::value_type& value : arenaIds) {
            const ObjectId areanId = value;
            Arena* arena = getArena(areanId);
            if (arena) {
                if (! arena->canJoin()) {
                    break;
                }
                ChallengerInfos::iterator pos = challengers.begin();
                if (pos != challengers.end()) {
                    const ChallengerInfo& challengerInfo = (*pos);
                    matched(challengerInfo, areanId, mapCode);
                    arena->addWaitMatcher(challengerInfo.playerId_);
                    challengers.erase(pos);
                }
            }
        }
    } 
}


void ArenaService::matchingVsMode(MapCode mapCode, ChallengerInfos& challengers)
{
    const gdt::arena_t* arenaTempate = ARENA_TABLE->getArenaTemplate(mapCode);
    if (! arenaTempate) {
        return;
    }

    const size_t minModeMatcherCount = arenaTempate->min_player_count();
    const size_t maxModeMatcherCount = arenaTempate->max_player_count();

    const size_t totalCount = challengers.size();
    if (totalCount < minModeMatcherCount) {
        return;
    }
    
    size_t remainder = totalCount % maxModeMatcherCount;
    size_t loopCount = totalCount;
    if (remainder < minModeMatcherCount) {
        loopCount -= remainder;
    }	
    
    for (size_t i = 0; i < loopCount; ++i) {
        const bool shouldGenerateArenaId = (i % maxModeMatcherCount) == 0;
        if (shouldGenerateArenaId) {
            ++lastArenaId_;
        }
        
        ChallengerInfos::iterator pos = challengers.begin();
        if (pos != challengers.end()) {
            const ChallengerInfo& challengerInfo = (*pos);
            matched(challengerInfo, lastArenaId_, mapCode);
            Arena* arena = createOrGetArena(lastArenaId_, mapCode);
            if (arena) {
                bool result = arena->addWaitMatcher(challengerInfo.playerId_);
                if (! result) {
                    assert(false);
                }
                challengers.erase(pos);
            }
        }
    }
}


void ArenaService::matched(const ChallengerInfo& challengerInfo, ObjectId arenaId, MapCode mapCode)
{
    MatchWaitingLists::iterator matchPos = matchWaitingLists_.find(challengerInfo.playerId_);
    if (matchPos != matchWaitingLists_.end()) {
        ArenaMatchInfos& arenaInfos = (*matchPos).second;
        arenaInfos.push_back(ArenaMatchInfo(mapCode, arenaId, GAME_TIMER->msec()));
    }
    else {
        ArenaMatchInfos arenaInfos;
        arenaInfos.push_back(ArenaMatchInfo(mapCode, arenaId, GAME_TIMER->msec()));
        matchWaitingLists_.emplace(challengerInfo.playerId_, arenaInfos);
    }
    ARENASERVERSIDEPROXY_MANAGER->matched(challengerInfo.zoneServerId_, 
        arenaId, challengerInfo.playerId_, mapCode);
}


void ArenaService::startMatching()
{
    sne::base::Future::Ref matchingTask = matchingTask_.lock();
    if (matchingTask.get()) {
        return;
    }

    const msec_t tenSec = 10000;
    matchingTask_ = TASK_SCHEDULER->schedule(std::make_unique<MatchingTask>(), tenSec, tenSec);
}


void ArenaService::stopMatching()
{
    sne::base::Future::Ref matchingTask = matchingTask_.lock();
    if (! matchingTask.get()) {
        return;
    }

    matchingTask->cancel();
    matchingTask_.reset();
}


void ArenaService::removeChallengerAndWaitMatcher_i(ObjectId challengerId)
{
    removeWaitMatcher_i(challengerId);
    removeChallenger_i(challengerId);
}


void ArenaService::removeWaitMatcher_i(ObjectId challengerId)
{
    MatchWaitingLists::const_iterator matchPos = matchWaitingLists_.find(challengerId);
    if (matchPos != matchWaitingLists_.end()) {
        const ArenaMatchInfos& arenaInfos = (*matchPos).second;
        ArenaMatchInfos::const_iterator pos = arenaInfos.begin();
        ArenaMatchInfos::const_iterator end = arenaInfos.end();
        for (; pos != end; ++pos) {
            const ArenaMatchInfo& info = (*pos);
            Arena* arena = getArena(info.arenaId_);
            if (arena) {
                arena->removeWaitMatcher(challengerId);
                if (arena->shouldDestroy()) {
                    removeArena_i(info.arenaId_);
                }
            }
        }
    }
    matchWaitingLists_.erase(challengerId);
}



void ArenaService::removeChallenger_i(ObjectId challengerId)
{
    ChallengeWaitingLists::iterator challengePos = challengeWaitingLists_.begin();
    ChallengeWaitingLists::const_iterator challengeEnd = challengeWaitingLists_.end();
    for (; challengePos != challengeEnd; ++challengePos) {  
        ChallengerInfos& challengerInfos = (*challengePos).second;
        ChallengerInfos::iterator pos = challengerInfos.begin();
        ChallengerInfos::const_iterator end = challengerInfos.end();
        for (; pos != end; ++pos) {
            const ChallengerInfo& info = (*pos);
            if (info.playerId_ == challengerId) {
                challengerInfos.erase(pos);
                break;
            }
        }
    }
}


void ArenaService::removeTimeoutWaitMatcher()
{
    GameTime now = GAME_TIMER->msec();
    MatchWaitingLists::iterator matchPos = matchWaitingLists_.begin();
    
    while (matchPos != matchWaitingLists_.end()) {
        ObjectId challengerId = (*matchPos).first;
        ArenaMatchInfos& matchInfos = (*matchPos).second;
        ArenaMatchInfos::iterator pos = matchInfos.begin();
        while (pos != matchInfos.end()) {
            const ArenaMatchInfo& info = (*pos);
            if (info.isWaitTimeout(now)) {
                Arena* arena = getArena(info.arenaId_);
                if (arena) {
                    arena->removeWaitMatcher(challengerId);
                    if (arena->shouldDestroy()) {
                        removeArena_i(info.arenaId_);
                    }
                }
                matchInfos.erase(pos++);
            }
            else {
                ++pos;
            }
        }

        if (matchInfos.empty()) {
            matchPos = matchWaitingLists_.erase(matchPos);
        }
        else {
            ++matchPos;
        }
    }
}


void ArenaService::removeEmptyArena()
{
    const msec_t after = 5000;

    Arenas::iterator pos = arenas_.begin();
    while (pos != arenas_.end()) {
        Arena* arena = (*pos).second;
        if (arena->shouldDestroy()) {
            arena->finalize();
            (void)TASK_SCHEDULER->schedule(std::make_unique<ArenaDestoryTask>(*arena), after);
            pos = arenas_.erase(pos);
        }
        else {
            ++pos;
        }
    }
}


void ArenaService::removeDeserters()
{
    sec_t now = getTime();
    DeserterMap::iterator pos = deserterMap_.begin();
    while (pos != deserterMap_.end()) {
        sec_t releaseTime = (*pos).second;
        if (releaseTime < now) {
            ObjectId playerId = (*pos).first;
            pos = deserterMap_.erase(pos);
            {
                DatabaseGuard db(SNE_DATABASE_MANAGER);
                db->asyncUpdateDeserterExpireTime(playerId, 0);
            }
            continue;
        }
        ++pos;
    }
}


void ArenaService::removeArena_i(ObjectId arenaId)
{
    const msec_t after = 5000;

    MapCode mapCode = invalidMapCode;
    Arenas::iterator pos = arenas_.find(arenaId);
    if (pos != arenas_.end()) {
        Arena* arena = (*pos).second;
        mapCode = arena->getMapCode();
        (void)TASK_SCHEDULER->schedule(std::make_unique<ArenaDestoryTask>(*arena), after);
    }
    arenas_.erase(arenaId);

    ArenaObjectIds::iterator idPos = arenaObjectIds_.find(mapCode);
    if (idPos != arenaObjectIds_.end()) {
        ObjectIdSet& ids = (*idPos).second;
        ids.erase(arenaId);
    }
}


void ArenaService::stopArenas()
{
    for (Arenas::value_type& value : arenas_) {
        Arena* arena = value.second;
        if (arena) {
            if (arena->shouldStop()) {
                arena->stop();
            }
        }
    }
}


void ArenaService::revivePlayerInArenas()
{
    for (Arenas::value_type& value : arenas_) {
        Arena* arena = value.second;
        if (arena) {
            if (arena->isPlaying()) {
                arena->revivePlayers();
            }
        }
    }
}


void ArenaService::releaseDeserters()
{
    std::lock_guard<LockType> lock(deserterLock_);

    sec_t now = getTime();
    DeserterMap::iterator pos = deserterMap_.begin();
    while (pos != deserterMap_.end()) {
        sec_t releaseTlme = (*pos).second;
        if (releaseTlme <= now) {
            pos = deserterMap_.erase(pos);
            continue;
        }
        ++pos;
    }
}


bool ArenaService::isExistChallenger(MapCode mapCode, ObjectId playerId) const
{
    ChallengeWaitingLists::const_iterator challengePos = challengeWaitingLists_.find(mapCode);
    if (challengePos != challengeWaitingLists_.end()) {
        const ChallengerInfos& challengerInfos = (*challengePos).second;
        for (const ChallengerInfo& info : challengerInfos) {
            if (info.playerId_ == playerId) {
                return true;
            }
        }
        return false;
    }
    assert(false);
    return false;
}


bool ArenaService::isExistWaitMatcher(MapCode mapCode, ObjectId playerId) const
{
    MatchWaitingLists::const_iterator matchPos = matchWaitingLists_.find(playerId);
    if (matchPos != matchWaitingLists_.end()) {
        const ArenaMatchInfos& matchInfos = (*matchPos).second;
        ArenaMatchInfos::const_iterator pos = matchInfos.begin();
        ArenaMatchInfos::const_iterator end = matchInfos.end();

        for (; pos != end; ++pos) {
            const ArenaMatchInfo& matchInfo = (*pos);
            if (matchInfo.mapCode_ == mapCode) {
                mapCode = matchInfo.mapCode_;
                return true;
            }
        }
    }
    return false;
}


Arena* ArenaService::getArena(ObjectId arenaId)
{
    Arenas::iterator pos = arenas_.find(arenaId);
    if (pos != arenas_.end()) {
        return (*pos).second;
    }

    return nullptr;
}


Arena* ArenaService::createOrGetArena(ObjectId arenaId, MapCode mapCode)
{
    Arena* arena = getArena(arenaId);
    if (! arena) {
        std::unique_ptr<Arena> newArena =
            std::make_unique<Arena>(arenaId, *this, worldMapHelper_);
        if (! newArena->initialize(mapCode)) {
            return nullptr;
        }
        arena = newArena.release();
        arenas_.emplace(arenaId, arena);

        const ArenaObjectIds::iterator pos = arenaObjectIds_.find(mapCode);
        if (pos != arenaObjectIds_.end()) {
            ObjectIdSet& ids = (*pos).second;
            ids.insert(arenaId);
        }

    }
    return arena;
}


bool ArenaService::isExistDeserter(ObjectId playerId) const
{
    return deserterMap_.find(playerId) != deserterMap_.end();
}

// = ArenaServiceCallback overriding

void ArenaService::registDeserter(ObjectId playerId)
{
    const sec_t releaseTime = getTime() + lockDeserterTime;
    {
        std::lock_guard<LockType> lock(deserterLock_);

        deserterMap_.emplace(playerId, releaseTime);
    }

    {   
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateDeserterExpireTime(playerId, releaseTime);
    }
}

}} // namespace gideon { namespace zoneserver {
