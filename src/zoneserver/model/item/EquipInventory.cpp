#include "ZoneServerPCH.h"
#include "EquipInventory.h"
#include "Inventory.h"
#include "../gameobject/Entity.h"
#include "../gameobject/Creature.h"
#include "../gameobject/ability/PassiveSkillCastable.h"
#include "../gameobject/ability/Inventoryable.h"
#include "../gameobject/ability/Liveable.h"
#include "../gameobject/ability/Achievementable.h"
#include "../gameobject/status/CreatureStatus.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/StatsCallback.h"
#include "../../controller/callback/EquipInventoryCallback.h"
#include "../../controller/callback/InventoryCallback.h"
#include "../../service/item/ItemOptionService.h"
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/GemTable.h>
#include "esut/Random.h"

namespace gideon { namespace zoneserver {

namespace {

BaseItemInfo createFragmentItem(FragmentCode code, uint8_t downCount)
{
    const gdt::equip_t* equip = EQUIP_TABLE->getEquip(code);
    assert(equip);
    const uint8_t itemCount = downCount;
    return BaseItemInfo(equip->fragment_code(), itemCount);
}


inline bool shuoldUnequip(uint8_t originalCount, uint8_t currentCount)
{
    return originalCount> currentCount;
}


} // namespace {


EquipInventory::EquipInventory(go::Entity& owner, InventoryInfo& inventoryInfo,
    CharacterEquipments& characterEquipments, CreatureEquipments& creatureEquipments) :
    owner_(owner),
    inventoryInfo_(inventoryInfo),
    characterEquipments_(characterEquipments),
    creatureEquipments_(creatureEquipments)
{
    for (EquipCode equipCode : creatureEquipments) {
        if (isValidEquipCode(equipCode)) {
            const gdt::equip_t* equip = EQUIP_TABLE->getEquip(equipCode);
            EquipPart part = getEquipPart(equipCode);
            equipPartTypeMap_.insert(
                EquipPartTypeMap::value_type(part, toEquipType(equip->equip_type())));
        }
    }
}


ErrorCode EquipInventory::equip(EquipCode& equipCode, ObjectId itemId)
{
    ErrorCode errorCode = ecOk;

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        errorCode = equip_i(equipCode, itemId);
    }    

    if (isSucceeded(errorCode)) {
        go::PassiveSkillCastable* passiveSkillCastable = owner_.queryPassiveSkillCastable();
        if (passiveSkillCastable) {
            passiveSkillCastable->notifyChangeCondition(pccEquip);
        }
        fullCreatureStatusChanged();
        owner_.queryAchievementable()->updateAchievement(acmtEquip);
    }

    return errorCode;
}


ErrorCode EquipInventory::unequip(EquipCode& equipCode, SlotId& slotId,
    ObjectId itemId)
{
    ErrorCode errorCode = ecOk;

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        errorCode = unequip_i(equipCode, slotId, itemId);
    }

    if (isSucceeded(errorCode)) {
        go::PassiveSkillCastable* passiveSkillCastable = owner_.queryPassiveSkillCastable();
        if (passiveSkillCastable) {
            passiveSkillCastable->notifyChangeCondition(pccEquip);
        }
        fullCreatureStatusChanged();
    }

    return errorCode;
}


