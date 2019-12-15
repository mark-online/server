#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockQuestItemTable.h>
#include <gideon/servertest/datatable/DataCodes.h>


namespace gideon { namespace servertest {


MockQuestItemTable::MockQuestItemTable()
{
	fillQuestItem();
}

const datatable::QuestItemTemplate* MockQuestItemTable::getQuestItemTemplate(QuestItemCode code) const
{
	const QuestItemMap::const_iterator pos = questItemMap_.find(code);
	if (pos != questItemMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}


void MockQuestItemTable::fillQuestItem()
{
    gdt::quest_item_t* questItem = new gdt::quest_item_t;
    questItem->quest_item_code(transportQuestItemCode);
    //questItem->usable(0);
    //questItem->usable_count(0);
    //questItem->is_zero_count_destory(false);
    //questItem->can_cast_on_moving(0);

    //questItem->targeting_type(0);
    //questItem->cast_type(0);
    //questItem->casting_time(0);
    //questItem->available_target(0);
    //questItem->except_target(0);
    //questItem->effect_range_direction(0);
    //questItem->effect_range_value(0);
    //questItem->use_global_cooldown_time(0);
    //questItem->global_cooldown_time(0);


    //questItem->effect_script_1(estUnknown);
    //questItem->effect_spell_group_1(escNone);
    //questItem->activate_time_1(0);
    //questItem->effect_tick_time_1(0);
    //questItem->effect_percent_by_value_1(0);
    //questItem->effect_value_1(0);

    //questItem->effect_script_2(estUnknown);
    //questItem->effect_spell_group_2(escNone);
    //questItem->activate_time_2(0);
    //questItem->effect_tick_time_2(0);
    //questItem->effect_percent_by_value_2(0);
    //questItem->effect_value_2(0);
    
    
    //questItemMap_.insert(QuestItemMap::value_type(transportQuestItemCode, 
    //    datatable::QuestItemTemplate(*questItem)));
}



void MockQuestItemTable::destroyQuestItem()
{
	for (QuestItemMap::value_type& value : questItemMap_) {
		datatable::QuestItemTemplate& qt = value.second;
		qt.deleteQuestItemInfo();		
	}
}



}} //namespace gideon { namespace servertest {