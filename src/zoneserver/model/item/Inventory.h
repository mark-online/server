#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/EquipmentInfo.h>
#include <gideon/cs/shared/data/GraveStoneInfo.h>
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <boost/noncopyable.hpp>

namespace gdt {
class equip_t;
}

namespace gideon { namespace zoneserver {

struct AddItemInfo;

namespace go {
class Entity;
} // namespace go 

/**
 * @class Inventory 
 */
class ZoneServer_Export Inventory : public boost::noncopyable
{
    typedef std::vector<ObjectId> EnequipIds;

public:
	enum LostType {
		ltProtect,
		ltDrop,
		ltDelete,
	};

    Inventory(go::Entity& owner, InventoryInfo& inventory,
        InvenType invenType);
    virtual ~Inventory() {}

public:   
    ErrorCode moveItem(ObjectId itemId, SlotId slotId);
    ErrorCode switchItem(ObjectId itemId1, ObjectId itemId2);
    // param2 invalidSlotId이면 서버에서 임의로 넣어준다.
    ErrorCode addItem(const AddItemInfo& addItemInfo,
        SlotId slotId = invalidSlotId);
    ErrorCode addItem(ObjectId& addItemId, const AddItemInfo& addItemInfo);

    ErrorCode addMercenaryItem(const BaseItemInfo& baseItemInfo,
        uint8_t workday, SlotId slotId = invalidSlotId, ObjectId itemId = invalidObjectId);

	ErrorCode removeItem(ObjectId itemId);
	ErrorCode divideItem(ObjectId itemId, uint8_t count, SlotId slotId);
	
    // 빌딩에서 사용한다
    void fillHalfItem(BaseItemInfos& itemInfos);
    void removeAllItems();
	
public:
    ItemInfo getItemInfo(ObjectId itemId) const;
    
public:  
	void useItemsByBaseItemInfo(const BaseItemInfo& baseItemInfo);
    void useItemsByBaseItemInfos(const BaseItemInfos& baseItemInfos);
    bool useItemsByItemId(ObjectId itemId, uint8_t count);

    bool checkHasItemsByBaseItemInfo(const BaseItemInfo& baseItemInfo) const;
    bool checkHasItemsByBaseItemInfos(const BaseItemInfos& baseItemInfos) const;
    bool canAddItemByBaseItemInfo(const BaseItemInfo& baseItemInfo) const;
    bool canAddItemByBaseItemInfos(const BaseItemInfos& baseItemInfos) const;
    ErrorCode checkAddSlot(SlotId slotId) const;

public:    
    DataCode getItemCode(ObjectId itemId) const;

public:
	void setTempHarvestItems(const BaseItemInfos& baseItems);
	ErrorCode acquireHarvest(DataCode itemCode);

public:
    InventoryInfo& getInventoryInfo() {
        return inventoryInfo_;
    }

    const InventoryInfo& getInventoryInfo() const {
        return inventoryInfo_;
    }

protected:
    ErrorCode addItem_i(ObjectId& addObjectId, const AddItemInfo& addItemInfo, SlotId slotId);
    
    ErrorCode removeItem_i(ObjectId itemId);
    void swaipItem_i(ItemInfo& itemInfo1, ItemInfo& itemInfo2);

private:
    void useItems_i(DataCode dataCode, int needCount);
    void useItems_i(ItemInfo& itemInfo, uint8_t needCount);
private:
    void initCastItem();
    void insertCastItem(ObjectId itemId, DataCode itemCode);
    void removeCastItem(ObjectId itemId);

private:   
    go::Entity& owner_;
    InvenType invenType_;
    InventoryInfo& inventoryInfo_;
	BaseItemInfos tempHarvestItems_;
};

}} // namespace gideon { namespace zoneserver {