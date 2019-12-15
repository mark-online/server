#pragma once

#include "Movement.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class PointMovement
 */
template <typename T>
class PointMovement : public MovementMedium<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<PointMovement<T> >
{
public:
    PointMovement(T& owner, const Position& destination) :
        MovementMedium<T>(owner),
        destination_(destination) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);

private:
    const Position destination_;
};

}} // namespace gideon { namespace zoneserver {