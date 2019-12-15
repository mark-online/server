#include "ZoneServerPCH.h"
#include "ArenaVsMode.h"
#include "ArenaEvents.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/status/CreatureStatus.h"
#include "../../../model/gameobject/ability/Partyable.h"
#include "../../../model/gameobject/ability/ArenaMatchable.h"
#include "../../../model/gameobject/ability/Liveable.h"
#include "../../../model/gameobject/ability/CharacterClassable.h"
#include "../../../world/World.h"
#include "../../../world/WorldMap.h"
#include "../../../world/region/MapRegion.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/ArenaCallback.h"
#include <gideon/cs/shared/data/RegionInfo.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {


namespace {


/**
 * @class CountdownTask
 */
class CountdownTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<CountdownTask>
{
public:
    CountdownTask(ArenaVsMode& mode) :
        mode_(mode) {}

private:
    virtual void run() {
        mode_.startPlaying();
    }

private:
    ArenaVsMode& mode_;
};

/**
 * @class CountdownTask
 */
class CountdownWaitingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<CountdownWaitingTask>
{
public:
    CountdownWaitingTask(ArenaVsMode& mode) :
        mode_(mode) {}

private:
    virtual void run() {
        mode_.waitCountdownCompleted();
    }

private:
    ArenaVsMode& mode_;
};


RegionType toRegionType(ArenaVsMode::TeamType teamType) 
{
    if (ArenaVsMode::blueTeam == teamType) {
        return rtBlueTeamSpawn;
    }
    else if (ArenaVsMode::redTeam == teamType) {
        return rtRedTeamSpawn;
    }
    assert(false);
    return rtUnknown;
}


ArenaTeamType toArenaTeamType(ArenaVsMode::TeamType teamType) 
{
    if (ArenaVsMode::blueTeam == teamType) {
        return attBlue;
    }
    else if (ArenaVsMode::redTeam == teamType) {
        return attRed;
    }
    assert(false);
    return attUnknown;
}


ArenaVsMode::TeamType toEnemyTeamType(ArenaVsMode::TeamType teamType) 
{
    if (ArenaVsMode::blueTeam == teamType) {
        return ArenaVsMode::redTeam;
    }
    else if (ArenaVsMode::redTeam == teamType) {
        return ArenaVsMode::blueTeam;
    }
    assert(false);
    return ArenaVsMode::noneTeam;
}


} // namespace {


ArenaVsMode::ArenaVsMode(ArenaModeType arenaModeType, uint32_t minChallenger,
    uint32_t maxChallenger, sec_t playingTime, WorldMap& worldMap) :
    ArenaMode(arenaModeType, minChallenger, maxChallenger, playingTime, worldMap)
{
    teamParties_[blueTeam] = std::make_shared<Party>(*this, invalidPartyId, ptSolo);
    teamParties_[redTeam] = std::make_shared<Party>(*this, invalidPartyId, ptSolo);
}


ArenaVsMode::~ArenaVsMode()
{
}


void ArenaVsMode::waitCountdownCompleted()
{
    cancelCountdownWaitingTask();

    if (matchModeInfo_.isWating()) {
        uint32_t startMinTeamMemberCount = getMinChallenger() / 2;
        if (teams_[blueTeam].size() >= startMinTeamMemberCount && 
            teams_[redTeam].size() >= startMinTeamMemberCount) {
                startCountdown();
        }
    }   
}


void ArenaVsMode::startPlaying()
{
    assert(matchModeInfo_.isCountdown());
    matchModeInfo_.startPlaying(getTime());
    setArenaState(matchModeInfo_.state_);
}


bool ArenaVsMode::initialize()
{
    return true;
}


void ArenaVsMode::finalize()
{
    cancelCountdownWaitingTask();
    cancelCountdownTask();
}


