#pragma once

#include "Movement.h"
#include "../time/GameTimer.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class ConfusedMovement
 */
template <typename T>
class ConfusedMovement : public MovementMedium<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<ConfusedMovement<T> >
{
public:
    ConfusedMovement(T& owner, GameTime duration) :
        MovementMedium<T>(owner),
        duration_(duration) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);

private:
    void reset();

    void setDestination();

private:
    const GameTime duration_;
    GameTimeTracker totalMoveTime_;
    GameTimeTracker restingTime_;
    Position originalPosition_;
    int32_t nextMoveIndex_;
};

}} // namespace gideon { namespace zoneserver {