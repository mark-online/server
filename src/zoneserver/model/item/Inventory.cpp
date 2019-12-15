#include "ZoneServerPCH.h"
#include "Inventory.h"
#include "../gameobject/Entity.h"
#include "../gameobject/ability/ItemCastable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/InventoryCallback.h"
#include "../../service/item/ItemIdGenerator.h"
#include "../../helper/InventoryHelper.h"
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/shared/data/Rate.h>

namespace gideon { namespace zoneserver {

namespace {




} // namespace {

Inventory::Inventory(go::Entity& owner, InventoryInfo& inventoryInfo,
    InvenType invenType) :
    owner_(owner),
    inventoryInfo_(inventoryInfo),
    invenType_(invenType)
{
    initCastItem();
}


ErrorCode Inventory::moveItem(ObjectId itemId, SlotId slotId)
{
    if (! inventoryInfo_.isStorable(slotId)) {
        return ecInventorySlotNotValid;
    }

    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (! itemInfo) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo->isEquipped()) {
        return ecInventoryCannotChangeEquippedItemSlot;
    }

    if (inventoryInfo_.isOccupiedSlot(slotId)) {
        return ecInventorySlotOccupied;
    }

    itemInfo->slotId_ = slotId;
	gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (callback) {
        callback->inventoryItemMoved(invenType_, itemId, slotId);	
    }
	
    return ecOk;
}


ErrorCode Inventory::switchItem(ObjectId itemId1, ObjectId itemId2)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* itemInfo1 = inventoryInfo_.getItemInfo(itemId1);
    if (! itemInfo1) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo1->isEquipped()) {
        return ecInventoryCannotChangeEquippedItemSlot;
    }

    ItemInfo* itemInfo2 = inventoryInfo_.getItemInfo(itemId2);
    if (! itemInfo2) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo2->isEquipped()) {
        return ecInventoryCannotChangeEquippedItemSlot;
    }

    swaipItem_i(*itemInfo1, *itemInfo2);

    return ecOk;
}


ErrorCode Inventory::addItem(const AddItemInfo& addItemInfo, SlotId slotId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ObjectId tempObjectId = invalidObjectId;
    return addItem_i(tempObjectId, addItemInfo, slotId);
}


ErrorCode Inventory::addItem(ObjectId& addItemId, const AddItemInfo& addItemInfo)
{
    return addItem_i(addItemId, addItemInfo, invalidSlotId);
}


ErrorCode Inventory::addMercenaryItem(const BaseItemInfo& baseItemInfo,
    uint8_t workday, SlotId slotId, ObjectId itemId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (inventoryInfo_.isStorable(slotId)) {
        if (inventoryInfo_.isOccupiedSlot(slotId)) {
            return ecInventorySlotOccupied;
        }
    }
    else {
        slotId = inventoryInfo_.getFirstEmptySlotId();
        if (! isValidSlotId(slotId)) {
            return ecInventoryIsFull;
        }
    }

    if (! isValidObjectId(itemId)) {
        itemId = ITEM_ID_GENERATOR->generateItemId();
    }
    ItemInfo itemInfo(baseItemInfo, itemId, slotId, workday);
    inventoryInfo_.addItem(itemInfo);


    gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (callback) {
        callback->inventoryItemAdded(invenType_, itemInfo);
    }

    return ecOk;
}


ErrorCode Inventory::removeItem(ObjectId itemId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return removeItem_i(itemId);
}


ErrorCode Inventory::divideItem(ObjectId itemId, uint8_t count, SlotId slotId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
	if (! itemInfo) {
		return ecInventoryItemNotFound;
	}

	if (itemInfo->isEquipment()) {
		return ecInventoryEquipItemNotDivide;
	}
    
    if (isValidSlotId(slotId)) {
        if (! inventoryInfo_.isStorable(slotId)) {
            return ecInventorySlotNotValid;
        }
        if (inventoryInfo_.isOccupiedSlot(slotId)) {
            return ecInventorySlotOccupied;
        }
    }
    else{
        slotId = inventoryInfo_.getFirstEmptySlotId();
    }

	if (! inventoryInfo_.isStorable(slotId)) {
		return ecInventoryIsFull;
	}

	if (itemInfo->count_ <= count) {
		return ecOk;
	}
	
	itemInfo->count_ -= count;
	gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (callback) {
        callback->inventoryItemCountUpdated(invenType_, itemId, itemInfo->count_);        
    }
    ObjectId tempId = invalidObjectId;
    addItem_i(tempId, createAddItemInfoByBaseItemInfo(BaseItemInfo(itemInfo->itemCode_, count)), slotId);
	
	return ecOk;
}