bool ArenaVsMode::arrangePosition(ObjectPosition& position, go::Entity& player, MapCode preGlobalMapCode)
{
    const TeamType teamType = getWaitingTeamType(player.getObjectId());
    if (! isValid(teamType)) {
        return false;
    }

    const RegionType regeionType = toRegionType(teamType);
    const MapRegion* mapRegion = getWorldMap().getMapRegion(regeionType);
    assert(mapRegion != nullptr);
    position = getSpawnPosition(*mapRegion);

    addTeamPlayer(player, *mapRegion, teamType, preGlobalMapCode);

    return true;
}


bool ArenaVsMode::addWaitMatcher(ObjectId matcherId)
{
    if (! canJoin()) {
        return false;
    }
    
    TeamType teamType = blueTeam;
    if (getTeamCount(blueTeam) > getTeamCount(redTeam)) {
        teamType = redTeam;
    }

    waitingTeams_[teamType].insert(matcherId);

    return true;
}


bool ArenaVsMode::addWaitMatchers(const ObjectIdSet& /*matcherIds*/)
{
    //uint32_t maxTeamCount = getMaxChallenger() / 2;
    //getTeamCount(blueTeam) + matcherIds.size();

    return true;
}


void ArenaVsMode::addTeamPlayer(go::Entity& player, const MapRegion& mapRegion,
    TeamType teamType, MapCode preGlobalMapCode)
{
    CharacterClass characterClass = ccUnknown;
    go::CharacterClassable* classable = player.queryCharacterClassable();
    if (classable != nullptr) {
        characterClass = classable->getCharacterClass();
    }
    ArenaPlayerInfo addPlyaerInfo(player.getObjectId(), characterClass, player.getNickname(),
        player.queryLiveable()->getCreatureStatus().getHitPoints(),
        player.queryLiveable()->getCreatureStatus().getManaPoints(), 0, 0);    

    addPlayer(&player, mapRegion, preGlobalMapCode);
    if (teamType == redTeam || teamType == blueTeam) {
        teams_[teamType].insert(addPlyaerInfo);
        moveCounts_.emplace(player.getObjectId(), 0);
        teamParties_[teamType]->addMember(player.getObjectId());
        player.queryPartyable()->setParty(teamParties_[teamType]);
    }
}


void ArenaVsMode::removeWaitMatcher(ObjectId matcherId)
{
    ChallengerIds::iterator pos = waitingTeams_[blueTeam].find(matcherId);
    if (pos != waitingTeams_[blueTeam].end()) {
        waitingTeams_[blueTeam].erase(pos);
        return;
    }
    pos = waitingTeams_[redTeam].find(matcherId);
    if (pos != waitingTeams_[redTeam].end()) {
        waitingTeams_[redTeam].erase(pos);
        return;
    }
}


void ArenaVsMode::leave(ObjectId playerId)
{
    go::Entity* player = getPlayer(playerId);
    if (player) {
        player->queryArenaMatchable()->setArena(nullptr);
    }
    ArenaMode::leave(playerId);

    const TeamType teamType = getPlayingTeamType(playerId);
    removeWaitMatcher(playerId);

    if (! isValid(teamType)) {
        return;
    }
    removeTeamPlayer(teamType, playerId);

    auto event = std::make_shared<ArenaLeavePlayerEvent>(playerId, toArenaTeamType(teamType));
    notifyEvent(event);
}


