#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ElementInfo.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class ItemCallback
 */
class ItemCallback
{
public:
    virtual ~ItemCallback() {}

public:
    virtual void functionItemUsed(ObjectId itemId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