ErrorCode EquipInventory::upgradeEquipItem(ObjectId equipId)
{
    EquipCode nextEquipCode = invalidEquipCode;
    uint8_t socketCount = 0;    
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        ItemInfo* equipitem = inventoryInfo_.getItemInfo(equipId);
        
        if (! equipitem) {
            SNE_LOG_ERROR("EquipInventory::upgradeEquipItem(A%" PRIu64 ") - character(C%" PRIu64 ") not find item(ID: %" PRIu64 ")",
                owner_.getAccountId(), owner_.getObjectId(), equipId);
            return ecInventoryItemNotFound;
        }
        
        const gdt::equip_t* equipTemplate = EQUIP_TABLE->getEquip(equipitem->itemCode_);
        if (! equipTemplate) {
            return ecServerInternalError;
        }

        if (equipTemplate->next_upgrade_code() == equipitem->itemCode_) {
            return ecItemMaxUpgradeEquipItem;
        }

        const FragmentCode fragmentCode = equipTemplate->fragment_code();
        if (! isValidFragmentCode(fragmentCode)) {
            return ecServerInternalError;
        }

        const BaseItemInfo fragmentItemInfo(fragmentCode, 1);
        if (! owner_.queryInventoryable()->getInventory().checkHasItemsByBaseItemInfo(fragmentItemInfo)) {
            return ecItemFragmentNotEnough;
        }

        owner_.queryInventoryable()->getInventory().useItemsByBaseItemInfo(fragmentItemInfo);

        if (isValidDataCode(equipTemplate->need_upgrade_item_code())) {
            const BaseItemInfo needItemInfo(equipTemplate->need_upgrade_item_code(), equipTemplate->need_upgrade_item_count());
            if (! owner_.queryInventoryable()->getInventory().checkHasItemsByBaseItemInfo(needItemInfo)) {
                return ecItemHasNotUpgradeNeedItem;
            }
            owner_.queryInventoryable()->getInventory().useItemsByBaseItemInfo(needItemInfo);
        }
        
        if (! isSuccessRate(equipTemplate->upgrade_rate())) {
            return ecOk;
        }

        if (equipitem->isEquipped()) {
            owner_.queryLiveable()->getCreatureStatus().equipItemChanged(equipitem->itemCode_, equipitem->equipItemInfo_, 
                equipTemplate->next_upgrade_code(), equipitem->equipItemInfo_);
            fullCreatureStatusChanged();
        }

        socketCount = equipitem->equipItemInfo_.socketCount_;
        if (equipitem->equipItemInfo_.socketCount_ == 0 && equipTemplate->max_socket_count() != 0) {
            socketCount = equipitem->equipItemInfo_.socketCount_ = static_cast<uint8_t>(esut::random(equipTemplate->min_socket_count(), equipTemplate->max_socket_count()));
        }
        nextEquipCode = equipitem->itemCode_ = equipTemplate->next_upgrade_code();
    }

    gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
    if (callback) {
        callback->equipItemInfoChanged(equipId, nextEquipCode, socketCount);
    }

    return ecOk;
}


ErrorCode EquipInventory::downgradeEquipItem(bool& shouldRemove, ObjectId equipId)
{
    shouldRemove = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        ItemInfo* equipitem = inventoryInfo_.getItemInfo(equipId);

        if (! equipitem) {
            SNE_LOG_ERROR("EquipInventory::upgradeEquipItem(A%" PRIu64 ") - character(C%" PRIu64 ") not find item(ID: %" PRIu64 ")",
                owner_.getAccountId(), owner_.getObjectId(), equipId);
            return ecInventoryItemNotFound;
        }

        const gdt::equip_t* equipTemplate = EQUIP_TABLE->getEquip(equipitem->itemCode_);
        if (! equipTemplate) {
            return ecServerInternalError;
        }

        if (equipitem->isEquipped()) {
            assert(false);
            return ecServerInternalError;
        }

        if (equipTemplate->pre_upgrade_code() == equipitem->itemCode_) {
            shouldRemove = true;
        }
        else {
            equipitem->itemCode_ = equipTemplate->pre_upgrade_code();
            gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
            if (callback) {
                callback->equipItemInfoChanged(equipId, equipitem->itemCode_, equipitem->equipItemInfo_.socketCount_);
            }
        }
    }

    return ecOk;
}


