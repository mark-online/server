#pragma once

#include "Movement.h"
#include "../time/GameTimer.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class WanderMovement
 */
template <typename T>
class WanderMovement : public MovementMedium<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<WanderMovement<T> >
{
public:
    WanderMovement(T& owner) :
        MovementMedium<T>(owner) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);

private:
    void setNextMoveTime();
    void setDestination();
    bool checkArrivedToDestination();

private:
    GameTimeTracker nextMoveTime_;
    Position homePosition_;
    float32_t walkDistance_;
    bool isWalkingToNextDestination_;
    ObjectPosition destination_;
};

}} // namespace gideon { namespace zoneserver {