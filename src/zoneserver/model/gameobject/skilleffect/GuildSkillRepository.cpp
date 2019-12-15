#include "ZoneServerPCH.h"
#include "GuildSkillRepository.h"
#include "../Player.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/GuildCallback.h"
#include <gideon/cs/datatable/GuildSkillTable.h>

namespace gideon { namespace zoneserver { namespace go {

GuildSkillRepository::GuildSkillRepository(Player& owner) :
    owner_(owner)
{
}


bool GuildSkillRepository::initialize(const SkillCodes& skillCodes)
{
    guildSkillManager_.initialize(skillCodes);
    for (SkillCode skillCode : skillCodes) {
        const gdt::guild_skill_t* guildSkill = GUILD_SKILL_TABLE->getGuildSkill(skillCode);
        if (! guildSkill) {
            assert(false);
            return false;
        }
        const EffectScriptType effectSciptType = toEffectScriptType(guildSkill->effect_script_type());
        bool result = effectMap_.insert(EffectMap::value_type(effectSciptType, guildSkill->effect_value())).second;
        if (! result) {
            assert(false);
            return false;
        }
    }
    return true;
}

void GuildSkillRepository::clear()
{
    guildSkillManager_.finalize();
    effectMap_.clear();
}


void GuildSkillRepository::activateSkill(SkillCode skillCode)
{   
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    const SkillTableType skillTableType = getSkillTableType(skillCode);
    const SkillIndex skillIndex = getSkillIndex(skillCode);
    guildSkillManager_.removeSkill(skillTableType, skillIndex);
    guildSkillManager_.updateSkill(skillCode);

    const gdt::guild_skill_t* guildSkill = GUILD_SKILL_TABLE->getGuildSkill(skillCode);
    if (! guildSkill) {
        assert(false);
        return;
    }


    const EffectScriptType effectSciptType = toEffectScriptType(guildSkill->effect_script_type());
    effectMap_.erase(effectSciptType);
    effectMap_.emplace(effectSciptType, guildSkill->effect_value()); 
    
}


void GuildSkillRepository::deactivateSkills()
{    
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    guildSkillManager_.finalize();
    effectMap_.clear();    
}


int32_t GuildSkillRepository::getEffectValue(EffectScriptType type) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    EffectMap::const_iterator pos = effectMap_.find(type);
    if (pos != effectMap_.end()) {
        return (*pos).second;
    }
    return 0;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