ErrorCode EquipInventory::upgradeRewardEquippedItem(EquipCode newEquipCode)
{
        // TODO: Status
    newEquipCode;
	/*uint8_t openSocketCount = 0;
	uint8_t fragmentCount = 0;
	const EquipPart part = getEquipPart(newEquipCode);
    ObjectId itemId = owner_.queryInventoryable()->getEquipedItemId(part);

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        EquipItemInfo* equipInfo = inventoryInfo_.getEquipItemInfo(itemId);
        ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
        if (itemInfo == nullptr) {
            SNE_LOG_ERROR("EquipInventory::upgradeEquipItem(A%" PRIu64 ") - character(C%" PRIu64 ") not find item(ID: %" PRIu64 ")",
                owner_.getAccountId(), owner_.getObjectId(), itemId);
            return ecItemNotExist;
        }
        if (equipInfo == nullptr) {
            SNE_LOG_ERROR("EquipInventory::upgradeEquipItem(A%" PRIu64 ") - character(C%" PRIu64 ") not find item(ID: %" PRIu64 ", Code: %u)",
                owner_.getAccountId(), owner_.getObjectId(), itemId, itemInfo->itemCode_);
            return ecItemNotExist;
        }

		const gdt::equip_t* equip = EQUIP_TABLE->getEquip(itemInfo->itemCode_);
		if (equip == nullptr) {
			return ecItemInvalidEquiptItem;
		}

        const gdt::equip_t* upgradeEquip = EQUIP_TABLE->getEquip(newEquipCode);
        if (upgradeEquip == nullptr) {
            return ecItemInvalidEquiptItem;
        }

        if (itemInfo->isEquipped()) {
			changeEquipItemStat(itemInfo->itemCode_, newEquipCode);
        }

		equipInfo->fragmentCount_ = fragmentCount = uint8_t(upgradeEquip->original_fragment());
		itemInfo->itemCode_ = newEquipCode;

		const uint8_t newOpenSlot = static_cast<uint8_t>(upgradeEquip->open_socket_count());
		const uint8_t openSlot = newOpenSlot - static_cast<uint8_t>(equip->open_socket_count());
		if (equipInfo->openSocketCount_ == 0) {
			for (uint8_t i = 0; i < openSlot; ++i) {
				SkillCode skillCode = equipInfo->skills_[equipInfo->openSocketCount_ + i];
				if (isValidSkillCode(skillCode)) {
					owner_.queryInventoryable()->enchant(getSkillIndex(skillCode), getSkillLevel(skillCode));
				}
			}
		}
		else {
			for (uint8_t i = equipInfo->openSocketCount_; i < newOpenSlot; ++i) {
				SkillCode skillCode = equipInfo->skills_[i];
				if (isValidSkillCode(skillCode)) {
					owner_.queryInventoryable()->enchant(getSkillIndex(skillCode), getSkillLevel(skillCode));
				}
			}
		}
		equipInfo->openSocketCount_ = upgradeEquip->open_socket_count();  
		openSocketCount = newOpenSlot;
    }

	gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
	if (callback) {
		callback->equipItemInfoChanged(itemId, newEquipCode);
	}
*/
	return ecOk;
}


bool EquipInventory::isMeleeWeaponEquipped() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    EquipPartTypeMap::const_iterator twoPos = equipPartTypeMap_.find(epTwoHands);
    if (twoPos != equipPartTypeMap_.end()) {
        EquipType equipType = (*twoPos).second;
        if (isBowType(equipType)) {
            return false;
        }
        return true;
    }

    EquipPartTypeMap::const_iterator onePos = equipPartTypeMap_.find(epRightHand);
    if (onePos != equipPartTypeMap_.end()) {
        return true;
    }
    return false;
}


bool EquipInventory::isShieldEquipped() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return isValidObjectId(characterEquipments_[epLeftHand]);
}


uint32_t EquipInventory::getEquipCountByItemGrade(ItemGrade grade) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    uint32_t count = 0;
    for (EquipCode equipCode : creatureEquipments_) {
        const gdt::equip_t* equip = EQUIP_TABLE->getEquip(equipCode);
        if (equip) {
            if (! isValid(grade)) {
                ++count;
            }
            else if (grade == toItemGrade(equip->item_grade())) {
                ++count;
            }
        }
    }
    return count;
}


