#include "CommunityServerPCH.h"
#include "GuildInventory.h"
#include "GuildInventoryCallback.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/cs/shared/data/Code.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/GemTable.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/FunctionTable.h>
#include <sne/database/DatabaseManager.h>

using DatabaseGuard = sne::database::Guard<gideon::serverbase::ProxyGameDatabase>;

namespace gideon { namespace communityserver {
namespace {


inline uint8_t getStackItemCount(DataCode itemCode)
{
    const CodeType codeType = getCodeType(itemCode);
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
    else if (ctFunction == codeType) {
        const gdt::function_t* funtion = FUNCTION_TABLE->getFunction(itemCode);
        if (funtion) {
            return static_cast<uint8_t>(funtion->stack_count());
        }
    }
    return 0;
}


inline bool isFullStack(const BaseItemInfo& itemInfo)
{
    return getStackItemCount(itemInfo.itemCode_) == itemInfo.count_;
}


inline void increaseItemCount(ItemInfo& source, ItemInfo& des)
{
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

inline bool canAddItemInventory(const VaultInfo& vaultInfo, const ItemInfo& newItemInfo) 
{
    if (isStackable(newItemInfo.itemCode_)) {
        int canStackCount = 0;
        for (const ItemMap::value_type& value : vaultInfo.itemMap_) {
            const ItemInfo& itemInfo = value.second;
            if (itemInfo.itemCode_ == newItemInfo.itemCode_) {
                uint8_t stackCount = getStackItemCount(itemInfo.itemCode_);
                if (stackCount == 0) {
                    return false;
                }
                canStackCount += getStackItemCount(itemInfo.itemCode_) - itemInfo.count_;
                if (canStackCount >= newItemInfo.count_) {
                    return true;
                }
            }
        }
    }
    return vaultInfo.isStorable(vaultInfo.getFirstEmptySlotId()) ;
}

} // namespace {

// = GuildInventory

GuildInventory::GuildInventory(GuildInventoryCallback& callback) :
    callback_(callback)
{
}


ErrorCode GuildInventory::addVault(const BaseVaultInfo& vaultInfo)
{
    if (vaultInfo.id_ != inventoryInfo_.getNextVaultId()) {
        return ecGuild_InvalidVault;
    }

    if (! inventoryInfo_.canAddVault()) {
        return ecGuild_TooManyVault;
    }

    inventoryInfo_.addVault(vaultInfo);

    callback_.vaultAdded(vaultInfo);
    return ecOk;
}


/// itemInfo를 의도적으로 복사해서 사용함
ErrorCode GuildInventory::pushItem(ItemInfo itemInfo, VaultId vaultId, SlotId slotId)
{
    VaultInfo* vaultInfo = inventoryInfo_.getVaultInfo(vaultId);
    if (! vaultInfo) {
        assert(false);
        return ecInventoryInvalidVault;
    }

    const uint8_t stackCount = getStackItemCount(itemInfo.itemCode_);
    if (itemInfo.count_ > stackCount) {
        assert(false);
        return ecServerInternalError;
    }

    if (vaultInfo->isStorable(slotId)) {
        if (vaultInfo->isOccupiedSlot(slotId)) {
            return ecInventorySlotOccupied;
        }
    }
    else {
        if (! canAddItemInventory(*vaultInfo, itemInfo)) {
            return ecInventoryIsFull;
        }
        const bool hasItem = vaultInfo->hasSameKindItem(itemInfo.itemCode_);
        if (hasItem && isStackable(itemInfo.itemCode_)) {
            for (ItemMap::value_type& value : vaultInfo->itemMap_) {
                ItemInfo& invenItemInfo = value.second;
                if (invenItemInfo.itemCode_ != itemInfo.itemCode_) {
                    continue;
                }

                // TODO: 뭔가 코드가 잘못된 것 같음
                // const uint8_t stackCount = getStackItemCount(itemInfo.itemCode_);
                if (stackCount == 0) {
                    return ecServerInternalError;
                }

                const uint8_t stackableCount = stackCount - invenItemInfo.count_;
                if (stackableCount > 0) {
                    const uint8_t increaseCount =
                        (stackableCount >= itemInfo.count_) ? itemInfo.count_ : stackableCount;
                    invenItemInfo.count_ += increaseCount;
                    itemInfo.count_ -= increaseCount;
                }

                if (itemInfo.count_ <= 0) {
                    return ecOk;
                }
            }
        }
    }

    if (! vaultInfo->isStorable(slotId)) {
        slotId = vaultInfo->getFirstEmptySlotId();
    }

    itemInfo.slotId_ = slotId;

    vaultInfo->addItem(itemInfo);

    callback_.inventoryItemAdded(vaultId, itemInfo);    

    return ecOk;
}


ErrorCode GuildInventory::popItem(VaultId vaultId, ObjectId itemId)
{
    VaultInfo* vaultInfo = inventoryInfo_.getVaultInfo(vaultId);
    if (! vaultInfo) {
        assert(false);
        return ecInventoryInvalidVault;
    }

    const ItemInfo* itemInfo = vaultInfo->getItemInfo(itemId);
    if (! itemInfo) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo->isEquipped()) {
        return ecInventoryEquippedItemNotRemove;
    }

