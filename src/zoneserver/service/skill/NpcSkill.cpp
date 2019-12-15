#include "ZoneServerPCH.h"
#include "NpcSkill.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Factionable.h"
#include "../../helper/CastChecker.h"
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver {


// = Skill

NpcSkill::NpcSkill(go::Entity& caster, bool shouldCheckSafeRegion, 
    const datatable::SkillEffectTemplate& skillEffectTemplate,
    const datatable::NpcActiveSkillTemplate& skillTemplate) :
    CreatureSkill(caster, shouldCheckSafeRegion, skillEffectTemplate),
    skillTemplate_(skillTemplate),
    npcSkillChecker_(caster, shouldCheckSafeRegion, skillEffectTemplate, skillTemplate)
{
}


NpcSkill::~NpcSkill()
{
}


const datatable::ActiveSkillTemplate& NpcSkill::getActiveSkillTemplate() const
{
    return skillTemplate_;
}


const datatable::SkillCastCheckPointInfo& NpcSkill::getSkillCastCheckPointInfo() const
{
    return skillTemplate_.castCheckPoint_;
}
}} // namespace gideon { namespace zoneserver {
