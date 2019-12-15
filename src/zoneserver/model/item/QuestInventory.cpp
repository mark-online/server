#include "ZoneServerPCH.h"
#include "QuestInventory.h"
#include "../gameobject/Entity.h"
#include "../gameobject/ability/ItemCastable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/QuestInventoryCallback.h"
#include "../../service/item/ItemIdGenerator.h"
#include <gideon/cs/datatable/QuestItemTable.h>

namespace gideon { namespace zoneserver {

QuestInventory::QuestInventory(go::Entity& owner, QuestInventoryInfo& itemInventoryInfo):
    owner_(owner),
    itemInventoryInfo_(itemInventoryInfo)
{
}


QuestInventory::~QuestInventory()
{
}


void QuestInventory::initialize()
{
    for (const QuestItemInfo& itemInfo : itemInventoryInfo_.questItems_) {
        insertUseableItem(itemInfo.itemId_, itemInfo.questItemCode_);
    }
}


bool QuestInventory::addItem(const QuestItemInfo& questItem)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return addItem_i(questItem);
}


void QuestInventory::removeItemsByQuestCode(QuestCode questCode)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    QuestItemInfos::iterator pos = itemInventoryInfo_.questItems_.begin();
    while (pos != itemInventoryInfo_.questItems_.end()) {
        QuestItemInfo& itemInfo = (*pos);
        if (itemInfo.questCode_ == questCode) {
            gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
            if (callback) {
                callback->inventoryQuestItemRemoved(itemInfo.itemId_);
            }
            pos = itemInventoryInfo_.questItems_.erase(pos);
            continue;;
        }
        ++pos;
    }
}


void QuestInventory::removeItem(QuestItemCode itemCode, QuestCode questCode)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    QuestItemInfos::iterator pos = itemInventoryInfo_.questItems_.begin();
    const QuestItemInfos::const_iterator end = itemInventoryInfo_.questItems_.end();
    for (pos; pos != end; ++pos) {
        QuestItemInfo& itemInfo = (*pos);
        if (itemInfo.questCode_ == questCode && itemInfo.questItemCode_ == itemCode) {
            gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
            if (callback) {
                callback->inventoryQuestItemRemoved(itemInfo.itemId_);
            }
            itemInventoryInfo_.questItems_.erase(pos);
            return;
        }
    }
}


void QuestInventory::removeItemsByItemCode(QuestItemCode itemCode, uint8_t removeCount)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    int i = 0;
    QuestItemInfos::iterator pos = itemInventoryInfo_.questItems_.begin();
    while (pos != itemInventoryInfo_.questItems_.end()) {
        if (i >= removeCount) {
            return;
        }
        QuestItemInfo& itemInfo = (*pos);
        if (itemInfo.questItemCode_ == itemCode) {
            gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
            if (callback) {
                callback->inventoryQuestItemRemoved(itemInfo.itemId_);
            }
            pos = itemInventoryInfo_.questItems_.erase(pos);
            ++i;
            continue;
        }
        ++pos;
    }
}


ErrorCode QuestInventory::removeItem(ObjectId itemId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (itemInventoryInfo_.getItemInfoById(itemId) != nullptr) {
        itemInventoryInfo_.removeItem(itemId);
        gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
        if (callback) {
            callback->inventoryQuestItemRemoved(itemId);
        }
        return ecOk;
    }
    return ecItemNotExist;
}


bool QuestInventory::checkHasQuestItem(QuestItemCode itemCode, uint8_t itemCount) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    int count = 0;
    for (const QuestItemInfo& item : itemInventoryInfo_.questItems_) {
        if (item.questItemCode_ == itemCode) {
            ++count;
        }
    }
    return count >= itemCount;
}


bool QuestInventory::checkHasQuestItems(BaseItemInfos& baseItemInfos) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (baseItemInfos.size() == 1) {
        BaseItemInfos::const_iterator pos = baseItemInfos.begin();
        if (pos != baseItemInfos.end()) {
            return checkHasQuestItem((*pos).itemCode_, (*pos).count_);
        }
    }
    else {
        QuestItemInfos questItems = itemInventoryInfo_.questItems_;        
        for (BaseItemInfo&  itemInfo : baseItemInfos) {
            bool isOk = false;
            QuestItemInfos::iterator pos = questItems.begin();
            const QuestItemInfos::const_iterator end = questItems.end();
            for (pos; pos != end; ++pos) {
                QuestItemInfo& questItem = (*pos);
                if (questItem.questItemCode_ == itemInfo.itemCode_) {
                    questItems.erase(pos);
                    isOk = true;
                    break;
                }
            }
            if (! isOk) {
                return false;
            }
        }
    }
    return true;
}


const QuestItemInfo* QuestInventory::getItemInfoById(ObjectId itemId) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    QuestItemInfo* itemInfo = itemInventoryInfo_.getItemInfoById(itemId);
    if (itemInfo) {
        return itemInfo;
    }
    return nullptr;
}


