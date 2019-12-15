#pragma once

#include <gideon/cs/shared/data/QuestInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {

/***
 * @class QuestInventory
 ***/
class QuestInventory
{
public:
    QuestInventory(go::Entity& owner, QuestInventoryInfo& itemInventoryInfo);
    ~QuestInventory();

    void initialize();

    bool addItem(const QuestItemInfo& questItem);

    void removeItemsByQuestCode(QuestCode questCode);
    void removeItemsByItemCode(QuestItemCode itemCode, uint8_t removeCount);
    void removeItem(QuestItemCode itemCode, QuestCode questCode);
    ErrorCode removeItem(ObjectId itemId);

    bool checkHasQuestItem(QuestItemCode itemCode, uint8_t itemCount) const;
    bool checkHasQuestItems(BaseItemInfos& baseItemInfos) const;
    
    const QuestItemInfo* getItemInfoById(ObjectId itemId) const;
	const QuestItemInfo* getItemInfoByCode(QuestItemCode questItemCode) const;

    bool canAddQuestItem(uint8_t itemCount);

    bool hasUseableCount(ObjectId itemId) const;
    void useItemCount(ObjectId itemId);

    void setTempHarvestItems(const QuestItemInfos& questItemInfos);
    ErrorCode acquireHarvest(DataCode itemCode);
private:
    bool addItem_i(const QuestItemInfo questItem);
    ErrorCode removeItem_i(ObjectId itemId);
    bool insertUseableItem(ObjectId itemId, QuestItemCode itemCode);

private:
    go::Entity& owner_;
    QuestItemInfos tempHarvestItems_;
    QuestInventoryInfo& itemInventoryInfo_;
};

}} // namespace gideon { namespace zoneserver {