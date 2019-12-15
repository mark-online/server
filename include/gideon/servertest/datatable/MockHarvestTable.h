#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/HarvestTable.h>

namespace gideon { namespace servertest {

/**
* @class MockHarvestTable
*/
class GIDEON_SERVER_API MockHarvestTable : public gideon::datatable::HarvestTable
{
public:
	MockHarvestTable();
	virtual ~MockHarvestTable() {
		destroyHarvests();
	}
	virtual const gdt::harvest_t* getHarvest(HarvestCode code) const ;

private:
	void fillHarvest();
    void fillHarvest2();
	void destroyHarvests();

	virtual const HarvestMap& getHarvestMap() const {
		return harvestMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	HarvestMap harvestMap_;
};

}} // namespace gideon { namespace servertest {