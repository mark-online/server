#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class VehicleCallback
 */
class VehicleCallback
{
public:
    virtual ~VehicleCallback() {}

public:
	virtual void mountVehicle() = 0;
    virtual void vehicleDismounted(const GameObjectInfo& creatureInfo, float32_t speed, bool isForce) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
