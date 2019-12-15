#pragma once

#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class MovementCallback
 */
class MovementCallback
{
public:
    virtual ~MovementCallback() {}

public:
    virtual void entityGoneToward(const GameObjectInfo& entityInfo,
        const ObjectPosition& position) = 0;

    virtual void entityMoved(const GameObjectInfo& entityInfo,
        const ObjectPosition& position) = 0;

    virtual void entityStopped(const GameObjectInfo& entityInfo,
        const ObjectPosition& position) = 0;

    virtual void entityTeleported(const GameObjectInfo& entityInfo,
        const ObjectPosition& position) = 0;

    virtual void entityTurned(const GameObjectInfo& entityInfo,
        Heading heading) = 0;

    virtual void entityJumped(const GameObjectInfo& entityInfo,
        const ObjectPosition& position) = 0;

    virtual void entityRun(const GameObjectInfo& entityInfo) = 0;

    virtual void entityWalked(const GameObjectInfo& entityInfo) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