FragmentCode EquipInventory::getEquipFragmentCode(EquipPart part) const
{
    if (! isValid(part)) {
        return invalidFragmentCode;
    }

    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    EquipCode equipCode = creatureEquipments_[part];
    if (isValidEquipCode(equipCode)) {
        const gdt::equip_t* equip = EQUIP_TABLE->getEquip(equipCode);
        if (equip) {
            return equip->fragment_code();
        }
    }
    return invalidFragmentCode;
}


ErrorCode EquipInventory::enchant(bool& isSuccess, ObjectId equipItemId, ObjectId gemId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    isSuccess = false;
    
    ItemInfo* equipItem = inventoryInfo_.getItemInfo(equipItemId);
    if (! equipItem) {
        return ecInventoryItemNotFound;
    }

    if (! equipItem->equipItemInfo_.canAddGemItem()) {
        return ecItemNotEnoughEnchantSlot;
    }

    ItemInfo* gemItem = inventoryInfo_.getItemInfo(gemId);
    if (! gemItem) {
        return ecInventoryItemNotFound;
    }

    const gdt::gem_t* gem= GEM_TABLE->getGem(gemItem->itemCode_);
    if (! gem) {
        return ecServerInternalError;
    }
    
    const ItemScriptInfos optionInfos = ItemOptionService::getItemOptions(gem->item_suffix_id());
    if (optionInfos.empty()) {
        assert(false);
        return ecServerInternalError;
    }
    EquipSocketInfo equipSocketInfo(gem->item_code(), optionInfos);

    SocketSlotId emptySlot = equipItem->equipItemInfo_.getEmptySocketSlotId();
    if (! isValidSocketSlotId(emptySlot)) {
        return ecServerInternalError;
    }
    const EquipItemInfo oldEquipInfo = equipItem->equipItemInfo_; 
    isSuccess = true;
    equipItem->equipItemInfo_.addGemItem(emptySlot, equipSocketInfo);
    if (equipItem->isEquipped()) {
        owner_.queryLiveable()->getCreatureStatus().equipItemChanged(equipItem->itemCode_, oldEquipInfo, 
            equipItem->itemCode_, equipItem->equipItemInfo_);
        fullCreatureStatusChanged();
    }

    owner_.queryInventoryable()->getInventory().useItemsByItemId(gemId, 1);
    gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
    if (callback) {
        callback->equipItemEnchanted(equipItemId, emptySlot, equipSocketInfo);
    }

    return ecOk;
}


ErrorCode EquipInventory::unenchant(ObjectId equipItemId, SocketSlotId slotId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    ItemInfo* equipItem = inventoryInfo_.getItemInfo(equipItemId);
    if (! equipItem) {
        return ecInventoryItemNotFound;
    }

    if (! equipItem->equipItemInfo_.hasSocketOption(slotId)) {
        return ecItemEnchantHasSlotInvalidSlot;
    }

    const EquipItemInfo oldEquipInfo = equipItem->equipItemInfo_; 
    equipItem->equipItemInfo_.removeGemItem(slotId);
    if (equipItem->isEquipped()) {
        owner_.queryLiveable()->getCreatureStatus().equipItemChanged(equipItem->itemCode_, oldEquipInfo, 
            equipItem->itemCode_, equipItem->equipItemInfo_);
        fullCreatureStatusChanged();
    }

    gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
    if (callback) {
        callback->equipItemUnenchanted(equipItemId, slotId);
    }

    
    return ecOk;
}


ErrorCode EquipInventory::equip_i(EquipCode& equipCode, ObjectId itemId)
{
    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (! itemInfo) {
        return ecInventoryItemNotFound;
    }

    if (itemInfo->isEquipped()) {
        return ecEquipItemAlreadyEquipped;
    }

    const gdt::equip_t* equip = EQUIP_TABLE->getEquip(itemInfo->itemCode_);
    if (! equip) {
        return ecServerInternalError;
    }

    if (equip->min_level() > static_cast<go::Creature&>(owner_).getCreatureLevel()) {
        return ecEquipNotEnoughLevel;   
    }

    const EquipPart partForEquip = getEquipPart(itemInfo->itemCode_);
    assert(gideon::isValid(partForEquip));

    UnequipItemInfo unequipItemInfo;
    ErrorCode errorCode = unequipItem(unequipItemInfo, partForEquip, itemInfo->slotId_, false);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    equipItem(unequipItemInfo, equipCode, *itemInfo, partForEquip);

    return ecOk;
}