const QuestItemInfo* QuestInventory::getItemInfoByCode(QuestItemCode questItemCode) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

	const QuestItemInfo* itemInfo = itemInventoryInfo_.getItemInfoByCode(questItemCode);
	if (itemInfo) {
		return itemInfo;
	}
	return nullptr;
}


bool QuestInventory::hasUseableCount(ObjectId itemId) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    QuestItemInfo* itemInfo = itemInventoryInfo_.getItemInfoById(itemId);
    if (! itemInfo) {
        return false;
    }

    const datatable::QuestItemTemplate* itemTemplate = 
        QUEST_ITEM_TABLE->getQuestItemTemplate(itemInfo->questItemCode_);
    if (! itemTemplate) {
        return false;
    }

    if (itemTemplate->getUsableCount() == 0) {
        return true; // 무제한
    }

    return itemInfo->usableCount_ > 0;
}


bool QuestInventory::canAddQuestItem(uint8_t itemCount)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return itemInventoryInfo_.canAddItem(itemCount);
}


void QuestInventory::useItemCount(ObjectId itemId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    QuestItemInfo* itemInfo = itemInventoryInfo_.getItemInfoById(itemId);
    if (! itemInfo) {
        return;
    }

    if (! itemInfo->useItem()) {
        return;
    }

    const datatable::QuestItemTemplate* itemTemplate = 
        QUEST_ITEM_TABLE->getQuestItemTemplate(itemInfo->questItemCode_);
    if (! itemTemplate) {
        assert(false);
        return;
    }

    if (itemTemplate->getUsableCount() > 0) {
        //if (itemTemplate->isZeroCountDestory()) {
        //    removeItem_i(itemId);
        //}
        //else {
        //    gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
        //    if (callback) {
        //        callback->inventoryQuestItemUsableCountUpdated(itemInfo->itemId_, itemInfo->usableCount_);;
        //    }
        //}
    }
}


void QuestInventory::setTempHarvestItems(const QuestItemInfos& questItemInfos)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    tempHarvestItems_ = questItemInfos;
}


ErrorCode QuestInventory::acquireHarvest(DataCode itemCode)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    QuestItemInfos::iterator pos = tempHarvestItems_.begin();
    QuestItemInfos::iterator end = tempHarvestItems_.end();
    for (; pos != end; ++pos) {
        QuestItemInfo& questItem = (*pos);
        if (questItem.questItemCode_ == itemCode) {         
            if (! addItem_i(questItem)) {
                return ecInventoryIsFull;
            }
            
            tempHarvestItems_.erase(pos);            
            return ecOk;
        }
    }

    return ecHarvestNotFindHarvestItem;
}



bool QuestInventory::addItem_i(QuestItemInfo questItem)
{
//    assert(isValidDataCode(questItem.questCode_));
	QuestItemInfo* questItemInfo = itemInventoryInfo_.getItemInfoByCode(questItem.questItemCode_);
	if (questItemInfo) {
		questItemInfo->stackCount_ += questItem.stackCount_;
		gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
		if (callback) {
			callback->inventoryQuestItemCountUpdated(questItemInfo->itemId_, questItemInfo->stackCount_);
		}
		return true;
	}

	questItem.itemId_ = ITEM_ID_GENERATOR->generateItemId();
	const bool result = itemInventoryInfo_.addItem(questItem);
    if (result) {
        gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
        if (callback) {
            callback->inventoryQuestItemAdded(questItem);
			insertUseableItem(questItem.itemId_, questItem.questItemCode_);
        }
    }

    return result;
}


ErrorCode QuestInventory::removeItem_i(ObjectId itemId)
{
    const QuestItemInfo* itemInfo = itemInventoryInfo_.getItemInfoById(itemId);
    if (! itemInfo) {
        return ecItemNotExist;
    }

    QuestItemCode itemCode = itemInfo->questItemCode_;

    itemInventoryInfo_.removeItem(itemId);
    gc::QuestInventoryCallback* callback = owner_.getController().queryQuestInventoryCallback();
    if (callback) {
        callback->inventoryQuestItemRemoved(itemId);
    }

    const datatable::QuestItemTemplate* itemTemplate = QUEST_ITEM_TABLE->getQuestItemTemplate(itemCode);
    if (itemTemplate) {
        if (itemTemplate->isUsable()) {
            owner_.queryItemCastable()->removeCastItem(itemId);
        }
    }
    return ecOk;
    
}


bool QuestInventory::insertUseableItem(ObjectId itemId, QuestItemCode itemCode)
{
    const datatable::QuestItemTemplate* itemTemplate = 
        QUEST_ITEM_TABLE->getQuestItemTemplate(itemCode);
    if (! itemTemplate) {
        assert(false);
        return false;
    }
    if (itemTemplate->isUsable()) {
        owner_.queryItemCastable()->insertCastItem(itemId, itemCode);
    }
    return true;
}

}} // namespace gideon { namespace zoneserver {
