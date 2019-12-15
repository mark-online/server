#pragma once

#include "Movement.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class IdleMovement
 */
template <typename T>
class IdleMovement : public MovementMedium<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<IdleMovement<T> >
{
public:
    IdleMovement(T& owner) :
        MovementMedium<T>(owner) {}

public:
    virtual void initialize() {}
    virtual void finalize() {}

    virtual bool update(GameTime /*diff*/) {
        return false;
    }
};

}} // namespace gideon { namespace zoneserver {