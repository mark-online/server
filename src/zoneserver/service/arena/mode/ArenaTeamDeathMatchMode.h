#pragma once

#include "ArenaVsMode.h"



namespace gideon { namespace zoneserver {

/**
 * @class ArenaTeamDeathMatchMode
 */
class ArenaTeamDeathMatchMode : public ArenaVsMode
{   
public:
    ArenaTeamDeathMatchMode(ArenaModeType arenaModeType, uint32_t minChallenger,
        uint32_t maxChallenger, sec_t playingTime, WorldMap& worldMap);
  
private:
    virtual bool shouldStop() const;
    virtual void stop();
    virtual void rewardTeam(TeamType teamType, ArenaResultType resultType);
};


}} // namespace gideon { namespace zoneserver {