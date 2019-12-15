#pragma once

#include <gideon/cs/shared/data/LootInvenItemInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class TreasureCallback
 */
class TreasureCallback
{
public:
    virtual ~TreasureCallback() {}

public:
    virtual void opened(ObjectId treasureId, const LootInvenItemInfoMap& infoMap) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
