#pragma once

#include "ZoneServer/service/time/GameTimer.h"

using namespace gideon;

/**
 * @class MockGameTimerSource
 */
class MockGameTimerSource :
    public zoneserver::GameTimerSource
{
public:
    MockGameTimerSource() :
        currentGameTime_(0) {}

    void set(GameTime gt) {
        currentGameTime_ = gt;
    }

private:
    virtual GameTime msec() const {
        return currentGameTime_;
    }

private:
    GameTime currentGameTime_;
};
