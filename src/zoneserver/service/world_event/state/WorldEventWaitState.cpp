#include "ZoneServerPCH.h"
#include "WorldEventWaitState.h"
#include "../WorldEventPhase.h"
#include "../callback/WorldEventScoreCallback.h"
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/datatable/WorldEventTable.h>

namespace gideon { namespace zoneserver {



WorldEventWaitState::WorldEventWaitState(WorldEventScoreCallback& callback,
    const gdt::world_event_t& eventTemplate) :
    callback_(callback),
    eventOpenType_(toWorldEventOpenType(eventTemplate.event_open_type())),
    openParam1_(eventTemplate.open_param_1()),
    openParam2_(eventTemplate.open_param_2()),
    randomOpenTime_(0)
{
    reInitialize();
}

    
bool WorldEventWaitState::shouldNextState() const
{
    sec_t now = getTime();
    struct tm* nowInfo = _localtime64(&now);        
    if (! nowInfo) {
        return false;
    }

    if (eventOpenType_ == weotHour) {
        if (openParam1_ == 0) {
            return nowInfo->tm_min == 0;
        }
        return (static_cast<uint32_t>(nowInfo->tm_min) % openParam1_) == 0;        
    }
    else if (eventOpenType_ == weotDay) {
        const uint32_t hour = nowInfo->tm_hour == 0 ? 24 : 0;
        bool isHourMatch = (hour % openParam1_) == 0;
        if (isHourMatch) {
            if (openParam2_ == 0) {
                return nowInfo->tm_min == 0;
            }
            return static_cast<uint32_t>(nowInfo->tm_min) == openParam2_;
        }
    }
    else if (eventOpenType_ == weotDate) {
        bool isDayMatch = (nowInfo->tm_mday% openParam1_) == 0;
        if (isDayMatch && static_cast<uint32_t>(nowInfo->tm_hour) == openParam2_) {
            return true;
        }
    }
    else if (eventOpenType_ == weotWeek) {
        bool isDayMatch = (nowInfo->tm_wday% openParam1_) == 0;
        if (isDayMatch && static_cast<uint32_t>(nowInfo->tm_hour) == openParam2_) {
            return true;
        }
    }
    else if (eventOpenType_ == weotRandom) {
        return randomOpenTime_ < now;         
    }
    return false;
}


void WorldEventWaitState::changeNextState()
{
    callback_.resetScore();
}


void WorldEventWaitState::reInitialize()
{
    if (eventOpenType_ == weotRandom) {
        const uint32_t startRandomValue = openParam1_ * 60;
        const uint32_t endRandomValue = openParam2_ * 60;
        randomOpenTime_ = getTime() + (esut::random(startRandomValue, endRandomValue));
    }
}

}} // namespace gideon { namespace zoneserver {
