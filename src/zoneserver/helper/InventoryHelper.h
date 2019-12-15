#pragma once

#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/GemTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/FunctionTable.h>
#include <gideon/cs/datatable/GliderTable.h>
#include <gideon/cs/datatable/VehicleTable.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/BuildingGuardTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/QuestItemTable.h>
#include "AddItemInfo.h"
#include "../service/item/ItemOptionService.h"

namespace gideon { namespace zoneserver {

inline AddItemInfo createAddItemInfoByItemInfo(const ItemInfo& itemInfo)
{
    if (itemInfo.isEquipment()) {
        return AddItemInfo(itemInfo, itemInfo.equipItemInfo_);
    }
    else if (itemInfo.isAccessory()) {
        return AddItemInfo(itemInfo, itemInfo.accessoryItemInfo_);
    }

    return AddItemInfo(itemInfo.itemCode_ , itemInfo.count_);
}


inline AddItemInfo createAddItemInfoByBuyBackItemInfo(const BuyBackItemInfo& itemInfo)
{
    if (itemInfo.isEquipment()) {
        return AddItemInfo(itemInfo, itemInfo.equipItemInfo_);
    }
    else if (itemInfo.isAccessory()) {
        return AddItemInfo(itemInfo, itemInfo.accessoryItemInfo_);
    }

    return AddItemInfo(itemInfo.itemCode_ , itemInfo.count_);
}


inline AddItemInfo createAddItemInfoByBaseItemInfo(const BaseItemInfo& itemInfo)
{
    if (itemInfo.isEquipment()) {
        const gdt::equip_t* equip = EQUIP_TABLE->getEquip(itemInfo.itemCode_);
        if (equip) {
            const EquipItemInfo equipItemInfo(static_cast<uint8_t>(esut::random(equip->min_socket_count(), equip->max_socket_count())),
                ItemOptionService::getItemOptions(equip->item_suffix_id()));
            return AddItemInfo(itemInfo, equipItemInfo);
        }
    }
    else if (itemInfo.isAccessory()) {
        const gdt::accessory_t* accessory = ACCESSORY_TABLE->getAccessory(itemInfo.itemCode_);
        if (accessory) {
            const AccessoryItemInfo accessoryItemInfo(ItemOptionService::getItemOptions(accessory->item_suffix_id()));
            return AddItemInfo(itemInfo, accessoryItemInfo);
        }
        assert(false);
    }
    return AddItemInfo(itemInfo.itemCode_ , itemInfo.count_);
}

inline AddItemInfo createAddItemInfoByItemCode(DataCode itemCode, uint8_t count)
{
    if (isEquipmentType(getCodeType(itemCode))) {
        const gdt::equip_t* equip = EQUIP_TABLE->getEquip(itemCode);
        if (equip) {
            const EquipItemInfo equipItemInfo(static_cast<uint8_t>(esut::random(equip->min_socket_count(), equip->max_socket_count())),
                ItemOptionService::getItemOptions(equip->item_suffix_id()));
            return AddItemInfo(BaseItemInfo(itemCode, 1), equipItemInfo);
        }
        assert(false);
    }
    else if (isAccessoryType(getCodeType(itemCode))) {
        const gdt::accessory_t* accessory = ACCESSORY_TABLE->getAccessory(itemCode);
        if (accessory) {
            const AccessoryItemInfo accessoryItemInfo(ItemOptionService::getItemOptions(accessory->item_suffix_id()));
            return AddItemInfo(BaseItemInfo(itemCode, 1), accessoryItemInfo);
        }
        assert(false);
    }
    return AddItemInfo(itemCode, count);
}

inline uint8_t getStackItemCount(DataCode itemCode)
{
    CodeType codeType = getCodeType(itemCode);
    if (ctRecipe == codeType) {
        const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(itemCode);
        if (recipe) {
            return recipe->getStackCount();
        }
    }
    else if (ctElement == codeType) {
        const gdt::element_t* element = ELEMENT_TABLE->getElement(itemCode);
        if (element) {
            return static_cast<uint8_t>(element->stack_count());
        }
    }
    else if (ctGem == codeType) {
        const gdt::gem_t* gem = GEM_TABLE->getGem(itemCode);
        if (gem) {
            return static_cast<uint8_t>(gem->stack_count());
        }
    }
    else if (ctFragment == codeType) {
        const gdt::fragment_t* fragment = FRAGMENT_TABLE->getFragment(itemCode);
        if (fragment) {
            return static_cast<uint8_t>(fragment->stack_count());
        }
    }
    else if (ctFunction == codeType) {
        const gdt::function_t* function = FUNCTION_TABLE->getFunction(itemCode);
        if (function) {
            return static_cast<uint8_t>(function->stack_count());
        }
    }
    else if (ctAnchor == codeType) {
        const datatable::AnchorTemplate* anchor = ANCHOR_TABLE->getAnchorTemplate(itemCode);
        if (anchor) {
            return anchor->getStackCount();
        }
    }
    else if (ctBuilding == codeType) {
        const datatable::BuildingTemplate* building = BUILDING_TABLE->getBuildingTemplate(itemCode);
        if (building) {
            return building->getStackCount();
        }
    }
    else if (ctEquipment == codeType || ctAccessory == codeType ||
        ctVehicle == codeType || ctGlider == codeType) {
        return 1;
    }
    else if (ctQuestItem == codeType) {
        // TODO: 퀘스트 아이템은 스택 가능??
        return 1;
    }
    else if (ctBuildingGuard == codeType) {
        return 1;
    }
    return 0;
}


inline bool isExistItem(DataCode itemCode)
{
    CodeType codeType = getCodeType(itemCode);
    if (ctRecipe == codeType) {
        return RECIPE_TABLE->getRecipe(itemCode) != nullptr;        
    }
    else if (ctElement == codeType) {
        return ELEMENT_TABLE->getElement(itemCode) != nullptr;
    }
    else if (ctGem == codeType) {
        return GEM_TABLE->getGem(itemCode) != nullptr;        
    }
    else if (ctFragment == codeType) {
        return FRAGMENT_TABLE->getFragment(itemCode) != nullptr;        
    }
    else if (ctFunction == codeType) {
        return FUNCTION_TABLE->getFunction(itemCode)  != nullptr;
    }
    else if (ctAnchor == codeType) {
        return ANCHOR_TABLE->getAnchorTemplate(itemCode) != nullptr;
    }
    else if (ctBuilding == codeType) {
        return BUILDING_TABLE->getBuildingTemplate(itemCode) != nullptr;
    }
    else if (ctEquipment == codeType) {
         return EQUIP_TABLE->getEquip(itemCode) != nullptr;
    }
    else if (ctAccessory == codeType) {
        return ACCESSORY_TABLE->getAccessory(itemCode) != nullptr;
    }
    else if (ctVehicle == codeType) {
        return VEHICLE_TABLE->getVehicle(itemCode) != nullptr;
    }
    else if (ctGlider == codeType) {
        return GLIDER_TABLE->getGlider(itemCode) != nullptr;
    }
    else if (ctQuestItem == codeType) {        
        return QUEST_ITEM_TABLE->getQuestItemTemplate(itemCode) != nullptr;
    }
    else if (ctBuildingGuard == codeType) {
        return BUILDING_TABLE->getBuildingTemplate(itemCode) != nullptr;
    }
    return false;
}



inline GameTime getLootingDelay(LootInvenItemType lootingType, DataCode itemCode)
{
    if (lootingType != liitItem) {
        return 0;
    }

    const CodeType ct = getCodeType(itemCode);
    if (isFragmentType(ct)) {
        const gdt::fragment_t* fragment = FRAGMENT_TABLE->getFragment(itemCode);
        if (fragment != nullptr) {
            return fragment->looting_mil_sec();
        }
    }
    else if (isRecipeType(ct)) {
        const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(itemCode);
        if (recipe) {
            return recipe->getLootingMilSec();
        }
    }
    else if (isEquipmentType(ct)) {
        const gdt::equip_t* equip = EQUIP_TABLE->getEquip(itemCode);
        if (equip != nullptr) {
            return equip->looting_mil_sec();
        }
    }
    else if (isElementType(ct)) {
        const gdt::element_t* element = ELEMENT_TABLE->getElement(itemCode);
        if (element != nullptr) {
            return element->looting_mil_sec();
        }
    }
    else if (isGemType(ct)) {
        const gdt::gem_t* gem = GEM_TABLE->getGem(itemCode);
        if (gem != nullptr) {
            return gem->looting_mil_sec();
        }
    }

    return 0;
}



inline bool canAddItemInventory(const InventoryInfo& inventory, DataCode itemCode, uint8_t itemCount) 
{
    if (isStackable(itemCode)) {
        int canStackCount = 0;
        for (const ItemMap::value_type& value : inventory.items_) {			
            if ((value.second).itemCode_ == itemCode) {
                uint8_t stackCount = getStackItemCount((value.second).itemCode_);
                if (stackCount == 0) {
                    return false;
                }
                canStackCount += getStackItemCount((value.second).itemCode_) - (value.second).count_;
                if (canStackCount >= itemCount) {
                    return true;
                }
            }
        }
    }
    return inventory.isStorable(inventory.getFirstEmptySlotId()) ;
}


inline void increaseItemCount(ItemInfo& source, ItemInfo& des) {
    if (isStackable(source.itemCode_) && source.itemCode_ == des.itemCode_) {
        const uint8_t remainCount = getStackItemCount(source.itemCode_) - source.count_;
        if (des.count_ <= remainCount) {
            source.count_ += des.count_;
            des.count_ = 0;
        }
        else {
            source.count_ += remainCount;
            des.count_ -= remainCount;
        }
    }
}


inline bool isFullStack(const BaseItemInfo& itemInfo)
{
    return getStackItemCount(itemInfo.itemCode_) == itemInfo.count_;
}


inline bool addInventoryItem(InventoryInfo& inventory, ItemInfo& itemInfo)
{
    if (! canAddItemInventory(inventory, itemInfo.itemCode_, itemInfo.count_)) {
        return false;
    }

    const bool hasItem = inventory.hasSameKindItem(itemInfo.itemCode_);
    if (hasItem && isStackable(itemInfo.itemCode_)) {
        for (ItemMap::value_type& value : inventory.items_) {
            ItemInfo& invenItemInfo = value.second;
            if (invenItemInfo.itemCode_ != itemInfo.itemCode_) {
                continue;
            }

            const uint8_t stackableCount = getStackItemCount(itemInfo.itemCode_) - invenItemInfo.count_;
            if (stackableCount > 0) {
                const uint8_t increaseCount = (itemInfo.count_ <= stackableCount) ? 
                    itemInfo.count_ : (itemInfo.count_ - stackableCount);
                invenItemInfo.count_ += increaseCount;
                itemInfo.count_ -= increaseCount;
            }

            if (itemInfo.count_ <= 0) {
                return true;
            }
        }
    }

    itemInfo.slotId_ = inventory.getFirstEmptySlotId();
    inventory.addItem(itemInfo);
    return true;
}


inline bool checkCanAddItems(InventoryInfo& inventory, const BaseItemInfos& baseItems)
{
    ObjectId newItemId = inventory.getHasMaxId();
    for (const BaseItemInfo& itemInfo : baseItems) {
        ItemInfo item(itemInfo, ++newItemId, invalidSlotId);
        assert(itemInfo.count_ <= getStackItemCount(itemInfo.itemCode_));       
        if (! addInventoryItem(inventory, item)) {
            return false;
        }        
    }
    return true;
}


inline bool checkCanRemoveItems(InventoryInfo& inventory, BaseItemInfos& baseItems)
{
    for (BaseItemInfo& baseItemInfo : baseItems) {
        while (true) {
            ItemInfo* findItem = inventory.getFirstItemInfo(baseItemInfo.itemCode_);
            if (! findItem) {
                return false;
            }
            if (findItem->count_ <= baseItemInfo.count_) {
                baseItemInfo.count_ -= findItem->count_;
                inventory.removeItem(findItem->itemId_);
                if (baseItemInfo.count_ == 0) {
                    break;
                }
                else {
                    continue;
                }
            }

            findItem->count_ -= baseItemInfo.count_;               
            break;           
        }
    }
    return true;
}


inline BaseItemInfos getWorkerItems(const BaseItemInfos& metarials)
{
    const ElementCode workerItemCode = 150994978;
    BaseItemInfos workerItems;
    uint8_t stackCount = getStackItemCount(workerItemCode);
    if (stackCount == 0) {
        return workerItems;
    }
    for (BaseItemInfo item : metarials) {
        if (item.itemCode_ == workerItemCode) {
            while (item.count_ != 0) {
                if (item.count_ <= stackCount) {
                    workerItems.push_back(item);
                    item.count_ = 0;
                    break;
                }
                else {
                    workerItems.push_back(BaseItemInfo(workerItemCode, stackCount));
                    item.count_ -= stackCount;
                }
            }
        }
    }
    return workerItems;
}



}} // namespace gideon { namespace zoneserver {
