#include "ZoneServerPCH.h"
#include "AccessoryInventory.h"
#include "../gameobject/Entity.h"
#include "../gameobject/ability/Inventoryable.h"
#include "../gameobject/ability/Liveable.h"
#include "../gameobject/status/CreatureStatus.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/StatsCallback.h"
#include "../../controller/callback/AccessoryInventoryCallback.h"
#include <gideon/cs/datatable/AccessoryTable.h>
#include "esut/Random.h"

namespace gideon { namespace zoneserver {


AccessoryInventory::AccessoryInventory(go::Entity& owner, InventoryInfo& inventory,
    CharacterAccessories& characterAccessories) :
    owner_(owner),
    inventoryInfo_(inventory),
    characterAccessories_(characterAccessories)

{
}


ErrorCode AccessoryInventory::equipAccessory(ObjectId itemId, AccessoryIndex index)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* equipItem = inventoryInfo_.getItemInfo(itemId);
    if (! equipItem) {
        return ecInventoryItemNotFound;
    }

    //if (! equipItem->isAccessory()) {
    //    return ecItemNotAccessory;
    //}

    if (equipItem->isEquipped()) {
        return ecEquipItemAlreadyEquipped;
    }

    const AccessoryPart partForEquip = getAccessoryPart(equipItem->itemCode_);
    assert(gideon::isValid(partForEquip));
	
	go::Inventoryable* inventoryable = owner_.queryInventoryable();
	if (! inventoryable) {
		return ecServerInternalError;
	}

    if (! isValid(index)) {
        if (partForEquip == apNecklace) {
            index = aiNecklace;
        }
        else if (partForEquip == apBracelet) {
            index = aiBracelet1;
            if (inventoryable->isEquipAccessory(aiBracelet1)) {
                if (! inventoryable->isEquipAccessory(aiBracelet2)) {
                    index = aiBracelet2;
                }
            }
        }
        else if (partForEquip == apRing) {
            index = aiRing1;
            if (inventoryable->isEquipAccessory(aiRing1)) {
                if (! inventoryable->isEquipAccessory(aiRing2)) {
                    index = aiRing2;
                }
            }
        }
    }

    //if (! isEquiipAccessoryPart(index, partForEquip)) {
    //    return ecItemNotInvalidAccessoryIndex;
    //}

    if (! isValidObjectId(characterAccessories_[index])) {
        equipAccessory(*equipItem, equipItem->accessoryItemInfo_, index);
        return ecOk;
    }
    
    ItemInfo* unequipItemInfo = inventoryInfo_.getItemInfo(characterAccessories_[index]);
    if (! unequipItemInfo) {
        return ecInventoryItemNotFound;
    }

    swapAccessory(*equipItem, equipItem->accessoryItemInfo_, *unequipItemInfo, unequipItemInfo->accessoryItemInfo_, index);
    return ecOk;
}


ErrorCode AccessoryInventory::unequipAccessory(ObjectId itemId, SlotId slotId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (! itemInfo) {
        return ecInventoryItemNotFound;
    }

    //if (! itemInfo->isAccessory()) {
    //    return ecItemNotAccessory;
    //}

    if (! itemInfo->isEquipped()) {
        return ecUnequipItemNotEquipped;
    }

    if (! isValidSlotId(slotId) || isEquipped(slotId)) {
        slotId = inventoryInfo_.getFirstEmptySlotId();
    }

    if (slotId == invalidSlotId) {
        return ecInventoryIsFull;
    }

    if (inventoryInfo_.isOccupiedSlot(slotId)) {
        return ecInventorySlotOccupied;
    }
	go::Inventoryable* inventoryable = owner_.queryInventoryable();
	if (! inventoryable) {
		return ecServerInternalError;
	}

    AccessoryIndex index = inventoryable->getAccessoryIndex(itemId);
    if (! isValid(index)) {
        return ecUnequipItemNotEquipped;
    }

    itemInfo->setSlotId(slotId);
    inventoryable->setAccessoryIndex(index, invalidObjectId);

    gc::AccessoryInventoryCallback* invenCallback = owner_.getController().queryAccessoryInventoryCallback();
    if (invenCallback) {                       
        invenCallback->unequipAccessoryItemReplaced(itemInfo->itemId_, slotId, index);
    }

    owner_.queryLiveable()->getCreatureStatus().accessoryUnequipped(itemInfo->itemCode_, itemInfo->accessoryItemInfo_);
    gc::StatsCallback* statCallback = owner_.getController().queryStatsCallback();
    if (statCallback != nullptr) {
        statCallback->fullCreatureStatusChanged();
    }

    return ecOk;
}


void AccessoryInventory::equipAccessory(ItemInfo& equipItemInfo, const AccessoryItemInfo& equipAccessoryInfo, AccessoryIndex index)
{
    equipItemInfo.setSlotId(equippedSlotId);

    owner_.queryLiveable()->getCreatureStatus().accessoryEquipped(equipItemInfo.itemCode_, equipAccessoryInfo);
    owner_.queryInventoryable()->setAccessoryIndex(index, equipItemInfo.itemId_);

    gc::AccessoryInventoryCallback* invenCallback = owner_.getController().queryAccessoryInventoryCallback();
    if (invenCallback) {
        invenCallback->equipAccessoryReplaced(equipItemInfo.itemId_, index);
    }

    gc::StatsCallback* statCallback = owner_.getController().queryStatsCallback();
    if (statCallback != nullptr) {
        statCallback->fullCreatureStatusChanged();
    }

}


void AccessoryInventory::swapAccessory(ItemInfo& equipItemInfo, const AccessoryItemInfo& equipAccessoryInfo,
    ItemInfo& unequipItemInfo, const AccessoryItemInfo& unequipAccessoryInfo, AccessoryIndex swapIndex)
{
    SlotId invenSlotId = equipItemInfo.slotId_;
    
    owner_.queryLiveable()->getCreatureStatus().accessoryChanged(unequipItemInfo.itemCode_, unequipAccessoryInfo,
        equipItemInfo.itemCode_, equipAccessoryInfo);

    unequipItemInfo.setSlotId(invenSlotId);
    equipItemInfo.setSlotId(equippedSlotId);
    owner_.queryInventoryable()->setAccessoryIndex(swapIndex, equipItemInfo.itemId_);
    
    gc::AccessoryInventoryCallback* invenCallback = owner_.getController().queryAccessoryInventoryCallback();
    if (invenCallback) {
        invenCallback->inventoryWithAccessoryItemReplaced(unequipItemInfo.itemId_, 
            swapIndex, equipItemInfo.itemId_, swapIndex);             
    }

    gc::StatsCallback* statCallback = owner_.getController().queryStatsCallback();
    if (statCallback != nullptr) {
        statCallback->fullCreatureStatusChanged();
    }
}

}} // namespace gideon { namespace zoneserver {