void Inventory::fillHalfItem(BaseItemInfos& itemInfos)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemMap::iterator pos = inventoryInfo_.items_.begin();
    gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (! callback) {
        return;
    }
    while (pos != inventoryInfo_.items_.end()) {
        ItemInfo& itemInfo = (*pos).second;
        if (itemInfo.isEquipment()) {
            if (isSuccessRate(500)) {
                itemInfos.push_back(itemInfo);                                        
            }
        }
        else {
            if (itemInfo.count_ == 1) {
                if (isSuccessRate(500)) {
                    itemInfos.push_back(itemInfo);                
                }
            }
            else if (itemInfo.count_ > 1)  {
                BaseItemInfo info(itemInfo.itemCode_, itemInfo.count_ / 2);
                itemInfos.push_back(info);
            }
        }

        ++pos;
    }
}

void Inventory::removeAllItems()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (! callback) {
        return;
    }
    inventoryInfo_.items_.clear();

    callback->inventoryItemAllRemoved(invenType_);
}


ItemInfo Inventory::getItemInfo(ObjectId itemId) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    const ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (itemInfo) {
        return *itemInfo ;
    }

    static ItemInfo nullItem;
    return nullItem;
}


void Inventory::useItemsByBaseItemInfo(const BaseItemInfo& baseItemInfo)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    useItems_i(baseItemInfo.itemCode_, baseItemInfo.count_);
}


void Inventory::useItemsByBaseItemInfos(const BaseItemInfos& baseItemInfos)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    for (const BaseItemInfo& baseItemInfo : baseItemInfos) {
        useItems_i(baseItemInfo.itemCode_, baseItemInfo.count_);
    }
}


bool Inventory::useItemsByItemId(ObjectId itemId, uint8_t count)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (itemInfo == nullptr) {
        return false;
    }

    useItems_i(*itemInfo, count);
    return true;
}


bool Inventory::checkHasItemsByBaseItemInfo(const BaseItemInfo& baseItemInfo) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return inventoryInfo_.hasEnoughItem(baseItemInfo.itemCode_, baseItemInfo.count_);
}


bool Inventory::checkHasItemsByBaseItemInfos(const BaseItemInfos& baseItemInfos) const
{
    InventoryInfo inventoryInfo;    
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        inventoryInfo = inventoryInfo_;
    }
    BaseItemInfos baseInfos = baseItemInfos;
    return checkCanRemoveItems(inventoryInfo, baseInfos);
}


bool Inventory::canAddItemByBaseItemInfo(const BaseItemInfo& baseItemInfo) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    assert(baseItemInfo.count_ <= getStackItemCount(baseItemInfo.itemCode_));
    return canAddItemInventory(inventoryInfo_, baseItemInfo.itemCode_, baseItemInfo.count_);
}


bool Inventory::canAddItemByBaseItemInfos(const BaseItemInfos& baseItemInfos) const
{
    InventoryInfo inventoryInfo;    
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        inventoryInfo = inventoryInfo_;
    }
    BaseItemInfos baseInfos = baseItemInfos;
    return checkCanAddItems(inventoryInfo, baseInfos);
}


ErrorCode Inventory::checkAddSlot(SlotId slotId) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (inventoryInfo_.isStorable(slotId)) {
        if (inventoryInfo_.isOccupiedSlot(slotId)) {
            return ecInventorySlotOccupied;
        }
        return ecOk;
    }
    else if (isValidSlotId(slotId)) {
        if (! (firstSlotId <= slotId  && slotId <= inventoryInfo_.getTotalSlotCount())) {
            return ecInventorySlotNotValid;
        }
    }
   
    return ecOk;
}


