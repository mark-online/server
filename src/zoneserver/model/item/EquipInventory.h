#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/FragmentInfo.h>
#include <gideon/cs/shared/data/GraveStoneInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/EquipmentInfo.h>
#include <boost/noncopyable.hpp>

namespace gdt {
class equip_t;
}

namespace gideon {
class EquipEnchantManager;
} // namespace gideon {

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go 

/**
 * @struct UnequipItemInfo 
 */
struct UnequipItemInfo
{
    ObjectId objectId_;
    EquipPart unequipPart_;

    UnequipItemInfo() :
        objectId_(invalidObjectId),
        unequipPart_(epInvalid) {}

    bool isValid() const {
        return isValidObjectId(objectId_) && 
            gideon::isValid(unequipPart_);
    }
};


/**
 * @class EquipInventory 
 */
class ZoneServer_Export EquipInventory : public boost::noncopyable
{
    typedef std::vector<ObjectId> EnequipIds;
public:
    EquipInventory(go::Entity& owner, InventoryInfo& inventory,
        CharacterEquipments& characterEquipments,
        CreatureEquipments& creatureEquipments);
    virtual ~EquipInventory() {}

public:
    ErrorCode equip(EquipCode& equipCode, ObjectId itemId);
    ErrorCode unequip(EquipCode& equipCode, SlotId& slotId, ObjectId itemId);

    ErrorCode upgradeEquipItem(ObjectId equipId);
    ErrorCode downgradeEquipItem(bool& shouldRemove, ObjectId equipId);

    ErrorCode enchant(bool& isSuccess, ObjectId equipItemId, ObjectId gemId);
    ErrorCode unenchant(ObjectId equipItemId, SocketSlotId slotId);

    ErrorCode upgradeRewardEquippedItem(EquipCode newEquipCode);

    const EquipPartTypeMap& getEquipPartTypeMap() const {
        return equipPartTypeMap_;
    }

    virtual bool isMeleeWeaponEquipped() const;
    virtual bool isShieldEquipped() const;

    uint32_t getEquipCountByItemGrade(ItemGrade grade) const;

    FragmentCode getEquipFragmentCode(EquipPart part) const;

protected:
    ErrorCode equip_i(EquipCode& equipCode, ObjectId itemId);
    ErrorCode unequip_i(EquipCode& equipCode, SlotId& slotId, ObjectId itemId);

    void equipItem(const UnequipItemInfo& unequipItemInfo, EquipCode& equipCode,
        ItemInfo& itemInfo, EquipPart partForEquip);
    ErrorCode unequipItem(UnequipItemInfo& unequipItemInfo, EquipPart partForEquip,
        SlotId slotId, bool onlyUnequip);
    
    ErrorCode unequipByTwoHandItem(UnequipItemInfo& unequipItemInfo, SlotId slotId);
    ErrorCode unequipItem(UnequipItemInfo& unequipItemInfo, EquipPart part, SlotId slotId);
  
    void setEquipCreatureInfo(ObjectId equipItemId, EquipCode equipCode, EquipPart part);
    ObjectId setUnequipCreatureInfo(EquipPart part, bool shouldApplyStats);

    void fullCreatureStatusChanged();

    ErrorCode checkUnequipSlotState(EquipPart partForEquip, bool onlyUnequip) const;

private:
    void removeEquipItem(ObjectId itemId);

private:
    bool isEquipped(EquipPart part) const {
        return isValidObjectId(characterEquipments_[part]);
    }

private:
    go::Entity& owner_;

    InventoryInfo& inventoryInfo_;
    CharacterEquipments& characterEquipments_;
    CreatureEquipments& creatureEquipments_;

    EquipPartTypeMap equipPartTypeMap_;
};


}} // namespace gideon { namespace zoneserver {
