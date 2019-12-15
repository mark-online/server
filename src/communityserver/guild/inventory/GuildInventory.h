#pragma once

#include <gideon/cs/shared/data/GuildInventoryInfo.h>
#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace communityserver {

class GuildInventoryCallback;

/***
 * class CommunityGuild
 ***/
class GuildInventory
{
public:
    GuildInventory(GuildInventoryCallback& callback);

    void setInventoryInfo(const GuildInventoryInfo& inventoryInfo) {
        inventoryInfo_ = inventoryInfo;
    }

    ErrorCode addVault(const BaseVaultInfo& vaultInfo);

public:
    ErrorCode pushItem(ItemInfo itemInfo, VaultId vaultId, SlotId slotId = invalidSlotId);
    ErrorCode popItem(VaultId vaultId, ObjectId itemId);
    ErrorCode switchItem(VaultId vaultId, ObjectId itemId1, ObjectId itemId2);
    ErrorCode moveItem(VaultId vaultId, ObjectId itemId, SlotId slotId);

    ErrorCode updateVaultName(VaultId vaultId, const VaultName& name);
    
public:
    void fillInventoryInfo(InventoryInfo& invenInfo, VaultId vaultId) const;

public:
    ItemInfo getItemInfo(VaultId vaultId, ObjectId itemId) const;
    
    const VaultInfos& getVaults() const {
        return inventoryInfo_.vaultInfos_;
    }

    size_t getVaultCount() const {
        return inventoryInfo_.vaultInfos_.size();
    }

private:
    GuildInventoryCallback& callback_;
    GuildInventoryInfo inventoryInfo_;

};

}} // namespace gideon { namespace communityserver {