DataCode Inventory::getItemCode(ObjectId itemId) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (itemInfo == nullptr) {
        return invalidDataCode;
    }

    return itemInfo->itemCode_;
}


void Inventory::setTempHarvestItems(const BaseItemInfos& baseItems)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    tempHarvestItems_ = baseItems;
}


ErrorCode Inventory::acquireHarvest(DataCode itemCode)
{
	BaseItemInfos::iterator pos = tempHarvestItems_.begin();
	BaseItemInfos::iterator end = tempHarvestItems_.end();
	for (; pos != end; ++pos) {
		const BaseItemInfo& baseItem = (*pos);
		if (baseItem.itemCode_ == itemCode) {
            ObjectId tempId = invalidObjectId;
			const ErrorCode errorCode = addItem_i(tempId, createAddItemInfoByBaseItemInfo(baseItem), invalidSlotId);
			if (isSucceeded(errorCode)) {
				tempHarvestItems_.erase(pos);
			}
			return errorCode;
		}
	}
	return ecHarvestNotFindHarvestItem;
}


ErrorCode Inventory::addItem_i(ObjectId& addObjectId, const AddItemInfo& addItemInfo, SlotId slotId)
{
    uint8_t addItemCount = addItemInfo.count_;
    const uint8_t currentStackCount = getStackItemCount(addItemInfo.itemCode_);
	if (addItemInfo.count_ > currentStackCount) {
		return ecServerInternalError;
	}

    if (inventoryInfo_.isStorable(slotId)) {
        if (inventoryInfo_.isOccupiedSlot(slotId)) {
            return ecInventorySlotOccupied;
        }
    }
    else {
        if (! canAddItemInventory(inventoryInfo_, addItemInfo.itemCode_, addItemInfo.count_)) {
            return ecInventoryIsFull;
        }
        const bool hasItem = inventoryInfo_.hasSameKindItem(addItemInfo.itemCode_);
        if (hasItem && isStackable(addItemInfo.itemCode_)) {
            for (ItemMap::value_type& value : inventoryInfo_.items_) {
                ItemInfo& invenItemInfo = value.second;
                if (invenItemInfo.itemCode_ != addItemInfo.itemCode_) {
                    continue;
                }

                const uint8_t stackCount = getStackItemCount(addItemInfo.itemCode_);
                if (stackCount == 0) {
                    return ecServerInternalError;
                }

                const uint8_t stackableCount =  stackCount - invenItemInfo.count_;
                if (stackableCount > 0) {
                    const uint8_t increaseCount =
                        (stackableCount >= addItemInfo.count_) ? addItemInfo.count_ : stackableCount;
                    invenItemInfo.count_ += increaseCount;
                    addItemCount -= increaseCount;
                    addObjectId = invenItemInfo.itemId_;
                    gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
                    if (callback) {
					    callback->inventoryItemCountUpdated(invenType_, invenItemInfo.itemId_, invenItemInfo.count_);
                    }
                }

                if (addItemCount <= 0) {
                    return ecOk;
                }
            }
        }
    }
	
    
    if (! inventoryInfo_.isStorable(slotId)) {
        slotId = inventoryInfo_.getFirstEmptySlotId();
    }
    ObjectId itemId = isValidObjectId(addItemInfo.itemId_) ? addItemInfo.itemId_ : ITEM_ID_GENERATOR->generateItemId(); 
    addObjectId = itemId; 
    ItemInfo itemInfo(addItemInfo.itemCode_, addItemCount,
        itemId, slotId, addItemInfo.expireTime_);

    if (addItemInfo.isAccessory()) {
        itemInfo.accessoryItemInfo_ = addItemInfo.accessoryItemInfo_;
    }
    else if (addItemInfo.isEquipment()) {
        itemInfo.equipItemInfo_ = addItemInfo.equipItemInfo_;
    }

    inventoryInfo_.addItem(itemInfo);
	
	SNE_LOG_INFO("GameLog create item(PID:%" PRIu64 ", ItemID:%" PRIu64 ", itemCode:%u)",
		owner_.getObjectId(), itemInfo.itemId_, itemInfo.itemCode_);

    insertCastItem(itemInfo.itemId_, itemInfo.itemCode_);

    gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (callback) {
        callback->inventoryItemAdded(invenType_, itemInfo);
    }
    
    return ecOk;
}


