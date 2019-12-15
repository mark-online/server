#pragma once

#include <gideon/cs/shared/data/ItemInfo.h>


namespace gideon { namespace zoneserver { namespace go {

/**
 * @class ConsumeItemable
 */
class ConsumeItemable
{
public:
    virtual ~ConsumeItemable() {}

public:
    virtual void consumeItem(const BaseItemInfo& baseItemInfo) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
