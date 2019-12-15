#pragma once

#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

/***
 * @class CastEventCallback
 ***/
class CastEventCallback
{
public:
    virtual ErrorCode castingCompleted(go::Entity* target, Position targetPosition,
        float32_t distanceToTargetSq) = 0;
    virtual void casted(go::Entity* target, const Position& targetPosition, bool isTargetDeffence) = 0;
};


}} // namespace gideon { namespace zoneserver {