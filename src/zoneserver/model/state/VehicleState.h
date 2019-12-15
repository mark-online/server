#pragma once

#include <gideon/cs/shared/data/VehicleInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class VehicleState
 **/
class VehicleState
{
public:
    virtual bool canMountVehicle() const = 0;
    virtual bool canDismountVehicle() const = 0;

    virtual void mountVehicle(VehicleCode code, HarnessCode harnessCode) = 0;
    virtual void dismountVehicle() = 0;

	virtual bool isVehicleMounting() const = 0;
};


}} // namespace gideon { namespace zoneserver {
