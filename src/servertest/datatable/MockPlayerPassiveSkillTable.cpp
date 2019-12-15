#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockPlayerPassiveSkillTable.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/template/player_passive_skill_table.hxx>

namespace gideon { namespace servertest {

namespace {

// TODO: 패시브 스킬도 애니메이션이 있남? - 2019-04-02
void fillClientInfo(gdt::player_passive_skill_t* skill)
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

const datatable::PassiveSkillTemplate*
    MockPlayerPassiveSkillTable::getPlayerSkill(SkillCode code) const
{
    const PlayerSkillMap::const_iterator pos = skillMap_.find(code);
    if (pos != skillMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockPlayerPassiveSkillTable::fillSpellCraft_1()
{
    gdt::player_passive_skill_t* skill = new gdt::player_passive_skill_t;
    skill->skill_code(spellCraft_1);
    skill->effect_level(1);
    skill->skill_type(sktGeneral);
    skill->need_skill_point(3);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetAny);
    skill->learn_character_class(ccAny);
    //skill->need_learn_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    //skill->need_learn_skill_code_2(gdt::any_skill_code_t(anySkillCode));
    //skill->next_skill_code_1(gdt::any_skill_code_t(anySkillCode));
    //skill->next_skill_code_2(gdt::any_skill_code_t(anySkillCode));
    skill->usable_state(susNotCast);

    skill->cooldown_time(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PassiveSkillTemplate(*skill));
}


void MockPlayerPassiveSkillTable::fillSpellCraft_2()
{
    gdt::player_passive_skill_t* skill = new gdt::player_passive_skill_t;
    skill->skill_code(spellCraft_2);
    skill->effect_level(1);
    skill->skill_type(sktGeneral);
    skill->need_skill_point(3);
    skill->check_equip_part(epInvalid);
    skill->check_castable_equip_type(scetShield);
    skill->learn_character_class(ccAny);
    skill->usable_state(susNotCast);

    skill->cooldown_time(0);

    fillClientInfo(skill);

    skillMap_.emplace(skill->skill_code(), datatable::PassiveSkillTemplate(*skill));
}

}} // namespace gideon { namespace servertest {
