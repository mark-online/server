#include "ZoneServerPCH.h"
#include "WorldClock.h"
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver {

WorldClock::WorldClock(ZoneId zoneId, WorldTime worldTime) :
    zoneId_(zoneId),
    worldTime_(worldTime)
{
    prevHour_ = getHour(worldTime_);

    start();
}


void WorldClock::start()
{
    const uint16_t worldTimeResolution =
        GIDEON_PROPERTY_TABLE->getPropertyValue<uint16_t>(L"world_time_resolution");

    clockTask_ =
        TASK_SCHEDULER->schedule(*this, worldTimeResolution, worldTimeResolution);
}


void WorldClock::stop()
{
    sne::base::Future::Ref task = clockTask_.lock();
    if (! task.get()) {
        return;
    }

    task->cancel();
    clockTask_.reset();
}


void WorldClock::updateWorldTime()
{
    // TODO: 나중에 활성화할 것!!!
    //sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    //(void)db->asyncUpdateWorldTime(zoneId_, worldTime_);
}

// = sne::base::Runnable overriding

void WorldClock::run()
{
    increaseWorldTime(worldTime_);

    const int hour = getHour(worldTime_);
    if (hour != prevHour_) {
        prevHour_ = hour;

        updateWorldTime();
    }
}

}} // namespace gideon { namespace zoneserver {
