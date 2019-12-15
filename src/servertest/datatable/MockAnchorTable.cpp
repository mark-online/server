#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockAnchorTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockAnchorTable::MockAnchorTable()
{
	fillAnchor();
}

const datatable::AnchorTemplate* MockAnchorTable::getAnchorTemplate(AnchorCode code) const
{
	const AnchorMap::const_iterator pos = anchorMap_.find(code);
	if (pos != anchorMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}


void MockAnchorTable::fillAnchor()
{
	gdt::anchor_t* anchor = new gdt::anchor_t;
	anchor->anchor_code(shabbyAncorCode);
    anchor->hp(hpDefault);

    //anchor->anchor_owner(1);
    //anchor->modeling_radius_size(10);
    //anchor->regist(0);
    //anchor->limit_active_sec(0);
    //anchor->complete_health_point(1000);

    //anchor->modeling_radius_size(1000);

    //anchor->check_building_code(0);
    //anchor->check_distance(0);

    //anchor->so_skill_code_1(selfAreaHpDamangeSkill);
    //anchor->is_auto_1(true);
    //anchor->auto_sec_1(60);

    //anchor->so_skill_code_2(selfDamangeSkill);
    //anchor->is_auto_2(true);
    //anchor->auto_sec_2(180);

    //anchor->so_skill_code_3(targetDamangeSkill);
    //anchor->is_auto_3(false);
    //anchor->auto_sec_3(0);

    //anchor->can_private_bank(false);
    //anchor->stack_count(255);

	anchorMap_.emplace(shabbyAncorCode, datatable::AnchorTemplate(*anchor));
}


void MockAnchorTable::destroyAnchors()
{
	for (AnchorMap::value_type& value : anchorMap_) {
		datatable::AnchorTemplate& vt = value.second;
		vt.deleteAnchorInfo();		
	}
}



}} //namespace gideon { namespace servertest {