ErrorCode Inventory::removeItem_i(ObjectId itemId)
{
    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (! itemInfo) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo->isEquipped()) {
        return ecInventoryEquippedItemNotRemove;
    }

    inventoryInfo_.removeItem(itemId);
    removeCastItem(itemId);
	gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (callback) {
        callback->inventoryItemRemoved(invenType_, itemId);
    }

    return ecOk;
}


void Inventory::swaipItem_i(ItemInfo& itemInfo1, ItemInfo& itemInfo2)
{
    bool isOnlySwap = itemInfo1.itemCode_ != itemInfo2.itemCode_;
    bool shouldDeleteItem1 = itemInfo1.shouldDelete();
    // 삭제될경우 때문에..
    ObjectId itemInfo1Id = itemInfo1.itemId_;

    if (! isOnlySwap) {
        isOnlySwap = (! isStackable(itemInfo1.itemCode_)) || (isFullStack(itemInfo1)) || 
            (isFullStack(itemInfo2));         
    }

    if (isOnlySwap) {        
        std::swap(itemInfo1.slotId_, itemInfo2.slotId_);
    }
    else {
        increaseItemCount(itemInfo2, itemInfo1);
        shouldDeleteItem1 = itemInfo1.shouldDelete();
        if (shouldDeleteItem1) {
            inventoryInfo_.removeItem(itemInfo1.itemId_);
            removeCastItem(itemInfo1.itemId_);
        }
    }

    gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (callback) {
        if (isOnlySwap) {        
            callback->inventoryItemSwitched(invenType_, itemInfo1.itemId_, itemInfo2.itemId_);	
        }
        else {
            callback->inventoryItemCountUpdated(invenType_, itemInfo2.itemId_, itemInfo2.count_);
            if (shouldDeleteItem1) {
                callback->inventoryItemRemoved(invenType_, itemInfo1Id);
            }
            else {
                callback->inventoryItemCountUpdated(invenType_, itemInfo1.itemId_, itemInfo1.count_);
            }    
        }
    }
}


void Inventory::useItems_i(DataCode dataCode, int needCount)
{
    ItemMap::iterator pos = inventoryInfo_.items_.begin();
    while (pos != inventoryInfo_.items_.end()) {
        ItemInfo& item = (*pos).second;
        if (item.itemCode_ == dataCode) {
            if (needCount <= item.count_ ) {                
                useItems_i(item, uint8_t(needCount));
                break;
            }
            else {
                needCount -= item.count_;
                removeItem_i(item.itemId_);
                pos = inventoryInfo_.items_.begin();
                continue;
            }
        }   
        ++pos;
    }
}


void Inventory::useItems_i(ItemInfo& itemInfo, uint8_t needCount)
{
    const ObjectId itemId = itemInfo.itemId_;

    if (needCount == itemInfo.count_) {
        removeItem_i(itemInfo.itemId_);
    }
    else {
        itemInfo.count_ -= needCount;
        gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
        if (callback) {
            callback->inventoryItemCountUpdated(invenType_, itemId, itemInfo.count_);
        }
    }
}


void Inventory::initCastItem()
{
    for (const ItemMap::value_type& value : inventoryInfo_.items_) {
        const ItemInfo& itemInfo = value.second;
        insertCastItem(itemInfo.itemId_, itemInfo.itemCode_);
    }
}


void Inventory::insertCastItem(ObjectId itemId, DataCode itemCode)
{
    CodeType codeType = getCodeType(itemCode);
    if (isElementType(codeType)) {
        go::ItemCastable* itemCastable = owner_.queryItemCastable();
        if (itemCastable) {
            itemCastable->insertCastItem(itemId, itemCode);
        }
    }
}


void Inventory::removeCastItem(ObjectId itemId)
{
    go::ItemCastable* itemCastable = owner_.queryItemCastable();
    if (itemCastable) {
        itemCastable->removeCastItem(itemId);
    }
}

}} // namespace gideon { namespace zoneserver {
