#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockBuildingTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockBuildingTable::MockBuildingTable()
{
	fillBuilding();
}

const datatable::BuildingTemplate* MockBuildingTable::getBuildingTemplate(BuildingCode code) const
{
	const BuildingMap::const_iterator pos = buildingMap_.find(code);
	if (pos != buildingMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}


void MockBuildingTable::fillBuilding()
{
	gdt::building_t* building = new gdt::building_t;

    building->building_code(castleWallCode);
    building->anchor_owner(botPrivate);
    building->modeling_radius_size(10);
    building->complete_regist(10);
    building->limit_active_sec(10);
    building->complete_health_point(100);

    building->check_building_code_1(0);
    building->check_distance_1(0);
    building->check_building_code_2(0);
    building->check_distance_2(0);

    building->foundation_health_point(100);
    building->foundation_regist(10);
    building->foundation_live_sec(100);

    building->building_materials_1(normalNpcElementCode1);
    building->building_materials_2(0);
    building->building_materials_3(0);
    building->building_materials_4(0);
    building->building_materials_5(0);
    building->building_materials_count_1(1);
    building->building_materials_count_2(0);
    building->building_materials_count_3(0);
    building->building_materials_count_4(0);
    building->building_materials_count_5(0);

    building->consume_item_code_1(0);
    building->consume_item_code_2(0);
    building->consume_item_code_3(0);

    building->consume_item_count_1(0);
    building->consume_item_count_2(0);
    building->consume_item_count_3(0);

    building->consume_sec_1(0);
    building->consume_sec_2(0);
    building->consume_sec_3(0);

    building->so_skill_code_1(selfAreaHpDamangeSkillCode);
    building->so_skill_cast_type_1(1);
    building->auto_sec_1(60);

    building->so_skill_code_2(selfDamangeSkillCode);
    building->so_skill_cast_type_2(1);
    building->auto_sec_2(180);

    building->so_skill_code_3(targetDamangeSkillCode);
    building->so_skill_cast_type_3(::gdt::so_skill_cast_type_t(0));
    building->auto_sec_3(0);
    building->stack_count(255);

    building->function_script_1(bfsUnknown);
    building->function_script_2(bfsUnknown);
    building->function_script_3(bfsUnknown);

    buildingMap_.emplace(castleWallCode, datatable::BuildingTemplate(*building));
}


void MockBuildingTable::destroyBuildings()
{
	for (BuildingMap::value_type& value : buildingMap_) {
		datatable::BuildingTemplate& vt = value.second;
		vt.deleteBuildingInfo();		
	}
}

}} //namespace gideon { namespace servertest {
