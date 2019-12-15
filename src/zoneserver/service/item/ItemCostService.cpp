#include "ZoneServerPCH.h"
#include "ItemCostService.h"
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/GemTable.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/VehicleTable.h>
#include <gideon/cs/datatable/GliderTable.h>
#include <gideon/cs/datatable/HarnessTable.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/FunctionTable.h>

namespace gideon { namespace zoneserver {

namespace {

template <typename T>
inline void fillSellItemCost(CostType& costType, uint32_t& cost, const T& itemTemplate) 
{
	costType = static_cast<CostType>(static_cast<uint8_t>(itemTemplate.sell_cost_type()));
	cost = itemTemplate.sell_cost();
}

template <typename T>
inline void fillDefaultBuyItemCost(CostType& costType, uint32_t& cost, const T& itemTemplate) 
{
	costType = static_cast<CostType>(static_cast<uint8_t>(itemTemplate.buy_cost_type_1()));
	cost = itemTemplate.buy_cost_1();
}

template <typename T>
inline void fillBuyItemCost(CostType& costType, uint32_t& cost, const T& itemTemplate) 
{
	costType = static_cast<CostType>(static_cast<uint8_t>(itemTemplate.buy_cost_type_2()));
	cost = itemTemplate.buy_cost_2();
}

} // namespace {

ErrorCode ItemCostService::getSellItemCost(CostType& costType, uint32_t& cost, DataCode itemCode)
{
	costType = ctNone;
	cost = 0;
	CodeType ct = getCodeType(itemCode);
	if (isAccessoryType(ct)) {
		const gdt::accessory_t*  accessory = ACCESSORY_TABLE->getAccessory(itemCode);
		if (! accessory) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *accessory);
	}
	else if (isGemType(ct)) {
		const gdt::gem_t*  gem = GEM_TABLE->getGem(itemCode);
		if (! gem) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *gem);
	}
	else if (isFragmentType(ct)) {
		const gdt::fragment_t*  fragment = FRAGMENT_TABLE->getFragment(itemCode);
		if (! fragment) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *fragment);
	}
	else if (isEquipmentType(ct)) {
		const gdt::equip_t*  equip = EQUIP_TABLE->getEquip(itemCode);
		if (! equip) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *equip);
	}
	else if (isRecipeType(ct)) {
		const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(itemCode);
		if (! recipe) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, recipe->getInfo());
	}
	else if (isBuildingType(ct)) {
		const datatable::BuildingTemplate* building = BUILDING_TABLE->getBuildingTemplate(itemCode);
		if (! building) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, building->getBuildingInfo());
	}
	else if (isAnchorType(ct)) {
		const datatable::AnchorTemplate* anchor = ANCHOR_TABLE->getAnchorTemplate(itemCode);
		if (! anchor) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, anchor->getAnchorInfo());
	}
	else if (isVehicleType(ct)) {
		const gdt::vehicle_t* vehicle = VEHICLE_TABLE->getVehicle(itemCode);
		if (! vehicle) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *vehicle);
	}
	else if (isGliderType(ct)) {
		const gdt::glider_t* glider = GLIDER_TABLE->getGlider(itemCode);
		if (! glider) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *glider);
	}
	else if (isHarnessType(ct)) {
		const gdt::harness_t*  harness = HARNESS_TABLE->getHarness(itemCode);
		if (! harness) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *harness);
	}
    else if (isElementType(ct)) {
		const gdt::element_t* element = ELEMENT_TABLE->getElement(itemCode);
		if (! element) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *element);
    }
    else if (isFunctionType(ct)) {
        const gdt::function_t* function = FUNCTION_TABLE->getFunction(itemCode);
		if (! function) {
			return ecStoreNotSellItem;
		}
		fillSellItemCost(costType, cost, *function);
    }
	else {
		return ecStoreNotSellItem;
	}

	return ecOk;
}


