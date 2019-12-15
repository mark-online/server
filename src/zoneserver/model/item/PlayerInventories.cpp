#include "ZoneServerPCH.h"
#include "PlayerInventories.h"
#include "../gameobject/Entity.h"
#include "../gameobject/ability/Liveable.h"
#include "../gameobject/ability/PassiveSkillCastable.h"
#include "../gameobject/status/CreatureStatus.h"
#include "esut/Random.h"

namespace gideon { namespace zoneserver {

namespace {

Inventory::LostType getLostType()
{
	uint32_t value = esut::random(0, 1000);
	if (value > 950) {
		return PlayerInventories::ltDelete;
	}
	else if (value > 150) {
		return PlayerInventories::ltProtect;
	}
	return PlayerInventories::ltDrop;
}

} // namespace {

PlayerInventories::PlayerInventories(go::Entity& owner, InventoryInfo& inventory,
    CharacterEquipments& characterEquipments, CreatureEquipments& creatureEquipments,
    CharacterAccessories& accessorys) :
    Inventory(owner, inventory, itPc),
    EquipInventory(owner, inventory, characterEquipments, creatureEquipments),
    AccessoryInventory(owner, inventory, accessorys),
    owner_(owner)
{
}


void PlayerInventories::robbed(LootInvenItemInfoMap& itemMap)
{
    bool isUnequipped = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! owner_.queryLiveable()->getCreatureStatus().isDied()) {
            return;
        }
        robInventoryItem(itemMap);
    }

    if (isUnequipped) {
        go::PassiveSkillCastable* passiveSkillCastable = owner_.queryPassiveSkillCastable();
        if (passiveSkillCastable) {
            passiveSkillCastable->notifyChangeCondition(pccEquip);
        }
    }
}


void PlayerInventories::robInventoryItem(LootInvenItemInfoMap& itemInfoMap)
{
    InventoryInfo& inventory = getInventoryInfo();
    ItemMap& inventoryItemMap = inventory.items_; 
    ItemMap::iterator pos = inventoryItemMap.begin();
    while (pos != inventoryItemMap.end()) {
        ItemInfo& itemInfo =  (*pos).second;
        if (itemInfo.isEquipped()) {
            ++pos;
            continue;
        }
        // 지워지지 않은 아이템 갯수(남한테 준다)
        LostType type = getLostType();
		if (ltProtect == type) {
			++pos;
			continue;
		}
		
		if (ltDrop == type) {
			LootInvenItemInfo lootItemInfo(liitItem);
			lootItemInfo.baseItemInfo_ = itemInfo;
			LootInvenId invenId = toLootInvenId(itemInfoMap.size() + 1);
			if (itemInfo.isEquipment()) {
				lootItemInfo.equipItemInfo_ = itemInfo.equipItemInfo_;
			}
            else if (itemInfo.isAccessory()) {                
                lootItemInfo.accessoryInfo_ = itemInfo.accessoryItemInfo_;
            }
			itemInfoMap.emplace(invenId, lootItemInfo);
		}
        
        removeItem_i(itemInfo.itemId_);
        pos = inventoryItemMap.begin();
    }
}


}} // namespace gideon { namespace zoneserver {
