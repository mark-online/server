#pragma once

#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/base/concurrent/Runnable.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver {

/**
 * @class WorldClock
 * 월드 시계
 */
class WorldClock : private sne::base::Runnable
{
public:
    WorldClock(ZoneId zoneId, WorldTime worldTime);

    ~WorldClock() {
        stop();
    }

public:
    WorldTime getWorldTime() const {
        return worldTime_;
    }

private:
    void start();
    void stop();

    void updateWorldTime();

private:
    // = sne::base::Runnable overriding
    virtual void run();

private:
    const ZoneId zoneId_;
    WorldTime worldTime_;
    int prevHour_;

    sne::base::Future::WeakRef clockTask_;
};

}} // namespace gideon { namespace zoneserver {
