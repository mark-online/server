#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/AccessoryInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <boost/noncopyable.hpp>


namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go 


/**
 * @class AccessoryInventory
 */
class ZoneServer_Export AccessoryInventory : public boost::noncopyable
{
    typedef std::vector<ObjectId> EnequipIds;
public:
    AccessoryInventory(go::Entity& owner, InventoryInfo& inventory,
        CharacterAccessories& characterAccessories);
    virtual ~AccessoryInventory() {}

public:
    ErrorCode equipAccessory(ObjectId itemId, AccessoryIndex index);
    ErrorCode unequipAccessory(ObjectId itemId, SlotId slotId);

private:
    void equipAccessory(ItemInfo& equipItemInfo, const AccessoryItemInfo& equipAccessoryInfo, AccessoryIndex index);
    void swapAccessory(ItemInfo& equipItemInfo, const AccessoryItemInfo& equipAccessoryInfo,
        ItemInfo& unequipItemInfo, const AccessoryItemInfo& unequipAccessoryInfo, AccessoryIndex swapIndex);

private:
    go::Entity& owner_;

    InventoryInfo& inventoryInfo_;
    CharacterAccessories& characterAccessories_;
};


}} // namespace gideon { namespace zoneserver {