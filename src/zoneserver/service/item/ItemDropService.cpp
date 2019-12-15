#include "ZoneServerPCH.h"
#include "ItemDropService.h"
#include "ItemOptionService.h"
#include "../../helper/InventoryHelper.h"
#include <gideon/cs/datatable/ItemDropTable.h>
#include <gideon/cs/datatable/WorldDropTable.h>
#include <gideon/cs/datatable/WorldDropSuffixTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/EquipTable.h>

namespace gideon { namespace zoneserver {

void ItemDropService::fillItemDropInfo(LootInvenItemInfoMap& lootInvenItemInfoMap, ItemDropId itemDropId)
{
    const datatable::ItemDropTemplate* itemDropTemplate = ITEM_DROP_TABLE->getItemDrop(itemDropId);
    if (! itemDropTemplate) {
        return;
    }

    const datatable::ItemDropInfos& itemDropInfos = itemDropTemplate->getItemDropInfos();
    for (const datatable::ItemDropInfo& dropInfo : itemDropInfos) {
        if (! dropInfo.shouldDrop()) {
            continue;
        }
        LootInvenItemInfo invenItemInfo;
        if (dropInfo.itemDropType_ == idtItem) {
            invenItemInfo.type_ = liitItem;
            CodeType ct = getCodeType(dropInfo.itemCode_);
            if (isAccessoryType(ct)) {
                const gdt::accessory_t* accessory = ACCESSORY_TABLE->getAccessory(dropInfo.itemCode_);
                if (! accessory) {
                    assert(false);
                    continue;
                }
                invenItemInfo.accessoryInfo_ = AccessoryItemInfo(ItemOptionService::getItemOptions(accessory->item_suffix_id()));

            }
            else if (isEquipmentType(ct)) {
                const gdt::equip_t* equip = EQUIP_TABLE->getEquip(dropInfo.itemCode_);
                if (! equip) {
                    assert(false);
                    continue;
                }
                const uint8_t socketCount = static_cast<uint8_t>(esut::random(uint32_t(equip->min_socket_count()), uint32_t(equip->max_socket_count())));
                invenItemInfo.equipItemInfo_ = EquipItemInfo(socketCount, ItemOptionService::getItemOptions(equip->item_suffix_id()));
            }

            uint8_t maxStackCount = getStackItemCount(dropInfo.itemCode_);
            uint8_t randomStackCount = static_cast<uint8_t>(dropInfo.getRandomValue());
            uint8_t stackCount = maxStackCount > randomStackCount ? randomStackCount : maxStackCount;
            invenItemInfo.baseItemInfo_.itemCode_ = dropInfo.itemCode_;
            invenItemInfo.baseItemInfo_.count_ = stackCount;
        }
        else if (dropInfo.itemDropType_ == idtExp) {
            invenItemInfo.type_ = liitExp;
            invenItemInfo.exp_ = toExpPoint(dropInfo.getRandomValue());
        }
        else if (dropInfo.itemDropType_ == idtGameMoney) {
            invenItemInfo.type_ = liitGameMoney;
            invenItemInfo.money_ = static_cast<GameMoney>(dropInfo.getRandomValue());
        }
        if (isValid(invenItemInfo.type_)) {
            LootInvenId lootId = toLootInvenId(lootInvenItemInfoMap.size() + 1);
            lootInvenItemInfoMap.emplace(lootId, invenItemInfo);
        }
    }
}


void ItemDropService::fillWorldDropInfo(LootInvenItemInfoMap& lootInvenItemInfoMap, WorldDropSuffixId suffixId)
{
    datatable::WorldDropSuffixTemplate itemDropTemplate = WORLD_DROP_SUFFIX_TABLE->getWorldDropSuffixTemplate(suffixId);
    if (! itemDropTemplate.isValid()) {
        return;
    }


    uint32_t loofCount = itemDropTemplate.getLoopCount();
    for (uint32_t i = 0; i < loofCount; ++i) {
        LootInvenItemInfo invenItemInfo;
        invenItemInfo.type_ = liitItem;
        if (itemDropTemplate.shouldDrop()) {
            WorldDropId dropId = itemDropTemplate.getRandomWorldDropId();
            DataCode itemCode = WORLD_DROP_TABLE->getRandomCode(dropId);
            CodeType ct = getCodeType(itemCode);
            if (isAccessoryType(ct)) {
                const gdt::accessory_t* accessory = ACCESSORY_TABLE->getAccessory(itemCode);
                if (! accessory) {
                    assert(false);
                    continue;
                }
                invenItemInfo.accessoryInfo_ = AccessoryItemInfo(ItemOptionService::getItemOptions(accessory->item_suffix_id()));
            }
            else if (isEquipmentType(ct)) {
                const gdt::equip_t* equip = EQUIP_TABLE->getEquip(itemCode);
                if (! equip) {
                    assert(false);
                    continue;
                }
                const uint8_t socketCount = static_cast<uint8_t>(esut::random(uint32_t(equip->min_socket_count()), uint32_t(equip->max_socket_count())));
                invenItemInfo.equipItemInfo_ = EquipItemInfo(socketCount, ItemOptionService::getItemOptions(equip->item_suffix_id()));
            }
            invenItemInfo.baseItemInfo_.itemCode_ = itemCode;
            invenItemInfo.baseItemInfo_.count_ = 1;
            if (isValidDataCode(invenItemInfo.baseItemInfo_.itemCode_ )) {
                LootInvenId lootId = toLootInvenId(lootInvenItemInfoMap.size() + 1);
                lootInvenItemInfoMap.emplace(lootId, invenItemInfo);
            }            
        }
    }
}

}} // namespace gideon { namespace zoneserver {