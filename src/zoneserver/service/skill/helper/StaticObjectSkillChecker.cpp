#include "ZoneServerPCH.h"
#include "StaticObjectSkillChecker.h"
#include "../../../model/gameobject/StaticObject.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/StaticObjectOwerable.h"
#include "../../../model/gameobject/ability/Partyable.h"
#include "../../../model/gameobject/ability/Guildable.h"
#include "../../party/PartyService.h"
#include <gideon/cs/datatable/SkillTemplate.h>


namespace gideon { namespace zoneserver {

StaticObjectSkillChecker::StaticObjectSkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
    const datatable::SkillEffectTemplate& effectTemplate, const datatable::SOActiveSkillTemplate& skillTemplate) :
    SkillChecker(caster, shouldCheckSafeRegion, effectTemplate),
    skillTemplate_(skillTemplate)
{

}


ErrorCode StaticObjectSkillChecker::canCastable() const
{
    ErrorCode errorCode = SkillChecker::canCastable();
    if (isFailed(errorCode)) {
        return errorCode;
    }

    return checkCasterPointStats();
}


bool StaticObjectSkillChecker::checkConcentrationSkill(go::Entity* /*target*/, const Position& /*targetPosition*/) const
{
    return false;
}

const datatable::ActiveSkillTemplate& StaticObjectSkillChecker::getActiveSkillTemplate() const
{
    return skillTemplate_;
}


bool StaticObjectSkillChecker::canCastableState() const
{
    // TODO: 액티브 상태
    return true;
}


ErrorCode StaticObjectSkillChecker::checkCasterPointStats() const
{
    //const go::StaticObject& caster = static_cast<const go::StaticObject&>(caster_);
    //const HitPoints currentPoints = caster.getCurrentPoints();
    //const datatable::StaticObjectSkillCastInfo& soSkillCastInfo = skillTemplate_.getStaticObjectSkillCastInfo();
    //if ((soSkillCastInfo.consumedHp_ > 0) && (currentPoints.hp_ < soSkillCastInfo.consumedHp_)) {
    //    return ecSkillNotEnoughStats;
    //}

    return ecOk;
}



}} // namespace gideon { namespace zoneserver {