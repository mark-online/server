#pragma once

#include "Movement.h"
#include "../time/GameTimer.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/3d/3d.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

/**
 * @class FleeingMovement
 */
template <typename T>
class FleeingMovement : public MovementMedium<T>
{
public:
    FleeingMovement(T& owner, go::Entity* fright) :
        MovementMedium<T>(owner),
        fright_(fright) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);

private:
    void setDestination();

private:
    go::Entity* fright_;

    Vector2 moveDirection_;
    GameTimeTracker nextCheckTime_;
};


/**
 * @class TimedFleeingMovement
 */
template <typename T>
class TimedFleeingMovement : public FleeingMovement<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<TimedFleeingMovement<T> >
{
public:
    TimedFleeingMovement(T& owner, go::Entity* fright, GameTime duration) :
        FleeingMovement<T>(owner, fright),
        duration_(duration) {}

public:
    virtual void initialize();
    //virtual void finalize();

    virtual bool update(GameTime diff);

private:
    GameTime duration_;
    GameTimeTracker totalFleeTime_;
};

}} // namespace gideon { namespace zoneserver {
