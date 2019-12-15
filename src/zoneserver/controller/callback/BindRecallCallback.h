#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class BindRecallCallback
 */
class BindRecallCallback
{
public:
    virtual ~BindRecallCallback() {}

public:
    virtual void completed(ObjectId linkId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
