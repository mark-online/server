#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockSkillTable.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/SkillTemplate.h>

namespace gideon { namespace servertest {

namespace {

//void fillClientInfo(gdt::skill_t* skill)
//{
//    skill->casting_animation_name(L"");
//    skill->casting_draw_animation_name(L"");
//    skill->caster_animation_name(L"");
//    skill->castee_animation_name(L"");
//    skill->casting_effect(L"");
//    skill->attacker1_caster_effect(L"");
//    skill->attacker2_caster_effect(L"");
//    skill->attacked1_castee_effect(L"");
//    skill->attacked2_castee_effect(L"");
//    skill->area_effect(L"");
//	skill->caster_effect(L"");
//	skill->trail_effect(L"");
//}

} // namespace
//
//const gideon::datatable::PlayerSkillTemplate* MockSkillTable::getSkill(SkillCode code) const
//{
//    const SkillMap::const_iterator pos = skillMap_.find(code);
//    if (pos != skillMap_.end()) {
//        return &(*pos).second;
//    }
//    return nullptr;
//}
//
//
//void MockSkillTable::fillObserverSkillCode()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(testObserverSkillCode);
//    skill->spell_package_index(101);
//    skill->skill_level(1);
//    skill->skill_type(sktGeneral);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone),
//	skill->equip_part_allowed_equip(scetAny);
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(0);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(100);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillStunSkill()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(stunSkillCode);
//    skill->spell_package_index(40);
//    skill->skill_level(1);
//    skill->skill_type(sktGeneral);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone),
//    skill->equip_part_allowed_equip(scetAny);
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(10);
//
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estStun);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(0);
//    skill->castee_active_time_1(3000);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillGraveStoneSkill()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(graveStoneSkillCode);
//    skill->spell_package_index(27);
//    skill->skill_level(1);
//    skill->skill_type(sktGeneral);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone),
//        skill->equip_part_allowed_equip(scetAny);
//    skill->cast_time(0);
//    skill->cool_time(3000);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(10);
//
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estGraveStone);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(0);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillMpRecoverySkill()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(mpRecoverySkillCode);
//    skill->spell_package_index(27);
//    skill->skill_level(1);
//    skill->skill_type(sktBuffMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone),
//    skill->equip_part_allowed_equip(scetAny);
//    skill->cast_time(0);
//    skill->cool_time(2000);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(10);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estMpRecovery);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(4);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//
//    skill->targeting_type(ttSelf);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//
//void MockSkillTable::fillHpRecoverySkill()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(hpRecoverySkillCode);
//    skill->spell_package_index(27);
//    skill->skill_level(1);
//    skill->skill_type(sktBuffMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//
//    skill->cast_time(0);
//    skill->cool_time(2000);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpRecovery);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(4);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);   
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillMeleeAttackSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultMeleeAttackSkillCode);
//    skill->spell_package_index(3);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//
//    skill->cast_time(0);
//    skill->cool_time(2000);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(1);
//    skill->consumed_cp(20);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(1);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillRangeAttackSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultRangeAttackSkillCode);
//    skill->spell_package_index(8);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackRange);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    //  TODO : test
//    //skill->usable_state(susCombat);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//
//    skill->cast_time(0);
//    skill->cool_time(2000);
//    skill->min_distance(300);
//    skill->max_distance(500);
//    skill->consumed_hp(20);
//    skill->consumed_mp(10);
//    skill->consumed_cp(20);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(1);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);;
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catProjectile);
//    skill->projectile_speed(25);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillMagicAttackSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultMagicAttackSkillCode);
//    skill->spell_package_index(2);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susCombat);
//    //skill->transition_state(stsNone),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(3000);
//    skill->min_distance(300);
//    skill->max_distance(500); // 2000이면 관심영역을 벗어나 버림
//    skill->consumed_hp(0);
//    skill->consumed_mp(10);
//    skill->consumed_cp(2);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(1);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillAreaSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultAreaAndCastee2SkillCode);
//    skill->spell_package_index(2);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    skill->except_target(setSelfExcept);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//
//    skill->cast_time(0);
//    skill->cool_time(3000);
//    skill->min_distance(0);
//    skill->max_distance(500); // 20이면 관심영역을 벗어나 버림
//    skill->consumed_hp(0);
//    skill->consumed_mp(10);
//    skill->consumed_cp(2);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(1);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estHpDamage);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(10);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->castee_effect_tick_time_2(0);
//    skill->use_global_cooldown_time(0);
//    
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttArea);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//    skill->effect_range_direction(erdEverySide);
//    skill->effect_range_value(300);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillSelfAreaSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultSelfAreaSkillCode);
//    skill->spell_package_index(2);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    skill->except_target(setSelfExcept);
//
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//
//    skill->cast_time(0);
//    skill->cool_time(3000);
//    skill->min_distance(0);
//    skill->max_distance(500); // 20이면 관심영역을 벗어나 버림
//    skill->consumed_hp(0);
//    skill->consumed_mp(10);
//    skill->consumed_cp(2);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//
//    skill->should_castee_critical_1(1);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttSelfArea);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//    skill->effect_range_direction(erdEverySide);
//    skill->effect_range_value(300);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillTargetAreaSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultTargetAreaSkillCode);
//    skill->spell_package_index(2);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//    skill->except_target(setSelfExcept);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//
//    skill->cast_time(0);
//    skill->cool_time(3000);
//    skill->min_distance(0);
//    skill->max_distance(500); // 20이면 관심영역을 벗어나 버림
//    skill->consumed_hp(0);
//    skill->consumed_mp(10);
//    skill->consumed_cp(2);
//    skill->should_castee_critical_1(0);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttTargetArea);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//    skill->effect_range_direction(erdEverySide);
//    skill->effect_range_value(300);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillLongGlobalCooldownTimeSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultLongGlobalCooldownTimeSkillCode);
//    skill->spell_package_index(2);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//
//    // TODO: test
//    //skill->usable_state(susCombat);
//    //skill->transition_state(stsPeace),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//
//    skill->cast_time(0);
//    skill->cool_time(3000);
//    skill->min_distance(0);
//    skill->max_distance(500); // 2000이면 관심영역을 벗어나 버림
//    skill->consumed_hp(0);
//    skill->consumed_mp(10);
//    skill->consumed_cp(2);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effec skill->caster_effect_value_2(0);
//    skill->caster_effec(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(1);
//    skill->global_cooldown_time(skill->cool_time() * 3);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillLongCastTimeSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(defaultLongCastTimeSkillCode);
//    skill->spell_package_index(2);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMagic);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//    skill->can_cast_on_moving(false);
//
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susAlaways);
//    skill->transition_state(stsNone),
//        skill->equip_part_allowed_equip(scetAny);
//
//    skill->cast_time(3000);
//    skill->cool_time(3000);
//    skill->min_distance(0);
//    skill->max_distance(500); // 2000이면 관심영역을 벗어나 버림
//    skill->consumed_hp(0);
//    skill->consumed_mp(10);
//    skill->consumed_cp(2);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(0);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillPeaceUseTypeSkill()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(peaceUseTypeSkillCode);
//    skill->spell_package_index(3);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susPeace);
//    skill->transition_state(stsCombat);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//
//void MockSkillTable::fillCombatUseTypeSkill() {
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(combatUseTypeSkillCode);
//    skill->spell_package_index(25);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susCombat);
//    skill->transition_state(stsCombat);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdFront);
//    skill->target_orientation(toFront);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//
//    //skill->global_cooldown_time(500);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillChangePeaceSkill()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(changePeaceSkillCode);
//    skill->spell_package_index(3);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susCombat);
//    skill->transition_state(susPeace);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->consumed_hp(10);
//    skill->consumed_mp(10);
//    skill->consumed_cp(10);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillLearnSkill_1()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(learnSkill_1);
//    skill->spell_package_index(4);
//    skill->skill_level(1);
//    skill->skill_type(sktAttackMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susPeace);
//    skill->transition_state(stsCombat);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->consumed_hp(10);
//    skill->consumed_mp(10);
//    skill->consumed_cp(10);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillLearnSkill_2()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(learnSkill_2);
//    skill->spell_package_index(4);
//    skill->skill_level(2);
//    skill->skill_type(sktAttackMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(stsCombat);
//    skill->transition_state(stsPeace);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(1);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//
//    //skill->global_cooldown_time(500);
//    skill->consumed_hp(10);
//    skill->consumed_mp(10);
//    skill->consumed_cp(10);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillLearnSkill_3()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(learnSkill_3);
//    skill->spell_package_index(4);
//    skill->skill_level(2);
//    skill->skill_type(sktAttackMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(false);
//
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(stsCombat);
//    skill->transition_state(stsCombat);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(estHpDamage);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(1);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//
//    skill->use_global_cooldown_time(0);
//
//    //skill->global_cooldown_time(500);
//    skill->consumed_hp(10);
//    skill->consumed_mp(10);
//    skill->consumed_cp(10);
//    skill->targeting_type(ttTarget);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//    skill->need_effect_script(estUnknown);
//    skill->consume_material(0);
//    skill->consume_material_count(0);
//    skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//
//void MockSkillTable::fillAbsolutenessDefence()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(absolutenessDefence);
//    skill->spell_package_index(6);
//    skill->skill_level(2);
//    skill->skill_type(sktBuffMelee);
//    skill->use_type(utActive);
//    skill->needed_sp(1);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(true);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(stsNone);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetShield);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(500);
//    skill->consumed_hp(1);
//    skill->consumed_mp(10);
//    skill->consumed_cp(1);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_spell_group_2(0);
//    skill->caster_effect_script_1(0);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_effect_script_2(estUnknown);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_spell_group_1(0);
//    skill->castee_effect_script_1(estBlockRateUp);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(10);
//    skill->castee_active_time_1(1000);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_spell_group_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->consumed_hp(10);
//    skill->consumed_mp(10);
//    skill->consumed_cp(10);
//    skill->targeting_type(ttSelf);
//    skill->cast_type(catDirect);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//
//void MockSkillTable::fillSpellCraft_1()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(spellCraft_1);
//    skill->spell_package_index(1);
//    skill->skill_level(1);
//    skill->skill_type(sktGeneral);
//    skill->use_type(utPassive);
//    skill->needed_sp(3);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(true);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susNotCast);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetAny);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(0);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(0);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_spell_group_1(0);
//    skill->caster_effect_script_1(estMagicPowerUp);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(60);
//    skill->caster_active_time_1(0);
//    skill->caster_effect_tick_time_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_spell_group_2(0);
//    skill->caster_effect_script_2(estMpCapacityUp);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(600);
//    skill->caster_active_time_2(0);
//    skill->caster_effect_tick_time_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(0);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(0);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(0);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(0);
//    skill->targeting_type(ttNoTarget);
//    skill->cast_type(catDefault);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//void MockSkillTable::fillSpellCraft_2()
//{
//    gdt::skill_t* skill = new gdt::skill_t;
//    skill->skill_code(spellCraft_2);
//    skill->spell_package_index(1);
//    skill->skill_level(1);
//    skill->skill_type(sktGeneral);
//    skill->use_type(utPassive);
//    skill->needed_sp(3);
//    skill->equip_part(epInvalid);
//    skill->allowed_class(ccAny);
//    skill->needed_skill_code(anySkillCode);
//    skill->can_cast_on_moving(true);
//    // TODO: test
//    //skill->usable_state(susAlaways);
//    //skill->transition_state(stsCombat),
//    skill->usable_state(susNotCast);
//    skill->transition_state(stsNone);
//    skill->equip_part_allowed_equip(scetShield);
//    skill->target_direction(tdNoCheck);
//    skill->target_orientation(toNoCheck);
//
//    skill->cast_time(0);
//    skill->cool_time(0);
//    skill->min_distance(0);
//    skill->max_distance(0);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(0);
//
//    skill->should_caster_critical_1(0);
//    skill->caster_spell_group_1(0);
//    skill->caster_effect_script_1(estMagicPowerUp);
//    skill->caster_effect_value_by_percent_1(0);
//    skill->caster_effect_value_1(70);
//    skill->caster_active_time_1(0);
//    skill->caster_effect_tick_time_1(0);
//    skill->caster_effect_type_1(etNone);
//
//    skill->should_caster_critical_2(0);
//    skill->caster_spell_group_2(0);
//    skill->caster_effect_script_2(estMpCapacityUp);
//    skill->caster_effect_value_by_percent_2(0);
//    skill->caster_effect_value_2(610);
//    skill->caster_active_time_2(0);
//    skill->caster_effect_tick_time_2(0);
//    skill->caster_effect_type_2(etNone);
//
//    skill->should_castee_critical_1(0);
//    skill->castee_effect_script_1(0);
//    skill->castee_effect_value_by_percent_1(0);
//    skill->castee_effect_value_1(0);
//    skill->castee_active_time_1(0);
//    skill->castee_effect_tick_time_1(0);
//    skill->castee_effect_type_1(etNone);
//
//    skill->should_castee_critical_2(1);
//    skill->castee_effect_script_2(estUnknown);
//    skill->castee_effect_value_by_percent_2(0);
//    skill->castee_effect_value_2(0);
//    skill->castee_active_time_2(0);
//    skill->castee_effect_tick_time_2(0);
//    skill->castee_effect_type_2(etNone);
//
//    skill->use_global_cooldown_time(0);
//    //skill->global_cooldown_time(500);
//    skill->consumed_hp(0);
//    skill->consumed_mp(0);
//    skill->consumed_cp(0);
//    skill->targeting_type(ttNoTarget);
//    skill->cast_type(catDefault);
//    skill->projectile_speed(0);
//	skill->need_effect_script(estUnknown);
//	skill->consume_material(0);
//	skill->consume_material_count(0);
//	skill->need_equipment(0);
//
//    skill->mez_check(cmtUnknown);
//
//    fillClientInfo(skill);
//
//    skillMap_.insert(
//        SkillMap::value_type(skill->skill_code(),
//            gideon::datatable::PlayerSkillTemplate(*skill)));
//}
//
//
//void MockSkillTable::destroySkills()
//{
//    for (SkillMap::value_type& value : skillMap_) {
//        datatable::SkillTemplate& st = value.second;
//        delete st.getSkillInfo();
//    }
//}

}} // namespace gideon { namespace servertest {
