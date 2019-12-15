#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/Time.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

/**
 * @class GameTimeTracker
 */
class GameTimeTracker
{
public:
    GameTimeTracker(GameTime expiry = 0)
        : expiryTime_(expiry) {}

    void update(int32_t diff) {
        expiryTime_ -= diff;
    }

    bool isPassed() const {
        return expiryTime_ <= 0;
    }

    void reset(GameTime interval) {
        expiryTime_ = interval;
    }

    int32_t getExpiry() const {
        return expiryTime_;
    }

private:
    int32_t expiryTime_;
};


/**
 * @class GameTimerSource
 */
class GameTimerSource
{
public:
    virtual ~GameTimerSource() {}

    virtual GameTime msec() const = 0;
};


/**
 * @class GameTimer
 * 동기화를 위한 게임 시간
 */
class ZoneServer_Export GameTimer : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(GameTimer);

public:
    GameTimer(std::unique_ptr<GameTimerSource> source = nullptr);

    GameTime msec() const {
        return source_->msec();
    }

    GameTime getElapsedTime(GameTime oldTime) const {
        return getGameTimeDiff(oldTime, msec());
    }

private:
    std::unique_ptr<GameTimerSource> source_;
};

}} // namespace gideon { namespace zoneserver {

#define GAME_TIMER gideon::zoneserver::GameTimer::instance()