ErrorCode EquipInventory::unequip_i(EquipCode& equipCode, SlotId& slotId,
    ObjectId itemId)
{
    ItemInfo* itemInfo = inventoryInfo_.getItemInfo(itemId);
    if (! itemInfo) {
        return ecInventoryItemNotFound;
    }

    if (! itemInfo->isEquipped()) {
        return ecUnequipItemNotEquipped;
    }

    if (slotId == equippedSlotId) {
        slotId = invalidSlotId;
    }

    const EquipPart partForUnequip = getEquipPart(itemInfo->itemCode_);
    assert(gideon::isValid(partForUnequip));

    UnequipItemInfo unequipItemInfo;
    const ErrorCode errorCode = unequipItem(unequipItemInfo, partForUnequip, slotId, true);
    if (isSucceeded(errorCode)) {
        slotId = itemInfo->slotId_;
        equipCode = itemInfo->itemCode_;
        gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
        if (callback) {
            callback->unequipItemReplaced(itemInfo->itemId_, slotId, partForUnequip);
        }
    }
    return errorCode;
}


void EquipInventory::equipItem(const UnequipItemInfo& unequipItemInfo, EquipCode& equipCode,
    ItemInfo& itemInfo, EquipPart partForEquip)
{
    setEquipCreatureInfo(itemInfo.itemId_, itemInfo.itemCode_, partForEquip);
    itemInfo.setSlotId(equippedSlotId);
    equipCode = itemInfo.itemCode_;

    gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
    if (callback) {
        if (unequipItemInfo.isValid()) {
            callback->inventoryWithEquipItemReplaced(unequipItemInfo.objectId_, 
                unequipItemInfo.unequipPart_, itemInfo.itemId_, partForEquip);        
        }
        else {
            callback->equipReplaced(itemInfo.itemId_, partForEquip);
        }
    }
}


ErrorCode EquipInventory::unequipItem(UnequipItemInfo& unequipItemInfo, EquipPart partForEquip,
    SlotId slotId, bool onlyUnequip)
{   
    const ErrorCode errorCode = checkUnequipSlotState(partForEquip, onlyUnequip);
    if (isFailed(errorCode)) {
        return errorCode;
    }	

    if (partForEquip == epTwoHands) {
        return unequipByTwoHandItem(unequipItemInfo, slotId);
    }
	else if (isOneHandWeapon(partForEquip)  && isEquipped(epTwoHands)) {
		return unequipItem(unequipItemInfo, epTwoHands, slotId);    
	}
    
    return unequipItem(unequipItemInfo, partForEquip, slotId);
}


ErrorCode EquipInventory::unequipByTwoHandItem(UnequipItemInfo& unequipItemInfo, SlotId slotId)
{
    if (isEquipped(epTwoHands)) {
        return unequipItem(unequipItemInfo, epTwoHands, slotId);
    }
    else if (isEquipped(epLeftHand) && isEquipped(epRightHand)) {
        const ErrorCode errorCode = unequipItem(unequipItemInfo, epLeftHand, slotId);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        return unequipItem(unequipItemInfo, epRightHand, invalidSlotId);                        
    }    
    else if (isEquipped(epLeftHand)) {
        return unequipItem(unequipItemInfo, epLeftHand, slotId);
    }    
    else if (isEquipped(epRightHand)) {
        return unequipItem(unequipItemInfo, epRightHand, slotId);
    }
    
    return ecOk;
}


