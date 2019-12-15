#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockElementTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const gdt::element_t* MockElementTable::getElement(ElementCode code) const
{
    const ElementMap::const_iterator pos = elementMap_.find(code);
    if (pos != elementMap_.end()) {
        return (*pos).second;
    }
    return nullptr; 
}


void MockElementTable::fillElement1()
{
    //gdt::element_t* element = new gdt::element_t;
    //element->element_code(normalNpcElementCode1);
    //element->looting_pct(1000);
    //element->looting_delay(5000);
    //element->need_element_count(5);

    
    //element->usable(1);
    //element->targeting_type(ttSelf);
    //element->cool_time(1000);
    //element->cast_type(catDirect);
    //element->can_cast_on_moving(1);
    //element->use_casting_time(0);
    //element->available_target(atSelf);
    //element->except_target(0);
    //element->effect_range_direction(0); 
    //element->effect_range_value(0);
    //element->use_global_cooldown_time(1);
    //element->global_cooldown_time(2000);
    //element->usable_min_distance(0);
    //element->usable_max_distance(10);


    //element->effect_script_1(estHpRecovery);
    //element->effect_spell_group_1(escNone);
    //element->activate_time_1(0);
    //element->effect_tick_time_1(0);
    //element->effect_percent_by_value_1(0);
    //element->effect_value_1(100);

    //element->effect_script_2(estHpRecovery);
    //element->effect_spell_group_2(escNone);
    //element->activate_time_2(0);
    //element->effect_tick_time_2(0);
    //element->effect_percent_by_value_2(0);
    //element->effect_value_2(100);
    //element->stack_count(255);


    //element->product_component_code(refinedShellComponent);
    //element->product_component_count(1);

    //elementMap_.insert(ElementMap::value_type(normalNpcElementCode1,
    //    datatable::ElementItemTemplate(element)));
}


void MockElementTable::fillElement2()
{
    //gdt::element_t* element = new gdt::element_t;
    //element->element_code(normalNpcElementCode2);
    //element->looting_pct(1000);
    //element->looting_delay(5000);
    //element->need_element_count(5);
    //element->use_casting_time(0);

    //element->usable(1);

    //element->targeting_type(ttSelf);
    //element->cast_type(catDirect);
    //element->can_cast_on_moving(1);
    //element->use_casting_time(0);
    //element->available_target(atSelf);
    //element->except_target(0);
    //element->effect_range_direction(0);
    //element->effect_range_value(0);
    //element->use_global_cooldown_time(0);
    //element->global_cooldown_time(0);
    //element->usable_min_distance(0);
    //element->usable_max_distance(10);


    //element->effect_script_1(estHpRecovery);
    //element->effect_spell_group_1(escNone);
    //element->activate_time_1(0);
    //element->effect_tick_time_1(0);
    //element->effect_percent_by_value_1(0);
    //element->effect_value_1(100);

    //element->effect_script_2(estHpRecovery);
    //element->effect_spell_group_2(escNone);
    //element->activate_time_2(0);
    //element->effect_tick_time_2(0);
    //element->effect_percent_by_value_2(0);
    //element->effect_value_2(100);
    //element->stack_count(255);

    //element->product_component_code(refinedVenomComponent);
    //element->product_component_count(1);
    //elementMap_.insert(ElementMap::value_type(normalNpcElementCode2,
    //    datatable::ElementItemTemplate(element)));
}


void MockElementTable::fillElement3()
{
    //gdt::element_t* element = new gdt::element_t;
    //element->element_code(servantNpcElementCode1);
    //element->looting_pct(100);
    //element->looting_delay(5000);
    //element->need_element_count(5);
    //element->usable(1);

    //element->targeting_type(ttSelf);
    //element->cast_type(catDirect);
    //element->can_cast_on_moving(1);
    //element->use_casting_time(0);
    //element->available_target(atSelf);
    //element->except_target(0);
    //element->effect_range_direction(0);
    //element->effect_range_value(0);
    //element->use_global_cooldown_time(0);
    //element->global_cooldown_time(0);
    //element->usable_min_distance(0);
    //element->usable_max_distance(10);


    //element->effect_script_1(estHpRecovery);
    //element->effect_spell_group_1(escNone);
    //element->activate_time_1(0);
    //element->effect_tick_time_1(0);
    //element->effect_percent_by_value_1(0);
    //element->effect_value_1(100);

    //element->effect_script_2(estHpRecovery);
    //element->effect_spell_group_2(escNone);
    //element->activate_time_2(0);
    //element->effect_tick_time_2(0);
    //element->effect_percent_by_value_2(0);
    //element->effect_value_2(100);

    //element->stack_count(255);

    //element->product_component_code(refinedLeatherComponent);
    //element->product_component_count(1);
    //elementMap_.insert(ElementMap::value_type(servantNpcElementCode1,
    //    datatable::ElementItemTemplate(element)));
}


