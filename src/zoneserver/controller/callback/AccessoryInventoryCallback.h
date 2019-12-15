#pragma once

#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/AccessoryInfo.h>


namespace gideon { namespace zoneserver { namespace gc {


/***
 * @class AssessoryInventoryCallback
 ***/
class AccessoryInventoryCallback
{
public:
	virtual ~AccessoryInventoryCallback() {}

    virtual void unequipAccessoryItemReplaced(ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart) = 0;
    virtual void equipAccessoryReplaced(ObjectId itemId, AccessoryIndex equipPart) = 0;
    virtual void inventoryWithAccessoryItemReplaced(ObjectId unequipItemId,
        AccessoryIndex unequipPart, ObjectId equipItemId, AccessoryIndex equipPart) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace gc {