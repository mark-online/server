#include "ZoneServerPCH.h"
#include "NpcSkillChecker.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/Factionable.h"
#include "../../../model/state/SkillCasterState.h"
#include "../../../model/state/CreatureState.h"
#include "../../../helper/CastChecker.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/SkillEffectTable.h>

namespace gideon { namespace zoneserver {


NpcSkillChecker::NpcSkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
    const datatable::SkillEffectTemplate& effectTemplate,
    const datatable::NpcActiveSkillTemplate& skillTemplate) :
    SkillChecker(caster, shouldCheckSafeRegion, effectTemplate),
    skillTemplate_(skillTemplate)
{
}


ErrorCode NpcSkillChecker::canCastable() const
{
    ErrorCode errorCode = SkillChecker::canCastable();
    if (isFailed(errorCode)) {
        return errorCode;
    }

    if (skillTemplate_.castCheckPoint_.checkStatTarget_ == csttCaster) {
        const ErrorCode ec = checkCastableStatState(caster_);
        if (isFailed(ec)) {
            return ec;
        }
    }
    
    return ecOk;
}


bool NpcSkillChecker::checkConcentrationSkill(go::Entity* target, const Position& targetPosition) const
{    
    TargetingType targetingType = skillEffectTemplate_.checkInfo_.targetingType_;
    if (targetingType == ttSelf || targetingType == ttSelfArea) {
        return true;
    }

    float32_t distanceToTargetSq = 0.0f;
    if (targetingType == ttTarget || targetingType == ttTargetArea) {
        const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
            skillTemplate_.skillCastCheckDistanceInfo_.maxDistance_, skillTemplate_.skillCastCheckDistanceInfo_.minDistance_, *target);
        if (isFailed(ecDistance)) {
            return false;
        }
    }
    else if (targetingType == ttArea) {
        const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
            skillTemplate_.skillCastCheckDistanceInfo_.maxDistance_, skillTemplate_.skillCastCheckDistanceInfo_.minDistance_,
            targetPosition);
        if (isFailed(ecDistance)) {
            return false;
        }
    }

    if (targetingType == ttTarget) {
        const ErrorCode ecDirection = caster_.getCastChecker().checkDirection(*target, targetingType,
            skillEffectTemplate_.checkInfo_.targetDirection_, skillEffectTemplate_.checkInfo_.targetOrientation_);
        if (isFailed(ecDirection)) {
            return false;
        }
    }

    return true;
}


const datatable::ActiveSkillTemplate& NpcSkillChecker::getActiveSkillTemplate() const
{
    return skillTemplate_;
}


bool NpcSkillChecker::canCastableState() const
{
    SkillCasterState* state = caster_.querySkillCasterState();
    if (! state) {
        return false;
    }

    CreatureState* creatureState = caster_.queryCreatureState();
    if (! creatureState) {
        return false;
    }

    if (creatureState->isDisableMagic()) {
        if (isMagicSkillType(skillTemplate_.skillType_)) {
            return false;
        }
    }

    return state->canCastSkill(false);
}




ErrorCode NpcSkillChecker::checkCastableStatState(go::Entity& target) const
{    
    go::CastCheckable* castCheckable = target.queryCastCheckable();
    if (! castCheckable) {
        return ecSkillInvalidAvailableTarget;
    }

    return castCheckable->checkCastablePoints(skillTemplate_.castCheckPoint_.pointType_,
        skillTemplate_.castCheckPoint_.isUsePercent_, skillTemplate_.castCheckPoint_.isUpCheckValue_,
        skillTemplate_.castCheckPoint_.checkValue_);
}



}} // namespace gideon { namespace zoneserver {