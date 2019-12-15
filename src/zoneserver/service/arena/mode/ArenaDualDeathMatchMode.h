#pragma once

#include "ArenaVsMode.h"


namespace gideon { namespace zoneserver {

/**
 * @class ArenaDualDeathMatchMode
 */
class ArenaDualDeathMatchMode : public ArenaVsMode
{   
public:
    ArenaDualDeathMatchMode(ArenaModeType arenaModeType, uint32_t minChallenger,
        uint32_t maxChallenger, sec_t playingTime, WorldMap& worldMap);
   
private:
    virtual void killed(go::Entity& killer, go::Entity& victim);
    virtual void revivePlayers();
    virtual bool shouldStop() const;
    virtual void stop();
    virtual void rewardTeam(TeamType teamType, ArenaResultType resultType);

private:
    bool isExistKillPlayer_;
};


}} // namespace gideon { namespace zoneserver {