void ArenaVsMode::readyToPlay(go::Entity& player)
{
    const TeamType teamType = getPlayingTeamType(player.getObjectId());
    if (! isValid(teamType)) {
        return;
    }   

    gc::ArenaCallback* callback = player.getController().queryArenaCallback();
    if (callback) {        
        callback->arenaMatchInfos(matchModeInfo_, toArenaTeamType(teamType),
            teams_[teamType], teams_[toEnemyTeamType(teamType)],
            getPlayerRegionCode(player.getObjectId()));
    }

    CharacterClass characterClass = ccUnknown;
    go::CharacterClassable* classable = player.queryCharacterClassable();
    if (classable != nullptr) {
        characterClass = classable->getCharacterClass();
    }
    const ArenaPlayerInfo addPlayerInfo(player.getObjectId(), characterClass, player.getNickname(),
        player.queryLiveable()->getCreatureStatus().getHitPoints(),
        player.queryLiveable()->getCreatureStatus().getManaPoints(), 0, 0); 

    auto event = std::make_shared<ArenaEnterPlayerEvent>(addPlayerInfo, toArenaTeamType(teamType));
    notifyEvent(event, player.getObjectId());

    if (matchModeInfo_.isWating()) {
        if (amtDualMatch == getArenaModeType()) {
            waitCountdownCompleted();
        }
        else {
            uint32_t startMinTeamMemberCount = getMinChallenger() / 2;
            if (teams_[blueTeam].size() >= startMinTeamMemberCount && 
                teams_[redTeam].size() >= startMinTeamMemberCount) {
                startReadyCountdown();
            }
        }
    }

    Position enterPlayerPosition = player.getPosition();

    for (const ArenaPlayerInfos::value_type& value : teams_[teamType]) {
        const ArenaPlayerInfo& info = value;
        go::Entity* member = getPlayer(info.playerId_);        
        if (! member || info.playerId_ == player.getObjectId()) {
            continue;
        }
        
        if (member->isValid()) {
            Position position = member->getPosition();
            if (callback) {
                auto eventMove = std::make_shared<ArenaTeamPlayerMoveEvent>(member->getObjectId(),
                    position.x_, position.y_);
                (void)TASK_SCHEDULER->schedule(
                    std::make_unique<go::EventCallTask>(player, eventMove));
            }

            {
                auto enterPlayerEvent = std::make_shared<ArenaTeamPlayerMoveEvent>(player.getObjectId(),
                    enterPlayerPosition.x_, enterPlayerPosition.y_);
                (void)TASK_SCHEDULER->schedule(
                    std::make_unique<go::EventCallTask>(*member, enterPlayerEvent));
            }
        }
    }       
}


void ArenaVsMode::killed(go::Entity& killer, go::Entity& victim)
{    
    uint32_t killerScore = 0;

    if (killer.isPlayer()) {
        const TeamType killerTeam = getPlayingTeamType(killer.getObjectId());
        if (isValid(killerTeam) && ! (killer.isSame(victim.getGameObjectInfo()))) {
            if (blueTeam == killerTeam) {
                ++matchModeInfo_.blueScore_;
                ++killCounts_[blueTeam];
            }
            else {
                ++matchModeInfo_.redScore_;
                ++killCounts_[redTeam];
            }
            ArenaPlayerInfo* arenaPlayer = getArenaPlayerInfo(killerTeam, killer.getObjectId());
            if (arenaPlayer) {
                ++arenaPlayer->killCount_;
                killerScore = ++arenaPlayer->score_;
            }
        }
    }
    else {
        const TeamType deadTeam = getPlayingTeamType(victim.getObjectId());
        if (blueTeam == deadTeam) {
            ++matchModeInfo_.redScore_;
        }
        else if( redTeam == deadTeam) {
            ++matchModeInfo_.blueScore_;
        }
    }
    if (victim.isPlayer()) {
        const TeamType deadManTeam = getPlayingTeamType(victim.getObjectId());
        if (isValid(deadManTeam)) {
            ArenaPlayerInfo* deadMan = getArenaPlayerInfo(deadManTeam, victim.getObjectId());
            if (deadMan) {
                ++deadMan->deathCount_;
            }
            ++deadCounts_[deadManTeam];
        }
    }
    
    auto event1 = std::make_shared<ArenaPlayerKillEvent>(
        killer.getGameObjectInfo(), victim.getGameObjectInfo());
    notifyEvent(event1);

    if (killer.isPlayer()) {
        auto event2 = std::make_shared<ArenaPlayerScoreUpdateEvent>(
            killer.getObjectId(), killerScore);
        notifyEvent(event2);
    }

    auto event3 = std::make_shared<ArenaScoreUpdateEvent>(
        matchModeInfo_.blueScore_, matchModeInfo_.redScore_);
    notifyEvent(event3);

    go::Entity* deadPlayer = getPlayer(victim.getObjectId());
    if (deadPlayer) {
        deadPlayer->getController().queryArenaCallback()->aranaReviveLeftTime(getReviveLeftTime());
    }
}


