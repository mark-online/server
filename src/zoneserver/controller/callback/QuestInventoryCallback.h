#pragma once

#include <gideon/cs/shared/data/QuestInfo.h>

namespace gideon { namespace zoneserver { namespace gc {


/***
 * @class QuestInventoryCallback
 ***/
class QuestInventoryCallback
{
public:
	virtual ~QuestInventoryCallback() {}

	virtual void inventoryQuestItemAdded(const QuestItemInfo& itemInfo) = 0;
    virtual void inventoryQuestItemRemoved(ObjectId itemId) = 0;
	virtual void inventoryQuestItemUsableCountUpdated(ObjectId itemId, uint8_t usableCount) = 0;
	virtual void inventoryQuestItemCountUpdated(ObjectId itemId, uint8_t usableCount) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace gc {