#pragma once

#include "Movement.h"
#include "../time/GameTimer.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

/**
 * @class SeekAssistMovement
 */
template <typename T>
class SeekAssistMovement : public MovementMedium<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<SeekAssistMovement<T> >
{
public:
    SeekAssistMovement(T& owner, go::Entity& assist) :
        MovementMedium<T>(owner),
        assist_(assist) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);

private:
    void setDestination();

private:
    go::Entity& assist_;
    GameTimeTracker nextCheckTime_;
};

}} // namespace gideon { namespace zoneserver {