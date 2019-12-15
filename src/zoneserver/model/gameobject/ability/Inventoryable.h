#pragma once

#include <gideon/cs/shared/data/GraveStoneInfo.h>
#include <gideon/cs/shared/data/AccessoryInfo.h>

namespace gideon { namespace zoneserver {

class Inventory;
class EquipInventory;
class AccessoryInventory;
class QuestInventory;
class BankAccount;

}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Inventoryable
 * 인벤토리를 사용할수 있다
 */
class Inventoryable
{
public:
    virtual ~Inventoryable() {}
	
public:
	virtual Inventory& getInventory() const = 0;
	virtual Inventory& getVehicleInventory() const = 0;
	virtual EquipInventory& getEquipInventory() const = 0;
	virtual AccessoryInventory& getAccessoryInventory() const = 0;
	virtual QuestInventory& getQuestInventory() const = 0;
	virtual BankAccount& getBankAccount() const = 0;
	virtual AccessoryIndex getAccessoryIndex(ObjectId itemId) const = 0;;
	virtual ObjectId getEquipedItemId(EquipPart part) const = 0;
	virtual InventoryInfo getInventoryInfo() const = 0;

	virtual bool isEquipAccessory(AccessoryIndex index) const = 0;
    virtual bool canExtendSlot(bool isCashSlot, uint8_t extendCount) const = 0;
	virtual bool isOpenOutsideInventory(InvenType invenType) const = 0;

    virtual uint8_t extendSlot(bool isCashSlot, uint8_t extendCount) = 0;
	virtual void robbed(LootInvenItemInfoMap& infoMap) = 0;
	virtual void openOutsideInventory(InvenType invenType) = 0;
	virtual void closeOutsideInventory(InvenType invenType) = 0;
	virtual void setAccessoryIndex(AccessoryIndex index, ObjectId itemId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
