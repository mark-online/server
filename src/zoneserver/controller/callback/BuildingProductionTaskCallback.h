#pragma once

#include <gideon/cs/shared/data/BuildingInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class BuildingProductionTaskCallback
 */
class BuildingProductionTaskCallback
{
public:
    virtual ~BuildingProductionTaskCallback() {}

public:
    virtual void buildingSelectRecipeProductionAdded(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& taskInfo) = 0;
    virtual void buildingSelectRecipeProductionUpdated(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& taskInfo) = 0;
    virtual void buildingSelectRecipeProductionRemoved(const GameObjectInfo& buildingInfo, RecipeCode recipeCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