void ArenaVsMode::say(ObjectId playerId, const ChatMessage& message)
{
    const TeamType teamType = getPlayingTeamType(playerId);
    if (! isValid(teamType)) {
        return;
    }
    auto event = std::make_shared<ArenaPlayerSayEvent>(playerId, message);
    notifyTeamEvent(event, teamType);
}



void ArenaVsMode::setWaypoint(ObjectId playerId, const Waypoint& waypoint)
{
    const TeamType teamType = getPlayingTeamType(playerId);
    if (! isValid(teamType)) {
        assert(false);
        return;
    }

    ArenaPlayerInfo* playerInfo = getArenaPlayerInfo(teamType, playerId);
    if (! playerInfo) {
        assert(false);
        return;
    }
    playerInfo->waypoint_ = waypoint;

    auto event = std::make_shared<ArenaWaypointEvent>(playerId, waypoint);
    notifyTeamEvent(event, teamType);
}


void ArenaVsMode::notifyMoved(ObjectId playerId, float32_t x, float32_t y)
{
    const TeamType team = getPlayingTeamType(playerId);
    if (noneTeam == team) {
        return;
    }
    //const uint32_t nofifyMoveCount = 10;
    //MoveCounts::iterator pos = moveCounts_.find(playerId);
    //if (pos == moveCounts_.end()) {
    //    return;
    //}

    //uint32_t& moveCount = (*pos).second;
    //if (moveCount < nofifyMoveCount) {
    //    ++moveCount;
    //    return;
    //}
    //moveCount = 0;

    auto event = std::make_shared<ArenaTeamPlayerMoveEvent>(playerId, x, y);
    for (const ArenaPlayerInfos::value_type& value : teams_[team]) {
        const ArenaPlayerInfo& info = value;
        go::Entity* player = getPlayer(info.playerId_);
        if (! player) {
            continue;
        }
        
        if (player->isValid()) {
            (void)TASK_SCHEDULER->schedule(std::make_unique<go::EventCallTask>(*player, event));
        }
    }
}


void ArenaVsMode::removeTeamPlayer(TeamType teamType, ObjectId playerId)
{
    moveCounts_.erase(playerId);
    ArenaPlayerInfo info(playerId);
    if (! isValid(teamType)) {
        return;
    }
    teams_[teamType].erase(info);
    teamParties_[teamType]->removeMember(playerId);
    removePlayer(playerId);
}


void ArenaVsMode::startCountdown()
{
    sne::base::Future::Ref countdownTask = countdownTask_.lock();
    if (countdownTask.get()) {
        return;
    }

    matchModeInfo_.startCountdown(getTime());
    setArenaState(matchModeInfo_.state_);

    auto event = std::make_shared<ArenaStartCountDownEvent>(getTime());
    notifyEvent(event);

    countdownTask_ = TASK_SCHEDULER->schedule(
        std::make_unique<CountdownTask>(*this), countdownSec * 1000);
}


void ArenaVsMode::startReadyCountdown()
{
    sne::base::Future::Ref countdownTask = countdownTask_.lock();
    if (countdownTask.get()) {
        return;
    }

    sne::base::Future::Ref countdownWaitTask = countdownWaitingTask_.lock();
    if (countdownWaitTask.get()) {
        return;
    }

    countdownWaitingTask_ = TASK_SCHEDULER->schedule(
        std::make_unique<CountdownWaitingTask>(*this), 20 * 1000);
}


bool ArenaVsMode::shouldDestroy() const 
{
    return (getTeamCount(blueTeam) + getTeamCount(redTeam)) == 0;
}


