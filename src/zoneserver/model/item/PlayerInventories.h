#pragma once

#include "../../zoneserver_export.h"
#include "Inventory.h"
#include "EquipInventory.h"
#include "AccessoryInventory.h"
#include <boost/noncopyable.hpp>


namespace gideon { namespace zoneserver {


/**
 * @class PlayerInventories 
 */
class ZoneServer_Export PlayerInventories : public Inventory,
    public EquipInventory,
    public AccessoryInventory
{
    typedef std::vector<ObjectId> EnequipIds;
public:
    PlayerInventories(go::Entity& owner, InventoryInfo& inventory,
        CharacterEquipments& characterEquipments, CreatureEquipments& creatureEquipments,
        CharacterAccessories& accessorys);

    void robbed(LootInvenItemInfoMap& itemMap);

private:
    void robInventoryItem(LootInvenItemInfoMap& itemMap);

private:
    go::Entity& owner_;
};


}} // namespace gideon { namespace zoneserver {