    vaultInfo->removeItem(itemId);

    callback_.inventoryItemRemoved(vaultId, itemId);

    return ecOk;
}


ErrorCode GuildInventory::switchItem(VaultId vaultId, ObjectId itemId1, ObjectId itemId2)
{
    VaultInfo* vaultInfo = inventoryInfo_.getVaultInfo(vaultId);
    if (! vaultInfo) {
        assert(false);
        return ecInventoryInvalidVault;
    }

    ItemInfo* itemInfo1 = vaultInfo->getItemInfo(itemId1);
    if (! itemInfo1) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo1->isEquipped()) {
        return ecInventoryCannotChangeEquippedItemSlot;
    }

    ItemInfo* itemInfo2 = vaultInfo->getItemInfo(itemId2);
    if (! itemInfo2) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo2->isEquipped()) {
        return ecInventoryCannotChangeEquippedItemSlot;
    }

    // 삭제될경우 때문에..
    const ObjectId itemInfo1Id = itemInfo1->itemId_;
    bool shouldDeleteItem1 = itemInfo1->shouldDelete();

    bool isOnlySwap = itemInfo1->itemCode_ != itemInfo2->itemCode_;
    if (! isOnlySwap) {
        isOnlySwap = (! isStackable(itemInfo1->itemCode_)) || (isFullStack(*itemInfo1)) || 
            (isFullStack(*itemInfo2));  
    }

    if (isOnlySwap) {        
        if (! vaultInfo->switchItem(itemId1, itemId2)) {
            return ecServerInternalError;
        }
    }
    else { // stack
        increaseItemCount(*itemInfo2, *itemInfo1);
        shouldDeleteItem1 = itemInfo1->shouldDelete();
        if (shouldDeleteItem1) {
            vaultInfo->removeItem(itemInfo1->itemId_);
        }
    }

    if (isOnlySwap) {
        callback_.inventoryItemSwitched(vaultId, itemInfo1->itemId_, itemInfo1->slotId_, 
            itemInfo2->itemId_, itemInfo2->slotId_);	
    }
    else {
        callback_.inventoryItemCountUpdated(vaultId, itemInfo2->itemId_, itemInfo2->count_);
        if (shouldDeleteItem1) {
            callback_.inventoryItemRemoved(vaultId, itemInfo1Id);
        }
        else {
            callback_.inventoryItemCountUpdated(vaultId, itemInfo1->itemId_, itemInfo1->count_);
        }    
    }
    
    return ecOk;
}


ErrorCode GuildInventory::moveItem(VaultId vaultId, ObjectId itemId, SlotId slotId)
{
    VaultInfo* vaultInfo = inventoryInfo_.getVaultInfo(vaultId);
    if (! vaultInfo) {
        assert(false);
        return ecInventoryInvalidVault;
    }

    if (! vaultInfo->isStorable(slotId)) {
        return ecInventorySlotNotValid;
    }

    ItemInfo* itemInfo = vaultInfo->getItemInfo(itemId);
    if (! itemInfo) {
        return ecInventoryItemNotFound;
    }

    if (vaultInfo->isOccupiedSlot(slotId)) {
        return ecInventorySlotOccupied;
    }

    vaultInfo->moveItem(itemId, slotId);
    
    callback_.inventoryItemMoved(vaultId, itemId, slotId);	
    
    return ecOk;
}


ErrorCode GuildInventory::updateVaultName(VaultId vaultId, const VaultName& name)
{
    VaultInfo* vaultInfo = inventoryInfo_.getVaultInfo(vaultId);
    if (! vaultInfo) {
        assert(false);
        return ecInventoryInvalidVault;
    }

    vaultInfo->name_ = name;

    callback_.vaultNameUpdated(vaultId, name);
    return ecOk;
}


ItemInfo GuildInventory::getItemInfo(VaultId vaultId, ObjectId itemId) const
{
    const ItemInfo* itemInfo = inventoryInfo_.getItemInfo(vaultId, itemId);
    if (itemInfo) {
        return *itemInfo;
    }

    static ItemInfo nullItem;
    return nullItem;
}


void GuildInventory::fillInventoryInfo(InventoryInfo& invenInfo, VaultId vaultId) const
{
    const VaultInfo* vaultInfo = inventoryInfo_.getVaultInfo(vaultId);
    if (! vaultInfo) {
        assert(false);
        return;
    }

    invenInfo.defaultInvenCount_ = maxGuildVaultSlotCount;
    invenInfo.items_ = vaultInfo->itemMap_;
}


}} // namespace gideon { namespace communityserver {
