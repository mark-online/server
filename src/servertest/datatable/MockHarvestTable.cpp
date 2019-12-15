#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockHarvestTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockHarvestTable::MockHarvestTable()
{
	fillHarvest();
    fillHarvest2();
}

const gdt::harvest_t* MockHarvestTable::getHarvest(HarvestCode code) const 
{
	const HarvestMap::const_iterator pos = harvestMap_.find(code);
	if (pos != harvestMap_.end()) {
		return (*pos).second;
	}
	return nullptr;
}


void MockHarvestTable::fillHarvest()
{
	gdt::harvest_t* harvest = new gdt::harvest_t;
	harvest->harvest_code(harvestCode1);
	//harvest->harvest_delay(0);
	//harvest->min_harvest_count(1);
	//harvest->max_harvest_count(1);
	//
	//harvest->harvest_1(normalNpcElementCode1);
	//harvest->harvest_pct_1(1000);
	//harvest->min_drop_count_1(1);
	//harvest->max_drop_count_1(2);

	//harvest->harvest_2(normalNpcElementCode2);
	//harvest->harvest_pct_2(300);
	//harvest->min_drop_count_2(1);
	//harvest->max_drop_count_2(3);


	//harvest->harvest_3(servantNpcElementCode1);
	//harvest->harvest_pct_3(100);
	//harvest->min_drop_count_3(1);
	//harvest->max_drop_count_3(1);

	harvestMap_.emplace(harvestCode1, harvest);
}


void MockHarvestTable::fillHarvest2()
{
    gdt::harvest_t* harvest = new gdt::harvest_t;
    harvest->harvest_code(harvestCode2);
    //harvest->harvest_delay(30000);
    //harvest->min_harvest_count(1);
    //harvest->max_harvest_count(1);

    //harvest->harvest_1(normalNpcElementCode1);
    //harvest->harvest_pct_1(1000);
    //harvest->min_drop_count_1(1);
    //harvest->max_drop_count_1(2);

    //harvest->harvest_2(normalNpcElementCode2);
    //harvest->harvest_pct_2(300);
    //harvest->min_drop_count_2(1);
    //harvest->max_drop_count_2(3);


    //harvest->harvest_3(servantNpcElementCode1);
    //harvest->harvest_pct_3(100);
    //harvest->min_drop_count_3(1);
    //harvest->max_drop_count_3(1);

    harvestMap_.emplace(harvestCode2, harvest);
}


void MockHarvestTable::destroyHarvests()
{
	//for (HarvestMap::value_type& value : harvestMap_) {
	//	datatable::HarvestTemplate& vt = value.second;
	//	vt.deleteHarvestInfo();		
	//}
}

}} //namespace gideon { namespace servertest {
