#include "ZoneServerPCH.h"
#include "CastingTimer.h"
#include "../../service/time/GameTimer.h"


namespace gideon { namespace zoneserver {

bool CastingTimer::isCasting(DataCode dataCode)
{
    GameTime castingTime = getCastingTime(dataCode);   
    if (castingTime == 0) {
        return false;
    }

    GameTime now = GAME_TIMER->msec();
    return now > castingTime;
}


void CastingTimer::startCasting(DataCode dataCode, GameTime castingTime)
{
    GameTime now = GAME_TIMER->msec();
    timeMap_.emplace(dataCode, now + castingTime);
}


void CastingTimer::endCasting(DataCode dataCode)
{
    removeTime(dataCode);
}


void CastingTimer::cancelCasting(DataCode dataCode)
{
    removeTime(dataCode);
}


GameTime CastingTimer::getCastingTime(DataCode dataCode) const
{
    CastingTimeInfoMap::const_iterator pos = timeMap_.find(dataCode);
    if (pos != timeMap_.end()) {
        return (*pos).second;
    }
    return 0;
}

void CastingTimer::removeTime(DataCode dataCode)
{
    timeMap_.erase(dataCode);
}

}} // namespace gideon { namespace zoneserver {

