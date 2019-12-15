#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ItemInfo.h>
#include <gideon/cs/shared/data/QuestInfo.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class HarvestCallback
 */
class HarvestCallback
{
public:
    virtual ~HarvestCallback() {}

public:
    virtual void rewarded(ObjectId harvestId,const BaseItemInfos& baseItems,
        const QuestItemInfos& questItems) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
