#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/RecipeTable.h>

namespace gideon { namespace servertest {
		
/**
* @class MockRecipeTable
*/
class GIDEON_SERVER_API MockRecipeTable :public gideon::datatable::RecipeTable
{
public:
	MockRecipeTable() {
		fillRecipes();
	}
	virtual ~MockRecipeTable() {
		destroyRecipes();
	}

public:
	virtual const gideon::datatable::RecipeTemplate* getRecipe(RecipeCode code) const override;

	virtual const RecipeMap& getRecipeMap() const override {
		return recipeMap_;
	}

	virtual const std::string& getLastError() const override {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const override {
		return true;
	}

    virtual const gideon::datatable::RecipeTemplate* getRecipeOf(DataCode itemCode) const override {
        itemCode;
        return nullptr;
    }

    virtual RecipeTable::Recipes getRecipesBy(DataCode itemCode) const override {
        itemCode;
        return RecipeTable::Recipes();
    }

private:
	void fillRecipes() {		
		fillRecipe1();
		fillRecipe2();
		fillRecipe3();
	}

	void destroyRecipes();

	void fillRecipe1();
	void fillRecipe2();
	void fillRecipe3();

private:
    typedef sne::core::Vector<gdt::recipe_t*> Recipes;
    Recipes recipes_;
	RecipeMap recipeMap_;
	RecipeGradeCodes gradeCodes_;
};



}} // namespace gideon { namespace servertest {
