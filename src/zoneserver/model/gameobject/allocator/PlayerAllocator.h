#pragma once

#include "../../../zoneserver_export.h"
#include "EntityAllocator.h"

namespace gideon { namespace zoneserver { namespace go {

class Player;

/**
 * @class PlayerAllocator
 * Player Factory
 */
class ZoneServer_Export PlayerAllocator : public EntityAllocator
{
public:
    virtual go::Entity* malloc();
    virtual void free(go::Entity* entity);
};

}}} // namespace gideon { namespace zoneserver { namespace go {