ErrorCode ItemCostService::getDefaultBuyItemCost(CostType& costType, uint32_t& cost, DataCode itemCode)
{
	costType = ctNone;
	cost = 0;
	CodeType ct = getCodeType(itemCode);
	if (isAccessoryType(ct)) {
		const gdt::accessory_t*  accessory = ACCESSORY_TABLE->getAccessory(itemCode);
		if (! accessory) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *accessory);
	}
	else if (isGemType(ct)) {
		const gdt::gem_t*  gem = GEM_TABLE->getGem(itemCode);
		if (! gem) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *gem);
	}
	else if (isElementType(ct)) {
		const gdt::element_t* element = ELEMENT_TABLE->getElement(itemCode);
		if (! element) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *element);
	}
	else if (isFragmentType(ct)) {
		const gdt::fragment_t*  fragment = FRAGMENT_TABLE->getFragment(itemCode);
		if (! fragment) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *fragment);
	}
	else if (isEquipmentType(ct)) {
		const gdt::equip_t*  equip = EQUIP_TABLE->getEquip(itemCode);
		if (! equip) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *equip);
	}
	else if (isRecipeType(ct)) {
		const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(itemCode);
		if (! recipe) {
			return ecStoreNotSellItem;
		}
		fillDefaultBuyItemCost(costType, cost, recipe->getInfo());
	}
	else if (isBuildingType(ct)) {
		const datatable::BuildingTemplate* building = BUILDING_TABLE->getBuildingTemplate(itemCode);
		if (! building) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, building->getBuildingInfo());
	}
	else if (isAnchorType(ct)) {
		const datatable::AnchorTemplate* anchor = ANCHOR_TABLE->getAnchorTemplate(itemCode);
		if (! anchor) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, anchor->getAnchorInfo());
	}
	else if (isVehicleType(ct)) {
		const gdt::vehicle_t* vehicle = VEHICLE_TABLE->getVehicle(itemCode);
		if (! vehicle) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *vehicle);
	}
	else if (isGliderType(ct)) {
		const gdt::glider_t* glider = GLIDER_TABLE->getGlider(itemCode);
		if (! glider) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *glider);
	}
	else if (isHarnessType(ct)) {
		const gdt::harness_t*  harness = HARNESS_TABLE->getHarness(itemCode);
		if (! harness) {
			return ecStoreNotBuyItem;
		}
		fillDefaultBuyItemCost(costType, cost, *harness);
	}
    else if (isFunctionType(ct)) {
        const gdt::function_t* function = FUNCTION_TABLE->getFunction(itemCode);
		if (! function) {
			return ecStoreNotSellItem;
		}
		fillDefaultBuyItemCost(costType, cost, *function);
    }
	else {
		return ecStoreNotBuyItem;
	}
	return ecOk;
}
	

ErrorCode ItemCostService::getBuyItemCost(CostType& costType, uint32_t& cost, DataCode itemCode)
{
	costType = ctNone;
	cost = 0;
	CodeType ct = getCodeType(itemCode);
	if (isAccessoryType(ct)) {
		const gdt::accessory_t*  accessory = ACCESSORY_TABLE->getAccessory(itemCode);
		if (! accessory) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, *accessory);
	}
	else if (isGemType(ct)) {
		const gdt::gem_t*  gem = GEM_TABLE->getGem(itemCode);
		if (! gem) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, *gem);
	}
	else if (isFragmentType(ct)) {
		const gdt::fragment_t*  fragment = FRAGMENT_TABLE->getFragment(itemCode);
		if (! fragment) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, *fragment);
	}
	else if (isEquipmentType(ct)) {
		const gdt::equip_t*  equip = EQUIP_TABLE->getEquip(itemCode);
		if (! equip) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, *equip);
	}
	else if (isRecipeType(ct)) {
		const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(itemCode);
		if (! recipe) {
			return ecStoreNotSellItem;
		}
		fillBuyItemCost(costType, cost, recipe->getInfo());
	}
	else if (isBuildingType(ct)) {
		const datatable::BuildingTemplate* building = BUILDING_TABLE->getBuildingTemplate(itemCode);
		if (! building) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, building->getBuildingInfo());
	}
	else if (isAnchorType(ct)) {
		const datatable::AnchorTemplate* anchor = ANCHOR_TABLE->getAnchorTemplate(itemCode);
		if (! anchor) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, anchor->getAnchorInfo());
	}
	else if (isVehicleType(ct)) {
		const gdt::vehicle_t* vehicle = VEHICLE_TABLE->getVehicle(itemCode);
		if (! vehicle) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, *vehicle);
	}
	else if (isGliderType(ct)) {
		const gdt::glider_t* glider = GLIDER_TABLE->getGlider(itemCode);
		if (! glider) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, *glider);
	}
	else if (isHarnessType(ct)) {
		const gdt::harness_t*  harness = HARNESS_TABLE->getHarness(itemCode);
		if (! harness) {
			return ecStoreNotBuyItem;
		}
		fillBuyItemCost(costType, cost, *harness);
	}
	else {
		return ecStoreNotBuyItem;
	}
	return ecOk;
}

}} // namespace gideon { namespace zoneserver {
