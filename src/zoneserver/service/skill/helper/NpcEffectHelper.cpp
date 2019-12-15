#include "ZoneServerPCH.h"
#include "NpcEffectHelper.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/Factionable.h"
#include <gideon/cs/datatable/SkillEffectTable.h>

namespace gideon { namespace zoneserver {

NpcEffectHelper::NpcEffectHelper(go::Entity& caster) :
    CreatureEffectHelper(caster)
{

}


NpcEffectHelper::~NpcEffectHelper()
{

}


ErrorCode NpcEffectHelper::canTargetEffected(SkillType skillType, bool isCheckSafeRegion, EffectLevel effectLevel,
    const datatable::SkillCastCheckPointInfo& checkPointInfo, const datatable::SkillEffectTemplate& effectTemplate, go::Entity& target) const
{
    ErrorCode errorCode = EffectHelper::canTargetEffected(skillType, isCheckSafeRegion, effectLevel, checkPointInfo, effectTemplate, target);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    return checkTargetEffectable(skillType, target);
}


ErrorCode NpcEffectHelper::checkTargetEffectable(SkillType skillType, go::Entity& target) const
{    
    const go::Factionable* factionable = getCaster().queryFactionable();
    if (! factionable) {
        return ecOk;
    }

    if (! factionable->isFriendlyTo(target)) {
        return ecOk;
    }

    if ((! isAttackSkillType(skillType)) && (! isDebuffSkillType(skillType))) {
        return ecOk;
    }

    return ecSkillInvalidAvailableTarget;
}


bool NpcEffectHelper::isAvailableTarget(go::Entity& target, AvailableTarget availableTarget) const
{
    if (target.isNpcOrMonster()) {
        if (static_cast<go::Npc&>(target).isStaticNpc()) {
            return false;
        }
    }

    if (availableTarget == atAll) {
        return true;
    }

    if (availableTarget == atSelf) {
        if (! target.isSame(getCaster().getGameObjectInfo())) {
            return false;
        }        
    }    
    else if (availableTarget == atMoster) {
        if (! target.isMonster()) {
            return false;
        }
    }

    return false;
}


bool NpcEffectHelper::isExceptTarget(go::Entity& target, ExceptTarget exceptTarget) const
{
    if (etNoneTaget ==  exceptTarget) {
        return false;   
    }

    if (etSelf == exceptTarget) {
        return getCaster().isSame(target);
    }

    if (etNpcOrMonster == exceptTarget) {
        return target.isNpcOrMonster();
    }

    if (etNpcOrMosterOrSelf == exceptTarget) {
        if (getCaster().isSame(target) || target.isMonster()) {
            return true;
        }
    }

    return false;
}

}} // namespace gideon { namespace zoneserver {

