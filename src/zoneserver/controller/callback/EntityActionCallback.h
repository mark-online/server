#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class EntityActionCallback
 */
class EntityActionCallback
{
public:
    virtual ~EntityActionCallback() {}

    virtual void actionPlayed(const GameObjectInfo& entityInfo, uint32_t actionCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
