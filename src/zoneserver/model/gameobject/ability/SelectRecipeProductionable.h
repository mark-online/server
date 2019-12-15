#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/RecipeInfo.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class SelectRecipeProductionable
 * 레시피를 이용해서 물건을 제작할수 있다
 */
class SelectRecipeProductionable
{
public:
	virtual ErrorCode startRecipeProduction(go::Entity& player, RecipeCode recipeCode, uint8_t count) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
