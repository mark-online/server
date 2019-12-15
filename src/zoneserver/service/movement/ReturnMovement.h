#pragma once

#include "Movement.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class ReturnMovement
 */
template <typename T>
class ReturnMovement : public MovementMedium<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<ReturnMovement<T> >
{
public:
    ReturnMovement(T& owner) :
        MovementMedium<T>(owner) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);
};

}} // namespace gideon { namespace zoneserver {