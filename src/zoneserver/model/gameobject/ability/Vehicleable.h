#pragma once


namespace gideon { namespace zoneserver { namespace go {

/**
* @class Vehicleable
 * .
 */
class Vehicleable
{
public:
    virtual ~Vehicleable() {}
	
public:
	virtual VehicleCode getVehicleCode() const = 0;
	virtual EntityVehicleInfo getEntityVehicle() const = 0;

	virtual void addVehicle(const VehicleInfo& info) = 0;
	virtual ErrorCode selectVehicle(ObjectId selectId) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
