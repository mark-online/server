#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockVehicleTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockVehicleTable::MockVehicleTable()
{
	fillVehicle();
}


const gdt::vehicle_t* MockVehicleTable::getVehicle(VehicleCode code) const
{
	const VehicleMap::const_iterator pos = vehicleMap_.find(code);
	if (pos != vehicleMap_.end()) {
		return (*pos).second;
	}
	return nullptr;
}


void MockVehicleTable::fillVehicle()
{
	gdt::vehicle_t* vehicle = new gdt::vehicle_t;
	vehicle->vehicle_code(vehicleCode1);
	vehicle->cast_time(0);
	vehicleMap_.emplace(vehicleCode1, vehicle);
}


void MockVehicleTable::destroyVehicles()
{
	for (VehicleMap::value_type& value : vehicleMap_) {
		delete value.second;
	}
}



}} //namespace gideon { namespace servertest {