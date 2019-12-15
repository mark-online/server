#pragma once

#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/SkillInfo.h>



namespace gideon { namespace zoneserver { namespace gc {


/***
 * @class EquipInventoryCallback
 ***/
class EquipInventoryCallback
{
public:
	virtual ~EquipInventoryCallback() {}

	virtual void unequipItemReplaced(ObjectId itemId, SlotId slotId, EquipPart unequipPart) = 0;
	virtual void equipReplaced(ObjectId itemId, EquipPart equipPart) = 0;
	virtual void inventoryWithEquipItemReplaced(ObjectId unequipItemId,
		EquipPart unequipPart, ObjectId equipItemId, EquipPart equipPart) = 0;
	virtual void equipItemInfoChanged(ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount) = 0;
    virtual void equipItemEnchanted(ObjectId itemId, SocketSlotId id, EquipSocketInfo& socketInfo) = 0;
	virtual void equipItemUnenchanted(ObjectId itemId, SocketSlotId id) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace gc {