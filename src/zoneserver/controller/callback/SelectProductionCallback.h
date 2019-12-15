#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class SelectProductionCallback
 */
class SelectProductionCallback
{
public:
    virtual ~SelectProductionCallback() {}

public:
    virtual void selectRecipeProductionAdded(RecipeCode recipeCode, uint8_t count, sec_t completeTime) = 0;
    virtual void selectRecipeProductionRemoved(RecipeCode recipeCode) = 0;
    virtual void selectRecipeProductionUpdated(RecipeCode recipeCode, uint8_t count, sec_t completeTime) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
