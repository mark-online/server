#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/VehicleTable.h>

namespace gideon { namespace servertest {

/**
* @class MockVehicleTable
*/
class GIDEON_SERVER_API MockVehicleTable : public gideon::datatable::VehicleTable
{
public:
	MockVehicleTable();
	virtual ~MockVehicleTable() {
		destroyVehicles();
	}
	virtual const gdt::vehicle_t* getVehicle(VehicleCode code) const;

private:
	void destroyVehicles();
	void fillVehicle();

private:

	virtual const VehicleMap& getVehicleMap() const {
		return vehicleMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	VehicleMap vehicleMap_;
};

}} // namespace gideon { namespace servertest {