#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockRecipeTable.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include "esut/Random.h"


namespace gideon { namespace servertest {

const gideon::datatable::RecipeTemplate* MockRecipeTable::getRecipe(RecipeCode code) const
{
    const RecipeMap::const_iterator pos = recipeMap_.find(code);
    if (pos != recipeMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockRecipeTable::destroyRecipes()
{
    recipeMap_.clear();
    for (gdt::recipe_t* recipe : recipes_) {
        delete recipe;
    }
}


void MockRecipeTable::fillRecipe1()
{
 //   gideon::BaseItemInfo need1(refinedShellComponent, 3);
 //   gideon::BaseItemInfo need2(refinedVenomComponent, 3);
 //   gideon::BaseItemInfos needs;
 //   needs.push_back(need1);
 //   needs.push_back(need2);
 //   gdt::recipe_t* recipeT = new gdt::recipe_t;
 //   recipeT->looting_delay(0);
 //   recipeT->looting_pct(0);
 //   recipeT->stack_count(255);
 //   recipes_.push_back(recipeT);
 //   gideon::datatable::RecipeTemplate recipe(*recipeT, shabbyHelmetRecipeCode, 2000, lanceEquipCode, 1, needs);
 //   recipeMap_.emplace(shabbyHelmetRecipeCode, recipe);
	//gradeCodes_[rgShabby].push_back(shabbyHelmetRecipeCode);

}


void MockRecipeTable::fillRecipe2()
{
 //   gideon::BaseItemInfo need1(refinedVenomComponent, 3);
 //   gideon::BaseItemInfo need2(refinedLeatherComponent, 3);
 //   gideon::BaseItemInfos needs;
 //   needs.push_back(need1);
 //   needs.push_back(need2);
 //   gdt::recipe_t* recipeT = new gdt::recipe_t;
 //   recipeT->looting_delay(0);
 //   recipeT->looting_pct(0);
 //   recipeT->stack_count(255);
 //   recipes_.push_back(recipeT);
 //   gideon::datatable::RecipeTemplate recipe(*recipeT, normalHelmetRecipeCode, 2000, otherHelmetEquipCode, 1, needs);
 //   recipeMap_.emplace(normalHelmetRecipeCode, recipe);
	//gradeCodes_[rgNormal].push_back(normalHelmetRecipeCode);
}


void MockRecipeTable::fillRecipe3()
{
 //   gideon::BaseItemInfo need1(refinedLeatherComponent, 3);
 //   gideon::BaseItemInfo need2(refinedQuintessenceComponent, 3);
 //   gideon::BaseItemInfos needs;
 //   needs.push_back(need1);
 //   needs.push_back(need2);
 //   gdt::recipe_t* recipeT = new gdt::recipe_t;
 //   recipeT->looting_delay(0);
 //   recipeT->looting_pct(0);
 //   recipeT->stack_count(255);
 //   recipes_.push_back(recipeT);
 //   gideon::datatable::RecipeTemplate recipe(*recipeT, epicHelmetRecipe, 2000, shoesEquipCode, 1, needs);
 //   recipeMap_.emplace(epicHelmetRecipe, recipe);
	//gradeCodes_[rgEpic].push_back(epicHelmetRecipe);
}

}} // namespace gideon { namespace servertest {
