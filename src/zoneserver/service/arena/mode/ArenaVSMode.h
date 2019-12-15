#pragma once

#include "ArenaMode.h"
#include "../../party/Party.h"
#include <sne/base/concurrent/Future.h>


namespace gideon { namespace zoneserver {

/**
 * @class ArenaVsMode
 */
class ArenaVsMode : public ArenaMode
{
public:
    enum TeamType
    {
        noneTeam = - 1,
        blueTeam = 0,
        redTeam = 1,
        teamCount = 2,
    };

    bool isValid(TeamType tt) const {
        return (noneTeam < tt) && (tt < teamCount);
    }

    typedef sne::core::Set<ObjectId> ChallengerIds;
    typedef std::array<ChallengerIds, teamCount> WaitTeams;
    typedef std::array<ArenaPlayerInfos, teamCount> Teams;
    typedef std::array<PartyPtr, teamCount> Partyies;
    typedef std::array<uint32_t, teamCount> DeadCounts;
    typedef std::array<uint32_t, teamCount> KillCounts;
    typedef sne::core::HashMap<ObjectId, uint32_t> MoveCounts;

public:
    ArenaVsMode(ArenaModeType arenaModeType, uint32_t minChallenger,
        uint32_t maxChallenger, sec_t playingTime, WorldMap& worldMap);
    virtual ~ArenaVsMode();

    void waitCountdownCompleted();
    void startPlaying();

protected:
    virtual bool initialize();
    virtual void finalize();

    virtual bool arrangePosition(ObjectPosition& position, go::Entity& player, MapCode preGlobalMapCode);
    virtual bool addWaitMatcher(ObjectId matcherId);
    virtual bool addWaitMatchers(const ObjectIdSet& matcherIds);
    virtual void removeWaitMatcher(ObjectId matcherId);
    virtual void leave(ObjectId playerId);
    virtual void readyToPlay(go::Entity& player);
    virtual void killed(go::Entity& killer, go::Entity& victim);
    virtual void say(ObjectId playerId, const ChatMessage& message);
    virtual void setWaypoint(ObjectId playerId, const Waypoint& waypoint);
    virtual void notifyMoved(ObjectId playerId, float32_t x, float32_t y);

protected:
    virtual void rewardTeam(TeamType teamType, ArenaResultType resultType) = 0;

protected:
    void addTeamPlayer(go::Entity& player, const MapRegion& mapRegion,
        TeamType teamType, MapCode preGlobalMapCode);
    void removeTeamPlayer(TeamType teamType, ObjectId playerId);
    void startCountdown();
    void startReadyCountdown();

protected:
    virtual bool shouldDestroy() const;
    virtual bool canJoin() const;
    virtual bool isSameTeam(ObjectId sourceId, ObjectId targetId) const;
    virtual bool isEnoughValidPlayTime() const;

protected:
    // 대기자 플레이중인 유저포함
    uint32_t getTeamCount(TeamType teamType) const;

    TeamType getWaitingTeamType(ObjectId playerId) const;
    TeamType getPlayingTeamType(ObjectId playerId) const;
    TeamType getRestutType(TeamType teamType) const;

    ArenaPlayerInfo* getArenaPlayerInfo(TeamType teamType, ObjectId playerId);

private:
    void notifyTeamEvent(go::EntityEvent::Ref event, TeamType teamType);    

    void cancelCountdownWaitingTask();
    void cancelCountdownTask();

protected:
    Teams teams_;
    WaitTeams waitingTeams_;
    ArenaMatchModeInfo matchModeInfo_;
    Partyies teamParties_;
    DeadCounts deadCounts_;
    KillCounts killCounts_;
    MoveCounts moveCounts_;

private:
    sne::base::Future::WeakRef countdownTask_;
    sne::base::Future::WeakRef countdownWaitingTask_;
};


}} // namespace gideon { namespace zoneserver {
