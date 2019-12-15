#pragma once

#include <gideon/cs/shared/data/GuildInventoryInfo.h>

namespace gideon { namespace communityserver {

/**
 * @class GuildInventoryCallback
 */
class GuildInventoryCallback
{
public:
    virtual void vaultAdded(const BaseVaultInfo& info) = 0;
    virtual void vaultNameUpdated(VaultId vaultId, const VaultName& name) = 0;

    virtual void inventoryItemCountUpdated(VaultId vaultId, ObjectId itemId, uint8_t count) = 0;
    virtual void inventoryItemAdded(VaultId vaultId, const ItemInfo& itemInfo) = 0;
    virtual void inventoryItemRemoved(VaultId vaultId, ObjectId itemId) = 0;
    virtual void inventoryItemSwitched(VaultId vaultId, ObjectId itemId1, SlotId slotId1, ObjectId itemId2, SlotId slotId2) = 0;
    virtual void inventoryItemMoved(VaultId vaultId, ObjectId itemId, SlotId slotId) = 0;
};

}}