bool ArenaVsMode::canJoin() const
{
    if (matchModeInfo_.isPlaying()) {
        const sec_t now = getTime();
        const sec_t endTime = matchModeInfo_.startTime_ + getPlayingTime();
        if (now >= endTime) {
            return false;
        }
        const sec_t leftTime = endTime - now;
        const sec_t joinableTime = getPlayingTime() / (100 / perJoinTime);
        if (leftTime < joinableTime) {
            return false;
        }
    }
    else if (matchModeInfo_.isStopPlaying()) {
        return false;
    }
    return getMaxChallenger() > (getTeamCount(blueTeam) + getTeamCount(redTeam));
}


bool ArenaVsMode::isSameTeam(ObjectId sourceId, ObjectId targetId) const
{
    return getPlayingTeamType(sourceId) == getPlayingTeamType(targetId);
}


bool ArenaVsMode::isEnoughValidPlayTime() const
{
    if (! matchModeInfo_.isWating()) {
        const sec_t now = getTime();
        const sec_t playTime = now - matchModeInfo_.startTime_;
        const sec_t playeInvalidTime = getPlayingTime() / (100 / perValidPlayTime);
        if (playTime > playeInvalidTime) {
            return true;
        }
    }
    return false;
}

uint32_t ArenaVsMode::getTeamCount(TeamType teamType) const 
{
    if (! isValid(teamType)) {
        return 0;
    }
    return uint32_t(teams_[teamType].size() + waitingTeams_[teamType].size());
}


ArenaVsMode::TeamType ArenaVsMode::getWaitingTeamType(ObjectId playerId) const
{
    ChallengerIds::const_iterator pos = waitingTeams_[blueTeam].find(playerId);
    if (pos != waitingTeams_[blueTeam].end()) {
        return blueTeam;
    }

    pos = waitingTeams_[redTeam].find(playerId);
    if (pos != waitingTeams_[redTeam].end()) {
        return redTeam;
    }
    return noneTeam;
}


ArenaVsMode::TeamType ArenaVsMode::getPlayingTeamType(ObjectId playerId) const
{
    ArenaPlayerInfo info(playerId);
    ArenaPlayerInfos::const_iterator pos = teams_[blueTeam].find(info);
    if (pos != teams_[blueTeam].end()) {
        return blueTeam;
    }

    pos = teams_[redTeam].find(info);
    if (pos != teams_[redTeam].end()) {
        return redTeam;
    }
    return noneTeam;
}


ArenaPlayerInfo* ArenaVsMode::getArenaPlayerInfo(TeamType teamType, ObjectId playerId)
{
    assert(isValid(teamType));
    ArenaPlayerInfos& members = teams_[teamType];
    const ArenaPlayerInfos::iterator pos = members.find(ArenaPlayerInfo(playerId));
    if (pos != members.end()) {
        return const_cast<ArenaPlayerInfo*>(&(*pos));
    }
    return nullptr;
}


void ArenaVsMode::notifyTeamEvent(go::EntityEvent::Ref event, TeamType teamType)
{
    for (const ArenaPlayerInfos::value_type& value : teams_[teamType]) {
        const ArenaPlayerInfo& info = value;
        go::Entity* player = getPlayer(info.playerId_);
        if (! player) {
            continue;
        }
        if (player->isValid()) {
            (void)TASK_SCHEDULER->schedule(std::make_unique<go::EventCallTask>(*player, event));
        }
    }
}


void ArenaVsMode::cancelCountdownWaitingTask()
{
    sne::base::Future::Ref countdownWaitingTask = countdownWaitingTask_.lock();
    if (countdownWaitingTask.get()) {
        countdownWaitingTask->cancel();
        countdownWaitingTask_.reset();
    }
}


void ArenaVsMode::cancelCountdownTask()
{
    sne::base::Future::Ref countdownTask = countdownTask_.lock();
    if (countdownTask.get()) {
        countdownTask->cancel();
        countdownTask_.reset();
    }
}

}} // namespace gideon { namespace zoneserver {
