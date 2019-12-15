#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/BuildingTable.h>

namespace gideon { namespace servertest {

/**
* @class MockBuildingTable
*/
class GIDEON_SERVER_API MockBuildingTable : public gideon::datatable::BuildingTable
{
public:
	MockBuildingTable();
	virtual ~MockBuildingTable() {
		destroyBuildings();
	}
	virtual const gideon::datatable::BuildingTemplate* getBuildingTemplate(BuildingCode code) const;

private:
	void fillBuilding();
	void destroyBuildings();

	virtual const BuildingMap& getBuildingMap() const {
		return buildingMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	BuildingMap buildingMap_;
};

}} // namespace gideon { namespace servertest {