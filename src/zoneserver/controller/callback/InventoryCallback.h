#pragma once

#include <gideon/cs/shared/data/InventoryInfo.h>


namespace gideon { namespace zoneserver { namespace gc {


/***
 * @class InventoryCallback
 ***/
class InventoryCallback
{
public:
	virtual ~InventoryCallback() {}

	virtual void inventoryItemMoved(InvenType invenType, ObjectId itemId, SlotId slotId, bool dbSave = true) = 0;
	virtual void inventoryItemSwitched(InvenType invenType, ObjectId itemId1, ObjectId itemId2, bool dbSave = true) = 0;
	virtual void inventoryItemAdded(InvenType invenType, const ItemInfo& itemInfo, bool dbSave = true) = 0;
	virtual void inventoryItemRemoved(InvenType invenType, ObjectId itemId, bool dbSave = true) = 0;
	virtual void inventoryItemCountUpdated(InvenType invenType, ObjectId itemId, uint8_t itemCount, bool dbSave = true) = 0;
    virtual void inventoryInfoUpdated(InvenType invenType, bool isCashSlot, uint8_t count, bool dbSave = true) = 0;
    virtual void inventoryItemAllRemoved(InvenType invenType) = 0; 
};


}}} // namespace gideon { namespace zoneserver { namespace gc {