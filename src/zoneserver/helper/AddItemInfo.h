#pragma once

#include <gideon/cs/shared/data/ItemInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class AddItemInfo 
 * 추가될 아이템 정보
 */
struct AddItemInfo : public BaseItemInfo
{
    ObjectId itemId_;
    sec_t expireTime_; // 파괴될 시간

    EquipItemInfo equipItemInfo_;
    AccessoryItemInfo accessoryItemInfo_;

    explicit AddItemInfo(DataCode itemCode, uint8_t count) :
        BaseItemInfo(itemCode, count),
        itemId_(invalidObjectId),
        expireTime_(0)
    {}

    explicit AddItemInfo(const BaseItemInfo& baseInfo, const EquipItemInfo& equipItemInfo) :
        BaseItemInfo(baseInfo.itemCode_, baseInfo.count_),
        expireTime_(0),
        itemId_(invalidObjectId),
        equipItemInfo_(equipItemInfo) 
    {
        assert(isEquipment());
    }

    explicit AddItemInfo(const BaseItemInfo& baseInfo, const AccessoryItemInfo& accessoryItemInfo) :
        BaseItemInfo(baseInfo.itemCode_, baseInfo.count_),
        expireTime_(0),
        itemId_(invalidObjectId),
        accessoryItemInfo_(accessoryItemInfo) 
    {
        assert(isAccessory());
    }

    explicit AddItemInfo(const ItemInfo& itemInfo) :
        BaseItemInfo(itemInfo.itemCode_, itemInfo.count_),
        itemId_(itemInfo.itemId_),
        equipItemInfo_(itemInfo.equipItemInfo_),
        accessoryItemInfo_(itemInfo.accessoryItemInfo_),
        expireTime_(0)

    {
    }

    explicit AddItemInfo(const BuyBackItemInfo& itemInfo) :
        BaseItemInfo(itemInfo.itemCode_, itemInfo.count_),
        itemId_(invalidObjectId),
        equipItemInfo_(itemInfo.equipItemInfo_),
        accessoryItemInfo_(itemInfo.accessoryItemInfo_),
        expireTime_(0)

    {
    }
};


}} // namespace gideon { namespace zoneserver {