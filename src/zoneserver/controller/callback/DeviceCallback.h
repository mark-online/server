#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class DeviceCallback
 */
class DeviceCallback
{
public:
    virtual ~DeviceCallback() {}

public:
    virtual void deviceDeactivated(ObjectId deviceId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
