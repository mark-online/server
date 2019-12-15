#pragma once

#include <gideon/cs/shared/data/Code.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CraftCallback
 */
class CraftCallback
{
public:
    virtual ~CraftCallback() {}

public:
    virtual void npcCraftComleted(DataCode recipeCode) = 0;
    virtual void playerCraftComleted(ObjectId recipeId) = 0;
    virtual void itemReprocessed(ObjectId itemId) = 0;
    virtual void equipItemUpgraded(ObjectId equipItemId) = 0;
    virtual void equipItemEnchanted(ObjectId equipItemId, ObjectId gemItemId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
