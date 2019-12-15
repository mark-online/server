#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/servertest/datatable/MockStaticObjectSkillTable.h>

namespace gideon { namespace servertest {

//const gideon::datatable::StaticObjectSkillTemplate* MockStaticObjectSkillTable::getSkill(SkillCode code) const
//{
//    const StaticObjectSkillMap::const_iterator pos = skillMap_.find(code);
//    if (pos != skillMap_.end()) {
//        return &(*pos).second;
//    }
//    return nullptr;
//}
//
//
//void MockStaticObjectSkillTable::fillSelfAreaEntityHpDamageSkill()
//{
//    gdt::so_skill_t* skill = new gdt::so_skill_t;
//    skill->so_skill_code(selfAreaHpDamangeSkill);
//    skill->skill_type(sktAttackMagic);
//    skill->targeting_type(ttSelfArea);
//    skill->cast_type(catDirect);
//    skill->available_target(atAll);
//    skill->except_target(setSelfExcept);
//    skill->min_distance(0);
//    skill->max_distance(100);
//    skill->target_direction(tdNoCheck);
//    skill->effect_range_direction(erdEverySide);
//    skill->effect_range_value(10);
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->use_global_cooldown_time(0);
//    skill->global_cooldown_time(0);
//    skill->consumed_hp(0);     
//
//    skill->mez_check(0);   
//    skill->check_stat_type(0);
//    skill->check_stat_up_down(0);
//    skill->check_stat_value_by_percent(0);
//    skill->check_stat_value(0);
//
//    skill->caster_spell_group_1(0);
//    skill->caster_effect_script_1(estUnknown);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_active_time_1(0);
//    skill->caster_effect_tick_time_1(0);
//
//    skill->caster_spell_group_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_active_time_2(0);
//    skill->caster_effect_tick_time_2(0);
//
//    skill->castee_spell_group_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//
//    skill->castee_spell_group_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//
//    skill->projectile_speed(0);
//
//    skillMap_.insert(
//        StaticObjectSkillMap::value_type(skill->so_skill_code(),
//            gideon::datatable::StaticObjectSkillTemplate(*skill)));
//}
//
//
//void MockStaticObjectSkillTable::fillSelfHpDamageSkill()
//{
//    gdt::so_skill_t* skill = new gdt::so_skill_t;
//    skill->so_skill_code(selfDamangeSkill);
//    skill->skill_type(sktAttackMagic);
//    skill->targeting_type(ttSelf);
//    skill->cast_type(catDirect);
//    skill->available_target(atAll);
//    skill->except_target(setNoExcept);
//    skill->min_distance(0);
//    skill->max_distance(100);
//    skill->target_direction(tdNoCheck);
//    skill->effect_range_direction(erdInvalid);
//    skill->effect_range_value(0);
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->use_global_cooldown_time(0);
//    skill->global_cooldown_time(0);
//    skill->consumed_hp(0);      
//
//    skill->mez_check(0);   
//    skill->check_stat_type(0);
//    skill->check_stat_up_down(0);
//    skill->check_stat_value_by_percent(0);
//    skill->check_stat_value(0);
//    
//    skill->caster_spell_group_1(0);
//    skill->caster_effect_script_1(estHpDamage);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(10);
//    skill->caster_active_time_1(0);
//    skill->caster_effect_tick_time_1(0);
//
//    skill->caster_spell_group_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_active_time_2(0);
//    skill->caster_effect_tick_time_2(0);
//
//    skill->castee_spell_group_1(0);
//    skill->castee_effect_script_1(estUnknown);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(0);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//
//    skill->castee_spell_group_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//
//
//    skill->projectile_speed(0);
//
//    skillMap_.insert(
//        StaticObjectSkillMap::value_type(skill->so_skill_code(),
//            gideon::datatable::StaticObjectSkillTemplate(*skill)));
//}
//
//
//void MockStaticObjectSkillTable::fillTargetHpDamageSkill()
//{
//    gdt::so_skill_t* skill = new gdt::so_skill_t;
//    skill->so_skill_code(targetDamangeSkill);
//    skill->skill_type(sktAttackMagic);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->available_target(atAll);
//    skill->except_target(setNoExcept);
//    skill->min_distance(0);
//    skill->max_distance(100);
//    skill->target_direction(tdNoCheck);
//    skill->effect_range_direction(erdInvalid);
//    skill->effect_range_value(0);
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->use_global_cooldown_time(0);
//    skill->global_cooldown_time(0);
//    skill->consumed_hp(100);     
// 
//    skill->mez_check(0);   
//    skill->check_stat_type(0);
//    skill->check_stat_up_down(0);
//    skill->check_stat_value_by_percent(0);
//    skill->check_stat_value(0);
//
//    skill->caster_spell_group_1(0);
//    skill->caster_effect_script_1(estUnknown);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_active_time_1(0);
//    skill->caster_effect_tick_time_1(0);
//
//    skill->caster_spell_group_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_active_time_2(0);
//    skill->caster_effect_tick_time_2(0);
//
//    skill->castee_spell_group_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//
//    skill->castee_spell_group_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//
//    skill->projectile_speed(0);
//
//    skillMap_.insert(
//        StaticObjectSkillMap::value_type(skill->so_skill_code(),
//            gideon::datatable::StaticObjectSkillTemplate(*skill)));
//}
//
//
//void MockStaticObjectSkillTable::destroySkills()
//{
//    for (StaticObjectSkillMap::value_type& value : skillMap_) {
//        datatable::StaticObjectSkillTemplate& st = value.second;
//        delete st.getSkillInfo();
//    }
//}

}} // namespace gideon { namespace servertest {
