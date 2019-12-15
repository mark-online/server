#pragma once

#include "../../../zoneserver_export.h"
#include "EntityAllocator.h"

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class TreasureAllocator
 * Treasure Factory
 */
class ZoneServer_Export TreasureAllocator : public EntityAllocator
{
public:
    virtual go::Entity* malloc();
    virtual void free(go::Entity* entity);
};

}}} // namespace gideon { namespace zoneserver { namespace go {