ErrorCode EquipInventory::unequipItem(UnequipItemInfo& unequipItemInfo, EquipPart part, SlotId slotId)
{
    if (! inventoryInfo_.isStorable(slotId)) {
        slotId = inventoryInfo_.getFirstEmptySlotId();
    }
    
    const bool shouldApplyStats = true;
    const ObjectId uneqipItemId = setUnequipCreatureInfo(part, shouldApplyStats);
    if (isValidObjectId(uneqipItemId)) {
        ItemInfo* itemInfo = inventoryInfo_.getItemInfo(uneqipItemId);
        if (! itemInfo) {
            assert(false);
            return ecInventoryItemNotFound;
        }

        itemInfo->setSlotId(slotId);
        if (unequipItemInfo.isValid()) {
            gc::EquipInventoryCallback* callback = owner_.getController().queryEquipInventoryCallback();
            if (callback) {
                callback->unequipItemReplaced(itemInfo->itemId_, slotId, part);
            }
        }
        else {
            unequipItemInfo.objectId_ = itemInfo->itemId_;
            unequipItemInfo.unequipPart_ = part;
        }    
    }

    return ecOk;
}


void EquipInventory::setEquipCreatureInfo(ObjectId equipItemId, EquipCode equipCode, EquipPart part)
{    
    const gdt::equip_t* equipInfo = EQUIP_TABLE->getEquip(equipCode);
    if (equipInfo) {
        characterEquipments_[part] = equipItemId;
        creatureEquipments_[part] = equipCode;
        equipPartTypeMap_.insert(EquipPartTypeMap::value_type(part,
            toEquipType(equipInfo->equip_type())));
        ItemInfo* itemInfo = inventoryInfo_.getItemInfo(equipItemId);
        if (! itemInfo) {
            assert(false);
            return ;
        }
        owner_.queryLiveable()->getCreatureStatus().equipItemEquipped(equipCode, itemInfo->equipItemInfo_);
    }
    else {
        assert(false);
    }
}



ObjectId EquipInventory::setUnequipCreatureInfo(EquipPart part, bool shouldApplyStats)
{
    ObjectId unequipItemId = characterEquipments_[part];
    if (! isValidObjectId(unequipItemId)) {
        return unequipItemId;
    }

    characterEquipments_[part] = invalidObjectId;
    equipPartTypeMap_.erase(part);
    assert(isValidEquipCode(creatureEquipments_[part]));
    EquipCode equipCode = creatureEquipments_[part];
    creatureEquipments_[part] = invalidEquipCode;
    
    if (shouldApplyStats) {
        const ItemInfo* itemInfo = inventoryInfo_.getItemInfo(unequipItemId);
        if (! itemInfo) {
            assert(false);
            return unequipItemId;
        }
        owner_.queryLiveable()->getCreatureStatus().equipItemUnequipped(equipCode, itemInfo->equipItemInfo_);
    }

    return unequipItemId;
}


void EquipInventory::fullCreatureStatusChanged()
{
    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    assert(callback != nullptr);
    if (callback != nullptr) {
        callback->fullCreatureStatusChanged();
    }
}


ErrorCode EquipInventory::checkUnequipSlotState(EquipPart partForEquip, bool onlyUnequip) const
{
    const uint32_t reserveEmptySlotCount = onlyUnequip ? 0 : 1;
    const uint32_t secureEmptySlot  = 
        inventoryInfo_.getEmptySlotCount() + reserveEmptySlotCount;
    const uint32_t twoHandSlot = 2;
    const uint32_t normalSlot = 1;

    if (partForEquip == epTwoHands) {
        if (isEquipped(epLeftHand) && isEquipped(epRightHand)) {
            if (twoHandSlot > secureEmptySlot) {
                return ecInventoryIsFull;
            }
        }
    }
    
    return normalSlot <= secureEmptySlot ? ecOk : ecInventoryIsFull;
}


void EquipInventory::removeEquipItem(ObjectId itemId)
{
    inventoryInfo_.removeItem(itemId);

    gc::InventoryCallback* callback = owner_.getController().queryInventoryCallback();
    if (callback) {
        callback->inventoryItemRemoved(itPc, itemId);
    }
}


}} // namespace gideon { namespace zoneserver {
