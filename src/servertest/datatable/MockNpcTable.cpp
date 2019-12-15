#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockNpcTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const datatable::NpcTemplate* MockNpcTable::getNpcTemplate(NpcCode code) const
{
    const NpcMap::const_iterator pos = monsterMap_.find(code);
    if (pos != monsterMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockNpcTable::fillNormalMonster()
{
    gdt::npc_t* monster = new gdt::npc_t;
    monster->npc_code(normalNpcCode);
    monster->npc_type(ntMonster);
    monster->level(10);
    monster->grade(mgMutantInfant);
    monster->exp(50);
    monster->hp(hpDefault);
    monster->mp(mpDefault);
    //monster->cp(cpDefault);
    //monster->npc_drop_code_1(normalNpcDropCode);
    //monster->npc_drop_code_2(invalidNpcDropCode);
    //monster->looting_pct(1100);
    monster->ai_reactive_type(gdt::ai_reactive_type_t(0));
    //monster->ai_reactive_distance(0);
    //monster->ai_wander_distance(0);
    //monster->ai_walk_speed(100);
    monster->ai_script_name(L"");
    monster->main_skill_code(::gdt::any_skill_code_t(invalidSkillCode));
    monster->sub_skill_code(::gdt::any_skill_code_t(invalidSkillCode));

    //monster->recovery_cp_apply(0);
    //monster->attack_apply(0);
    //monster->spell_power_apply(0);
    //monster->critical_block_apply(0);
    //monster->critical_damage_apply(0);
    //monster->avoid_apply(0);
    //monster->block_apply(0);
    //monster->critical_block_apply(0);
    //monster->resist_apply(0);
    monster->sell_function(1);
    monster->buy_function(1);

    monster->npc_righthand(gdt::equip_code_t(invalidEquipCode));
    monster->npc_lefthand(gdt::equip_code_t(invalidEquipCode));

    monster->special_skill_1(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_1(0);
    monster->special_skill_2(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_2(0);
    monster->special_skill_3(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_3(0);

    monster->faction(defaultMonsterFactionCode);

    gdt::float_list_t scaleList;
    scaleList.push_back(static_cast<float>(1.0f));
    monster->scale(scaleList);
    
    monsterMap_.insert(
        NpcMap::value_type(monster->npc_code(),
        datatable::NpcTemplate(*monster)));
}


void MockNpcTable::fillServantMonster()
{
    gdt::npc_t* monster = new gdt::npc_t;
    monster->npc_code(servantNpcCode);
    monster->npc_type(ntMonster);
    monster->level(15);
    monster->grade(mgInfant);
    monster->exp(100);
    monster->hp(hpDefault);
    monster->mp(mpDefault);
    //monster->max_cp(cpDefault);
    //monster->npc_drop_code_1(servantNpcDropCode);
    //monster->npc_drop_code_2(invalidNpcDropCode);
    //monster->looting_pct(1000);
    monster->ai_reactive_type(gdt::ai_reactive_type_t(0));
    //monster->ai_reactive_distance(0);
    //monster->ai_wander_distance(0);
    //monster->ai_walk_speed(100);
    monster->ai_script_name(L"");
    monster->main_skill_code(::gdt::any_skill_code_t(invalidSkillCode));
    monster->sub_skill_code(::gdt::any_skill_code_t(invalidSkillCode));

    //monster->recovery_cp_apply(0);
    //monster->attack_apply(0);
    //monster->spell_power_apply(0);
    //monster->critical_block_apply(0);
    //monster->critical_damage_apply(0);
    //monster->avoid_apply(0);
    //monster->block_apply(0);
    //monster->critical_block_apply(0);
    //monster->resist_apply(0);
    monster->sell_function(1);
    monster->buy_function(1);

    monster->npc_righthand(gdt::equip_code_t(invalidEquipCode));
    monster->npc_lefthand(gdt::equip_code_t(invalidEquipCode));

    monster->special_skill_1(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_1(0);
    monster->special_skill_2(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_2(0);
    monster->special_skill_3(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_3(0);

    monster->faction(defaultMonsterFactionCode);

    gdt::float_list_t scaleList;
    scaleList.push_back(static_cast<float>(1.0f));
    monster->scale(scaleList);

    monsterMap_.insert(
        NpcMap::value_type(monster->npc_code(),
        datatable::NpcTemplate(*monster)));
}


void MockNpcTable::fillBossMonster()
{
    gdt::npc_t* monster = new gdt::npc_t;
    monster->npc_code(bossNpcCode);
    monster->npc_type(ntMonster);
    monster->level(20);
    monster->grade(mgBoss);
    monster->exp(200);
    monster->hp(hpDefault);
    monster->mp(mpDefault);
    //monster->max_cp(cpDefault);
    //monster->npc_drop_code_1(bossNpcDropCode);
    //monster->npc_drop_code_2(invalidNpcDropCode);
    //monster->looting_pct(900);
    monster->ai_reactive_type(gdt::ai_reactive_type_t(0));
    //monster->ai_reactive_distance(0);
    //monster->ai_wander_distance(0);
    //monster->ai_walk_speed(100);
    monster->ai_script_name(L"");
    monster->main_skill_code(::gdt::any_skill_code_t(invalidSkillCode));
    monster->sub_skill_code(::gdt::any_skill_code_t(invalidSkillCode));
    //
    //monster->recovery_cp_apply(0);
    //monster->attack_apply(0);
    //monster->spell_power_apply(0);
    //monster->critical_block_apply(0);
    //monster->critical_damage_apply(0);
    //monster->avoid_apply(0);
    //monster->block_apply(0);
    //monster->critical_block_apply(0);
    //monster->resist_apply(0);
    monster->sell_function(1);
    monster->buy_function(1);

    monster->npc_righthand(gdt::equip_code_t(invalidEquipCode));
    monster->npc_lefthand(gdt::equip_code_t(invalidEquipCode));

    monster->special_skill_1(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_1(0);
    monster->special_skill_2(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_2(0);
    monster->special_skill_3(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_3(0);

    monster->faction(defaultMonsterFactionCode);

    gdt::float_list_t scaleList;
    scaleList.push_back(1.0f);
    monster->scale(scaleList);

    monsterMap_.emplace(monster->npc_code(), datatable::NpcTemplate(*monster));
}


void MockNpcTable::fillNpcQuest()
{
    gdt::npc_t* monster = new gdt::npc_t;
    monster->npc_code(questNpcCode);
    monster->npc_type(ntMonster);
    monster->level(20);
    monster->grade(mgBoss);
    monster->exp(0);
    monster->hp(hpDefault);
    monster->mp(mpDefault);
    //monster->max_cp(cpDefault);
    //monster->npc_drop_code_1(0);
    //monster->npc_drop_code_2(0);
    //monster->looting_pct(0);
    monster->ai_reactive_type(gdt::ai_reactive_type_t(0));
    //monster->ai_reactive_distance(0);
    //monster->ai_wander_distance(0);
    //monster->ai_walk_speed(0);
    monster->ai_script_name(L"");
    monster->main_skill_code(::gdt::any_skill_code_t(invalidSkillCode));
    monster->sub_skill_code(::gdt::any_skill_code_t(invalidSkillCode));

    //monster->recovery_cp_apply(0);
    //monster->attack_apply(0);
    //monster->spell_power_apply(0);
    //monster->critical_block_apply(0);
    //monster->critical_damage_apply(0);
    //monster->avoid_apply(0);
    //monster->block_apply(0);
    //monster->critical_block_apply(0);
    //monster->resist_apply(0);
    monster->sell_function(1);
    monster->buy_function(1);

    monster->npc_righthand(gdt::equip_code_t(invalidEquipCode));
    monster->npc_lefthand(gdt::equip_code_t(invalidEquipCode));

    monster->special_skill_1(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_1(0);
    monster->special_skill_2(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_2(0);
    monster->special_skill_3(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_3(0);

    monster->faction(defaultMonsterFactionCode);

    gdt::float_list_t scaleList;
    scaleList.push_back(1.0f);
    monster->scale(scaleList);

    monsterMap_.emplace(monster->npc_code(), datatable::NpcTemplate(*monster));
}


void MockNpcTable::fillStoreNpc()
{
    gdt::npc_t* monster = new gdt::npc_t;
    monster->npc_code(storeNpcCode);
    monster->npc_type(ntNpc);
    monster->level(20);
    monster->grade(mgBoss);
    monster->exp(0);
    monster->hp(hpDefault);
    monster->mp(mpDefault);
    //monster->max_cp(cpDefault);
    //monster->npc_drop_code_1(0);
    //monster->npc_drop_code_2(0);
    //monster->looting_pct(0);
    monster->ai_reactive_type(gdt::ai_reactive_type_t(0));
    //monster->ai_reactive_distance(0);
    //monster->ai_wander_distance(0);
    //monster->ai_walk_speed(0);
    monster->ai_script_name(L"");
    monster->main_skill_code(::gdt::any_skill_code_t(invalidSkillCode));
    monster->sub_skill_code(::gdt::any_skill_code_t(invalidSkillCode));

    //monster->recovery_cp_apply(0);
    //monster->attack_apply(0);
    //monster->spell_power_apply(0);
    //monster->critical_block_apply(0);
    //monster->critical_damage_apply(0);
    //monster->avoid_apply(0);
    //monster->block_apply(0);
    //monster->critical_block_apply(0);
    //monster->resist_apply(0);
    monster->sell_function(1);
    monster->buy_function(1);

    monster->npc_righthand(gdt::equip_code_t(invalidEquipCode));
    monster->npc_lefthand(gdt::equip_code_t(invalidEquipCode));

    monster->special_skill_1(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_1(0);
    monster->special_skill_2(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_2(0);
    monster->special_skill_3(::gdt::any_skill_code_t(0));
    monster->special_skill_rate_3(0);

    monster->faction(defaultMonsterFactionCode);

    gdt::float_list_t scaleList;
    scaleList.push_back(1.0f);
    monster->scale(scaleList);

    monsterMap_.emplace(monster->npc_code(), datatable::NpcTemplate(*monster));
}


void MockNpcTable::destroyMonsters()
{
    for (NpcMap::value_type& value : monsterMap_) {
        datatable::NpcTemplate& nt = value.second;
        nt.deleteNpcInfo();
    }
}

}} // namespace gideon { namespace servertest {
