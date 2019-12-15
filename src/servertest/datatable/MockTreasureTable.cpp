#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockTreasureTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockTreasureTable::MockTreasureTable()
{
	fillTreasure1();
    fillTreasure2();
}

const gdt::treasure_t* MockTreasureTable::getTreasure(TreasureCode code) const
{
	const TreasureMap::const_iterator pos = treasureMap_.find(code);
	if (pos != treasureMap_.end()) {
		return (*pos).second;
	}
	return nullptr;
}


void MockTreasureTable::fillTreasure1()
{
	gdt::treasure_t* treasure = new gdt::treasure_t;
	treasure->treasure_code(treasureCode1);
	//treasure->open_delay(0);
	//
	//treasure->reward_type_1(trtEpicRecipeRandom);
	//treasure->reward_info_1(0);
	//treasure->reward_pct_1(10);
	//treasure->min_reward_value_1(1);
	//treasure->max_reward_value_1(1);

	//treasure->reward_type_2(trtNormalRecipeRandom);
	//treasure->reward_info_2(0);
	//treasure->reward_pct_2(20);
	//treasure->min_reward_value_2(1);
	//treasure->max_reward_value_2(1);
	//
	//treasure->reward_type_3(trtShabbyRecipeRandom);
	//treasure->reward_info_3(0);
	//treasure->reward_pct_3(1000);
	//treasure->min_reward_value_3(1);
	//treasure->max_reward_value_3(1);

	//treasure->add_reward_type_1(trtMoney);
	//treasure->add_reward_info_1(0);
	//treasure->add_reward_pct_1(1000);
	//treasure->min_add_reward_value_1(100);
	//treasure->max_add_reward_value_1(2000);


	//treasure->add_reward_type_2(trtExp);
	//treasure->add_reward_info_2(0);
	//treasure->add_reward_pct_2(1000);
	//treasure->min_add_reward_value_2(50);
	//treasure->max_add_reward_value_2(500);

	treasureMap_.emplace(treasureCode1, treasure);	
}


void MockTreasureTable::fillTreasure2()
{
    gdt::treasure_t* treasure = new gdt::treasure_t;
    treasure->treasure_code(treasureCode2);
    /*treasure->open_delay(30000);

    treasure->reward_type_1(trtEpicRecipeRandom);
    treasure->reward_info_1(0);
    treasure->reward_pct_1(10);
    treasure->min_reward_value_1(1);
    treasure->max_reward_value_1(1);

    treasure->reward_type_2(trtNormalRecipeRandom);
    treasure->reward_info_2(0);
    treasure->reward_pct_2(20);
    treasure->min_reward_value_2(1);
    treasure->max_reward_value_2(1);

    treasure->reward_type_3(trtShabbyRecipeRandom);
    treasure->reward_info_3(0);
    treasure->reward_pct_3(1000);
    treasure->min_reward_value_3(1);
    treasure->max_reward_value_3(1);

    treasure->add_reward_type_1(trtMoney);
    treasure->add_reward_info_1(0);
    treasure->add_reward_pct_1(1000);
    treasure->min_add_reward_value_1(100);
    treasure->max_add_reward_value_1(2000);


    treasure->add_reward_type_2(trtExp);
    treasure->add_reward_info_2(0);
    treasure->add_reward_pct_2(1000);
    treasure->min_add_reward_value_2(50);
    treasure->max_add_reward_value_2(500);*/

    treasureMap_.emplace(treasureCode2, treasure);	
}


void MockTreasureTable::destroyTreasures()
{
	//for (TreasureMap::value_type& value : treasureMap_) {
	//	datatable::TreasureTemplate& vt = value.second;
	//	vt.deleteTreasureInfo();		
	//}
}



}} //namespace gideon { namespace servertest {