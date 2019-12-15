#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockEquipTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const gdt::equip_t* MockEquipTable::getEquip(EquipCode code) const
{
    const EquipMap::const_iterator pos = equipMap_.find(code);
    if (pos != equipMap_.end()) {
        return (*pos).second;}
    return nullptr;
}


void MockEquipTable::fillOneHandSword()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(defaultOneHandSwordEquipCode);
    equip->equip_type(41);
    equip->wear_type(2);
 //   equip->hp_apply(0);
 //   equip->hp_sign(0);
 //   equip->mp_apply(0);
 //   equip->mp_sign(0);
 //   equip->cp_apply(0);
 //   equip->cp_sign(0);
 //   equip->recovery_cp_apply(0);
 //   equip->recovery_cp_sign(0);
 //   equip->attack_apply(0);
 //   equip->attack_sign(0);
 //   equip->spell_power_apply(0);
 //   equip->spell_power_sign(0);
 //   equip->critical_damage_apply(0);
 //   equip->critical_damage_sign(0);
 //   equip->critical_apply(0);
 //   equip->critical_sign(0);
 //   equip->avoid_apply(0);
 //   equip->avoid_sign(0);
 //   equip->block_apply(0);
 //   equip->block_sign(0);
 //   equip->resist_apply(0);
 //   equip->resist_sign(0);
 //   equip->critical_block_apply(0);
 //   equip->critical_block_sign(0);
 //   equip->previous_item_code(defaultOneHandSwordEquipCode);
 //   equip->original_fragment(45);
 //   equip->next_upgrade_fragment_count(50);
 //   equip->next_item_code(upgradeOneHandSwordEquipCode);
    equip->fragment_code(oneHandSwordFragmentCode);
 //   equip->absorptive_pct(1000);
 //   equip->open_socket_count(2);
 //   equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillUpgradeOneHandSword()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(upgradeOneHandSwordEquipCode);
    equip->equip_type(41);
    equip->wear_type(2);
 //   equip->hp_apply(0);
 //   equip->hp_sign(0);
 //   equip->mp_apply(0);
 //   equip->mp_sign(0);
 //   equip->cp_apply(0);
 //   equip->cp_sign(0);
 //   equip->recovery_cp_apply(0);
 //   equip->recovery_cp_sign(0);
 //   equip->attack_apply(0);
 //   equip->attack_sign(0);
 //   equip->spell_power_apply(0);
 //   equip->spell_power_sign(0);
 //   equip->critical_damage_apply(0);
 //   equip->critical_damage_sign(0);
 //   equip->critical_apply(0);
 //   equip->critical_sign(0);
 //   equip->avoid_apply(0);
 //   equip->avoid_sign(0);
 //   equip->block_apply(0);
 //   equip->block_sign(0);
 //   equip->resist_apply(0);
 //   equip->resist_sign(0);
 //   equip->critical_block_apply(0);
 //   equip->critical_block_sign(0);
 //   equip->previous_item_code(defaultOneHandSwordEquipCode);
 //   equip->original_fragment(50);
 //   equip->next_upgrade_fragment_count(0);
 //   equip->next_item_code(upgradeOneHandSwordEquipCode);
    equip->fragment_code(oneHandSwordFragmentCode);
 //   equip->absorptive_pct(1000);
 //   equip->open_socket_count(3);
 //   equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillShield()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(defaultShieldEquipCode);
    equip->equip_type(50);
    equip->wear_type(2);
 //   equip->hp_apply(0);
 //   equip->hp_sign(0);
 //   equip->mp_apply(0);
 //   equip->mp_sign(0);
 //   equip->cp_apply(0);
 //   equip->cp_sign(0);
 //   equip->recovery_cp_apply(0);
 //   equip->recovery_cp_sign(0);
 //   equip->attack_apply(0);
 //   equip->attack_sign(0);
 //   equip->spell_power_apply(0);
 //   equip->spell_power_sign(0);
 //   equip->critical_damage_apply(0);
 //   equip->critical_damage_sign(0);
 //   equip->critical_apply(0);
 //   equip->critical_sign(0);
 //   equip->avoid_apply(0);
 //   equip->avoid_sign(0);
 //   equip->block_apply(0);
 //   equip->block_sign(0);
 //   equip->resist_apply(0);
 //   equip->resist_sign(0);
 //   equip->critical_block_apply(0);
 //   equip->critical_block_sign(0);
 //   equip->previous_item_code(defaultShieldEquipCode);
 //   equip->original_fragment(45);
 //   equip->next_upgrade_fragment_count(50);
 //   equip->next_item_code(upgradeShieldEquipCode);
    equip->fragment_code(shieldFragmentCode);
 //   equip->absorptive_pct(1000);
 //   equip->open_socket_count(2);
 //   equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillUpgradeShield()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(upgradeShieldEquipCode);
    equip->equip_type(50);
    equip->wear_type(2);
 //   equip->hp_apply(0);
 //   equip->hp_sign(0);
 //   equip->mp_apply(0);
 //   equip->mp_sign(0);
 //   equip->cp_apply(0);
 //   equip->cp_sign(0);
 //   equip->recovery_cp_apply(0);
 //   equip->recovery_cp_sign(0);
 //   equip->attack_apply(0);
 //   equip->attack_sign(0);
 //   equip->spell_power_apply(0);
 //   equip->spell_power_sign(0);
 //   equip->critical_damage_apply(0);
 //   equip->critical_damage_sign(0);
 //   equip->critical_apply(0);
 //   equip->critical_sign(0);
 //   equip->avoid_apply(0);
 //   equip->avoid_sign(0);
 //   equip->block_apply(0);
 //   equip->block_sign(0);
 //   equip->resist_apply(0);
 //   equip->resist_sign(0);
 //   equip->critical_block_apply(0);
 //   equip->critical_block_sign(0);
 //   equip->previous_item_code(defaultShieldEquipCode);
 //   equip->original_fragment(50);
 //   equip->next_upgrade_fragment_count(0);
 //   equip->next_item_code(upgradeShieldEquipCode);
    equip->fragment_code(shieldFragmentCode);
 //   equip->absorptive_pct(1000);
 //   equip->open_socket_count(3);
 //   equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillHelmet()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(defaultHelmetEquipCode);
    equip->equip_type(5);
    equip->wear_type(1);
 //   equip->hp_apply(10);
 //   equip->hp_sign(0);
 //   equip->mp_apply(10);
 //   equip->mp_sign(0);
 //   equip->cp_apply(10);
 //   equip->cp_sign(0);
 //   equip->recovery_cp_apply(0);
 //   equip->recovery_cp_sign(0);
 //   equip->attack_apply(0);
 //   equip->attack_sign(0);
 //   equip->spell_power_apply(0);
 //   equip->spell_power_sign(0);
 //   equip->critical_damage_apply(0);
 //   equip->critical_damage_sign(0);
 //   equip->critical_apply(0);
 //   equip->critical_sign(0);
 //   equip->avoid_apply(0);
 //   equip->avoid_sign(0);
 //   equip->block_apply(0);
 //   equip->block_sign(0);
 //   equip->resist_apply(0);
 //   equip->resist_sign(0);
 //   equip->critical_block_apply(0);
 //   equip->critical_block_sign(0);
 //   equip->previous_item_code(defaultHelmetEquipCode);
 //   equip->original_fragment(45);
 //   equip->next_upgrade_fragment_count(50);
 //   equip->next_item_code(upgradeHelmetEquipCode);
    equip->fragment_code(babutaHelmetFragmentCode);
 //   equip->absorptive_pct(1000);
 //   equip->open_socket_count(2);
 //   equip->slot_point(4);
 //   equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillUpgradeHelmet()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(defaultHelmetEquipCode);
    equip->equip_type(5);
    equip->wear_type(1);
 //   equip->hp_apply(10);
 //   equip->hp_sign(0);
 //   equip->mp_apply(10);
 //   equip->mp_sign(0);
 //   equip->cp_apply(10);
 //   equip->cp_sign(0);
 //   equip->recovery_cp_apply(0);
 //   equip->recovery_cp_sign(0);
 //   equip->attack_apply(0);
 //   equip->attack_sign(0);
 //   equip->spell_power_apply(0);
 //   equip->spell_power_sign(0);
 //   equip->critical_damage_apply(0);
 //   equip->critical_damage_sign(0);
 //   equip->critical_apply(0);
 //   equip->critical_sign(0);
 //   equip->avoid_apply(0);
 //   equip->avoid_sign(0);
 //   equip->block_apply(0);
 //   equip->block_sign(0);
 //   equip->resist_apply(0);
 //   equip->resist_sign(0);
 //   equip->critical_block_apply(0);
 //   equip->critical_block_sign(0);
 //   equip->previous_item_code(defaultHelmetEquipCode);
 //   equip->original_fragment(50);
 //   equip->next_upgrade_fragment_count(0);
 //   equip->next_item_code(upgradeHelmetEquipCode);
    equip->fragment_code(babutaHelmetFragmentCode);
 //   equip->absorptive_pct(1000);
 //   equip->open_socket_count(3);
 //   equip->slot_point(4);
 //   equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillOtherHelmet()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(otherHelmetEquipCode);
    equip->equip_type(5);
    equip->wear_type(1);
    //equip->hp_apply(0);
    //equip->hp_sign(0);
    //equip->mp_apply(0);
    //equip->mp_sign(0);
    //equip->cp_apply(0);
    //equip->cp_sign(0);
    //equip->recovery_cp_apply(0);
    //equip->recovery_cp_sign(0);
    //equip->attack_apply(10);
    //equip->attack_sign(0);
    //equip->spell_power_apply(0);
    //equip->spell_power_sign(0);
    //equip->critical_damage_apply(50);
    //equip->critical_damage_sign(0);
    //equip->critical_apply(1000);
    //equip->critical_sign(0);
    //equip->avoid_apply(0);
    //equip->avoid_sign(0);
    //equip->block_apply(0);
    //equip->block_sign(0);
    //equip->resist_apply(100);
    //equip->resist_sign(0);
    //equip->critical_block_apply(100);
    //equip->critical_block_sign(0);
    //equip->previous_item_code(otherHelmetEquipCode);
    //equip->original_fragment(45);
    //equip->next_upgrade_fragment_count(50);
    //equip->next_item_code(upgradeOtherHelmetEquipCode);
    equip->fragment_code(shabbyHelmatFragmentCode);
    //equip->absorptive_pct(1000);
    //equip->open_socket_count(2);
    //equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillUpgradeOtherHelmet()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(upgradeOtherHelmetEquipCode);
    equip->equip_type(5);
    equip->wear_type(1);
    //equip->hp_apply(0);
    //equip->hp_sign(0);
    //equip->mp_apply(0);
    //equip->mp_sign(0);
    //equip->cp_apply(0);
    //equip->cp_sign(0);
    //equip->recovery_cp_apply(0);
    //equip->recovery_cp_sign(0);
    //equip->attack_apply(10);
    //equip->attack_sign(0);
    //equip->spell_power_apply(0);
    //equip->spell_power_sign(0);
    //equip->critical_damage_apply(50);
    //equip->critical_damage_sign(0);
    //equip->critical_apply(1000);
    //equip->critical_sign(0);
    //equip->avoid_apply(0);
    //equip->avoid_sign(0);
    //equip->block_apply(0);
    //equip->block_sign(0);
    //equip->resist_apply(100);
    //equip->resist_sign(0);
    //equip->critical_block_apply(100);
    //equip->critical_block_sign(0);
    //equip->previous_item_code(otherHelmetEquipCode);
    //equip->original_fragment(50);
    //equip->next_upgrade_fragment_count(0);
    //equip->next_item_code(upgradeOtherHelmetEquipCode);
    equip->fragment_code(shabbyHelmatFragmentCode);
    //equip->absorptive_pct(1000);
    //equip->open_socket_count(3);
    //equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillLance()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(lanceEquipCode);
    equip->equip_type(45);
    equip->wear_type(2);
    //equip->hp_apply(0);
    //equip->hp_sign(0);
    //equip->mp_apply(0);
    //equip->mp_sign(0);
    //equip->cp_apply(0);
    //equip->cp_sign(0);
    //equip->recovery_cp_apply(0);
    //equip->recovery_cp_sign(0);
    //equip->attack_apply(0);
    //equip->attack_sign(0);
    //equip->spell_power_apply(0);
    //equip->spell_power_sign(0);
    //equip->critical_damage_apply(0);
    //equip->critical_damage_sign(0);
    //equip->critical_apply(0);
    //equip->critical_sign(0);
    //equip->avoid_apply(0);
    //equip->avoid_sign(0);
    //equip->block_apply(0);
    //equip->block_sign(0);
    //equip->resist_apply(0);
    //equip->resist_sign(0);
    //equip->critical_block_apply(0);
    //equip->critical_block_sign(0);
    //equip->previous_item_code(lanceEquipCode);
    //equip->original_fragment(45);
    //equip->next_upgrade_fragment_count(50);
    //equip->next_item_code(upgradeLanceEquipCode);
    equip->fragment_code(lanceFragmentCode);
    //equip->absorptive_pct(1000);
    //equip->open_socket_count(2);
    //equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillUpgradeLance()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(lanceEquipCode);
    equip->equip_type(45);
    equip->wear_type(2);
    //equip->hp_apply(0);
    //equip->hp_sign(0);
    //equip->mp_apply(0);
    //equip->mp_sign(0);
    //equip->cp_apply(0);
    //equip->cp_sign(0);
    //equip->recovery_cp_apply(0);
    //equip->recovery_cp_sign(0);
    //equip->attack_apply(0);
    //equip->attack_sign(0);
    //equip->spell_power_apply(0);
    //equip->spell_power_sign(0);
    //equip->critical_damage_apply(0);
    //equip->critical_damage_sign(0);
    //equip->critical_apply(0);
    //equip->critical_sign(0);
    //equip->avoid_apply(0);
    //equip->avoid_sign(0);
    //equip->block_apply(0);
    //equip->block_sign(0);
    //equip->resist_apply(0);
    //equip->resist_sign(0);
    //equip->critical_block_apply(0);
    //equip->critical_block_sign(0);
    //equip->previous_item_code(lanceEquipCode);
    //equip->original_fragment(50);
    //equip->next_upgrade_fragment_count(0);
    //equip->next_item_code(upgradeLanceEquipCode);
    equip->fragment_code(lanceFragmentCode);
    //equip->absorptive_pct(1000);
    //equip->open_socket_count(3);
    //equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillShoes()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(shoesEquipCode);
    equip->equip_type(32);
    equip->wear_type(1);
    //equip->hp_apply(0);
    //equip->hp_sign(0);
    //equip->mp_apply(0);
    //equip->mp_sign(0);
    //equip->cp_apply(0);
    //equip->cp_sign(0);
    //equip->recovery_cp_apply(0);
    //equip->recovery_cp_sign(0);
    //equip->attack_apply(0);
    //equip->attack_sign(0);
    //equip->spell_power_apply(0);
    //equip->spell_power_sign(0);
    //equip->critical_damage_apply(0);
    //equip->critical_damage_sign(0);
    //equip->critical_apply(0);
    //equip->critical_sign(0);
    //equip->avoid_apply(0);
    //equip->avoid_sign(0);
    //equip->block_apply(0);
    //equip->block_sign(0);
    //equip->resist_apply(0);
    //equip->resist_sign(0);
    //equip->critical_block_apply(0);
    //equip->critical_block_sign(0);
    //equip->previous_item_code(shoesEquipCode);
    //equip->original_fragment(45);
    //equip->next_upgrade_fragment_count(50);
    //equip->next_item_code(upgradeShoesEquipCode);
    equip->fragment_code(shoesFragmentCode);
    //equip->absorptive_pct(1000);
    //equip->open_socket_count(3);
    //equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::fillUpgradeShoes()
{
    gdt::equip_t* equip = new gdt::equip_t;
    equip->item_code(shoesEquipCode);
    equip->equip_type(32);
    equip->wear_type(1);
    //equip->hp_apply(0);
    //equip->hp_sign(0);
    //equip->mp_apply(0);
    //equip->mp_sign(0);
    //equip->cp_apply(0);
    //equip->cp_sign(0);
    //equip->recovery_cp_apply(0);
    //equip->recovery_cp_sign(0);
    //equip->attack_apply(0);
    //equip->attack_sign(0);
    //equip->spell_power_apply(0);
    //equip->spell_power_sign(0);
    //equip->critical_damage_apply(0);
    //equip->critical_damage_sign(0);
    //equip->critical_apply(0);
    //equip->critical_sign(0);
    //equip->avoid_apply(0);
    //equip->avoid_sign(0);
    //equip->block_apply(0);
    //equip->block_sign(0);
    //equip->resist_apply(0);
    //equip->resist_sign(0);
    //equip->critical_block_apply(0);
    //equip->critical_block_sign(0);
    //equip->previous_item_code(shoesEquipCode);
    //equip->original_fragment(50);
    //equip->next_upgrade_fragment_count(0);
    //equip->next_item_code(upgradeShoesEquipCode);
    equip->fragment_code(shoesFragmentCode);
    //equip->absorptive_pct(1000);
    //equip->open_socket_count(2);
    //equip->slot_point(4);
    //equip->downgrade_pct(0);
    equipMap_.emplace(equip->item_code(), equip);
}


void MockEquipTable::destroyItems()
{
    for (const EquipMap::value_type& value : equipMap_) {
        delete value.second;
    }
}

}} // namespace gideon { namespace servertest {
