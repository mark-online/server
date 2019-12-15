#include "ZoneServerPCH.h"
#include "ArenaTeamDeathMatchMode.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../controller/callback/ArenaCallback.h"
#include "../../../controller/EntityController.h"


namespace gideon { namespace zoneserver {


ArenaTeamDeathMatchMode::ArenaTeamDeathMatchMode(ArenaModeType arenaModeType,
    uint32_t minChallenger, uint32_t maxChallenger, sec_t playingTime, WorldMap& worldMap) :
    ArenaVsMode(arenaModeType, minChallenger, maxChallenger, playingTime, worldMap)
{

}


bool ArenaTeamDeathMatchMode::shouldStop() const
{
    if (! matchModeInfo_.isPlaying()) {
        return false;
    }
    const sec_t now = getTime();
    const sec_t endTime = matchModeInfo_.startTime_ + getPlayingTime();
    if (now > endTime) {
        return true;
    }
    return getTeamCount(blueTeam) == 0 || getTeamCount(redTeam) == 0;
}


void ArenaTeamDeathMatchMode::stop()
{
    ArenaResultType blueTeamResult = artCancel;
    ArenaResultType redTeamResult = artCancel;
    if (isEnoughValidPlayTime()) {
        if (! teams_[blueTeam].empty() && ! teams_[redTeam].empty()) {
            if (matchModeInfo_.isDraw()) {
                if (killCounts_[blueTeam] == killCounts_[redTeam]) {
                    if (deadCounts_[blueTeam] == deadCounts_[redTeam]) {
                        blueTeamResult = artDraw;
                        redTeamResult = artDraw;
                    }
                    else {
                        blueTeamResult = deadCounts_[blueTeam] < deadCounts_[redTeam] ? artWin : artLose;
                        redTeamResult = (blueTeamResult == artWin) ? artLose : artWin;
                    }
                }
                else {
                    blueTeamResult = killCounts_[blueTeam] > killCounts_[redTeam] ? artWin : artLose;
                    redTeamResult = (blueTeamResult == artWin) ? artLose : artWin;
                }
            }
            else {
                blueTeamResult = matchModeInfo_.getRestutType(attBlue);
                redTeamResult = matchModeInfo_.getRestutType(attRed);
            }
        }
        else {
            blueTeamResult = teams_[redTeam].empty() ? 
                artWin : matchModeInfo_.getRestutType(attBlue);
            redTeamResult = teams_[blueTeam].empty() ? 
                artWin : matchModeInfo_.getRestutType(attRed);
        }
    }

    matchModeInfo_.state_ = asStop;
    setArenaState(matchModeInfo_.state_);

    rewardTeam(blueTeam, blueTeamResult);
    rewardTeam(redTeam, redTeamResult);    
}



void ArenaTeamDeathMatchMode::rewardTeam(TeamType teamType, ArenaResultType resultType)
{    
    ArenaPoint rewardPoint = getRewardMatchPoint(resultType);
    for (const ArenaPlayerInfos::value_type& value : teams_[teamType]) {
        const ArenaPlayerInfo& info = value;
        go::Entity* player = getPlayer(info.playerId_);
        if (! player) {
            continue;
        }
        gc::ArenaCallback* callback = player->getController().queryArenaCallback();
        if (callback) {
            callback->arenaStopped(getArenaModeType(), rewardPoint, rewardPoint, resultType);
        }
    }
}

}} // namespace gideon { namespace zoneserver {