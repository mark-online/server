#pragma once

#include "Movement.h"
#include "../time/GameTimer.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class ChaseMovement
 */
template <typename T>
class ChaseMovement : public MovementMedium<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<ChaseMovement<T> >
{
public:
    ChaseMovement(T& owner) :
        MovementMedium<T>(owner) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);

private:
    void setDestination();

private:
    GameTimeTracker nextCheckTime_;
};

}} // namespace gideon { namespace zoneserver {