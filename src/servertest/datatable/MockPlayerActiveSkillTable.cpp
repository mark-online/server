#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockPlayerActiveSkillTable.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/template/player_active_skill_table.hxx>

namespace gideon { namespace servertest {

namespace {

void fillClientInfo(gdt::player_active_skill_t* skill)
{
    skill->casting_animation_name(L"");
    skill->casting_draw_animation_name(L"");
    skill->caster_animation_name(L"");
    skill->castee_animation_name(L"");
    skill->casting_effect(L"");
    skill->attacker1_caster_effect(L"");
    skill->attacker2_caster_effect(L"");
    skill->attacked1_castee_effect(L"");
    skill->attacked2_castee_effect(L"");
    skill->area_effect(L"");
    skill->caster_effect(L"");
    skill->trail_effect(L"");
}

} // namespace

const datatable::PlayerActiveSkillTemplate*
    MockPlayerActiveSkillTable::getPlayerSkill(SkillCode code) const
{
    const PlayerSkillMap::const_iterator pos = skillMap_.find(code);
    if (pos != skillMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockPlayerActiveSkillTable::fillObserverSkillCode()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(testObserverSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktGeneral);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone),
    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(0);
    skill->consumed_mp(0);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillStunSkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(stunSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktGeneral);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone),
    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(0);
    skill->consumed_mp(0);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillGraveStoneSkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(graveStoneSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktGeneral);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone),
    skill->cast_time(0);
    skill->cooldown_time(3000);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(0);
    skill->consumed_mp(0);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillMpRecoverySkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(mpRecoverySkillCode);
    skill->effect_level(1);
    skill->skill_type(sktBuffMagic);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),

    skill->usable_state(susAlaways);
    skill->transition_state(stsNone),
    skill->cast_time(0);
    skill->cooldown_time(2000);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(0);
    skill->consumed_mp(0);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillHpRecoverySkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(hpRecoverySkillCode);
    skill->effect_level(1);
    skill->skill_type(sktBuffMagic);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone);

    skill->cast_time(0);
    skill->cooldown_time(2000);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(1);
    skill->consumed_mp(10);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillMeleeAttackSkill() {
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(defaultMeleeAttackSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone);

    skill->cast_time(0);
    skill->cooldown_time(2000);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(1);
    skill->consumed_mp(1);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillRangeAttackSkill() {
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(defaultRangeAttackSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackRange);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    //  TODO : test
    //skill->usable_state(susCombat);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone);

    skill->cast_time(0);
    skill->cooldown_time(2000);
    skill->min_distance(300);
    skill->max_distance(500);
    skill->consumed_hp(20);
    skill->consumed_mp(10);

    skill->cast_type(catProjectile);
    skill->projectile_speed(25);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillMagicAttackSkill() {
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(defaultMagicAttackSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMagic);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susCombat);
    //skill->transition_state(stsNone),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone);

    skill->cast_time(0);
    skill->cooldown_time(3000);
    skill->min_distance(300);
    skill->max_distance(500); // 2000이면 관심영역을 벗어나 버림
    skill->consumed_hp(0);
    skill->consumed_mp(10);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillAreaSkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(defaultAreaAndCastee2SkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMagic);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);

    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone);

    skill->cast_time(0);
    skill->cooldown_time(3000);
    skill->min_distance(0);
    skill->max_distance(500); // 20이면 관심영역을 벗어나 버림
    skill->consumed_hp(0);
    skill->consumed_mp(10);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillTargetAreaSkill() {
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(defaultTargetAreaSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMagic);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone);

    skill->cast_time(0);
    skill->cooldown_time(3000);
    skill->min_distance(0);
    skill->max_distance(500); // 20이면 관심영역을 벗어나 버림
    skill->consumed_hp(0);
    skill->consumed_mp(10);
    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillLongGlobalCooldownTimeSkill() {
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(defaultLongGlobalCooldownTimeSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMagic);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);

    // TODO: test
    //skill->usable_state(susCombat);
    //skill->transition_state(stsPeace),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone);


    skill->cast_time(0);
    skill->cooldown_time(3000);
    skill->min_distance(0);
    skill->max_distance(500); // 2000이면 관심영역을 벗어나 버림
    skill->consumed_hp(0);
    skill->consumed_mp(10);

    skill->global_cooldown_time(skill->cooldown_time() * 3);
    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillLongCastTimeSkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(defaultLongCastTimeSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMagic);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);

    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susAlaways);
    skill->transition_state(stsNone),

    skill->cast_time(3000);
    skill->cooldown_time(3000);
    skill->min_distance(0);
    skill->max_distance(500); // 2000이면 관심영역을 벗어나 버림
    skill->consumed_hp(0);
    skill->consumed_mp(10);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillPeaceUseTypeSkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(peaceUseTypeSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susPeace);
    skill->transition_state(stsCombat);

    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(1);
    skill->consumed_mp(10);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillCombatUseTypeSkill() {
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(combatUseTypeSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susCombat);
    skill->transition_state(stsCombat);

    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(1);
    skill->consumed_mp(10);

    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillChangePeaceSkill()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(changePeaceSkillCode);
    skill->effect_level(1);
    skill->skill_type(sktAttackMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susCombat);
    skill->transition_state(susPeace);

    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(10);
    skill->consumed_mp(10);
    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillLearnSkill_1()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(learnSkill_1);
    skill->effect_level(1);
    skill->skill_type(sktAttackMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(susPeace);
    skill->transition_state(stsCombat);

    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(10);
    skill->consumed_mp(10);
    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillLearnSkill_2()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(learnSkill_2);
    skill->effect_level(2);
    skill->skill_type(sktAttackMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);

    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(stsCombat);
    skill->transition_state(stsPeace);

    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(10);
    skill->consumed_mp(10);
    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillLearnSkill_3()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(learnSkill_3);
    skill->effect_level(2);
    skill->skill_type(sktAttackMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(false);

    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(stsCombat);
    skill->transition_state(stsCombat);

    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(10);
    skill->consumed_mp(10);
    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}


void MockPlayerActiveSkillTable::fillAbsolutenessDefence()
{
    gdt::player_active_skill_t* skill = new gdt::player_active_skill_t;
    skill->skill_code(absolutenessDefence);
    skill->effect_level(2);
    skill->skill_type(sktBuffMelee);
    skill->need_skill_point(1);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetShield);
    skill->learn_character_class(ccAny);
    skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    skill->can_cast_on_moving(true);
    // TODO: test
    //skill->usable_state(susAlaways);
    //skill->transition_state(stsCombat),
    skill->usable_state(stsNone);
    skill->transition_state(stsNone);

    skill->cast_time(0);
    skill->cooldown_time(0);
    skill->min_distance(0);
    skill->max_distance(500);
    skill->consumed_hp(10);
    skill->consumed_mp(10);
    skill->cast_type(catDirect);
    skill->projectile_speed(0);
    skill->consume_item_code(0);
    skill->consume_item_count(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PlayerActiveSkillTemplate(*skill));
}

}} // namespace gideon { namespace servertest {