void MockElementTable::fillElement4()
{
    //gdt::element_t* element = new gdt::element_t;
    //element->element_code(servantNpcElementCode2);
    //element->looting_pct(200);
    //element->looting_delay(5000);
    //element->need_element_count(5);
    //element->usable(1);
    //
    //element->targeting_type(ttSelf);
    //element->cast_type(catDirect);
    //element->can_cast_on_moving(1);
    //element->use_casting_time(0);
    //element->available_target(atSelf);
    //element->except_target(0);
    //element->effect_range_direction(0);
    //element->effect_range_value(0);
    //element->use_global_cooldown_time(0);
    //element->global_cooldown_time(0);
    //element->usable_min_distance(0);
    //element->usable_max_distance(10);


    //element->effect_script_1(estHpRecovery);
    //element->effect_spell_group_1(escNone);
    //element->activate_time_1(0);
    //element->effect_tick_time_1(0);
    //element->effect_percent_by_value_1(0);
    //element->effect_value_1(100);

    //element->effect_script_2(estHpRecovery);
    //element->effect_spell_group_2(escNone);
    //element->activate_time_2(0);
    //element->effect_tick_time_2(0);
    //element->effect_percent_by_value_2(0);
    //element->effect_value_2(100);

    //element->stack_count(255);

    //element->product_component_code(refinedQuintessenceComponent);
    //element->product_component_count(1);
    //elementMap_.insert(ElementMap::value_type(servantNpcElementCode2, 
    //    datatable::ElementItemTemplate(element)));
}


void MockElementTable::fillElement5()
{
    //gdt::element_t* element = new gdt::element_t;
    //element->element_code(bossNpcElementCode);
    //element->looting_pct(100);
    //element->looting_delay(5000);
    //element->need_element_count(5);   
    //element->usable(1);

    //element->targeting_type(ttSelf);
    //element->can_cast_on_moving(1);
    //element->cast_type(catDirect);
    //element->use_casting_time(0);
    //element->available_target(0);
    //element->except_target(0);
    //element->effect_range_direction(0);
    //element->effect_range_value(0);
    //element->use_global_cooldown_time(0);
    //element->global_cooldown_time(0);
    //element->usable_min_distance(0);
    //element->usable_max_distance(10);

    //element->stack_count(255);
    //element->effect_script_1(estHpRecovery);
    //element->effect_spell_group_1(escNone);
    //element->activate_time_1(0);
    //element->effect_tick_time_1(0);
    //element->effect_percent_by_value_1(0);
    //element->effect_value_1(100);

    //element->effect_script_2(estHpRecovery);
    //element->effect_spell_group_2(escNone);
    //element->activate_time_2(0);
    //element->effect_tick_time_2(0);
    //element->effect_percent_by_value_2(0);
    //element->effect_value_2(100);



    //element->product_component_code(refinedHornComponent);
    //element->product_component_count(1);
    //elementMap_.insert(ElementMap::value_type(bossNpcElementCode, 
    //    datatable::ElementItemTemplate(element)));
}


void MockElementTable::fillElement6()
{
    //gdt::element_t* element = new gdt::element_t;
    //element->element_code(extendInventoryElementCode);
    //element->looting_pct(0);
    //element->looting_delay(0);
    //element->need_element_count(0);   
    //element->usable(1);

    //element->targeting_type(ttSelf);
    //element->can_cast_on_moving(1);
    //element->cast_type(catDirect);
    //element->use_casting_time(0);
    //element->available_target(0);
    //element->except_target(0);
    //element->effect_range_direction(0);
    //element->effect_range_value(0);
    //element->use_global_cooldown_time(0);
    //element->global_cooldown_time(0);
    //element->usable_min_distance(0);
    //element->usable_max_distance(10);

    //element->stack_count(255);
    //element->effect_script_1(estInvenExtendFreeSlot);
    //element->effect_spell_group_1(escNone);
    //element->activate_time_1(0);
    //element->effect_tick_time_1(0);
    //element->effect_percent_by_value_1(0);
    //element->effect_value_1(5);

    //element->effect_script_2(estUnknown);
    //element->effect_spell_group_2(escNone);
    //element->activate_time_2(0);
    //element->effect_tick_time_2(0);
    //element->effect_percent_by_value_2(0);
    //element->effect_value_2(0);


    //element->product_component_code(0);
    //element->product_component_count(1);
    //elementMap_.insert(ElementMap::value_type(extendInventoryElementCode, 
    //    datatable::ElementItemTemplate(element)));
}



void MockElementTable::destroyElements()
{

}

}} // namespace gideon { namespace servertest {
