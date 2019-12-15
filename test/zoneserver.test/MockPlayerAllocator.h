#pragma once

#include "ZoneServer/model/gameobject/allocator/PlayerAllocator.h"

/**
 * @class MockPlayerAllocator
 */
class MockPlayerAllocator : public gideon::zoneserver::go::PlayerAllocator
{
public:
    virtual gideon::zoneserver::go::Entity* malloc();
    virtual void free(gideon::zoneserver::go::Entity* entity);
};