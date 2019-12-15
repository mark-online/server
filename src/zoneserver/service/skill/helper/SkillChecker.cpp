#include "ZoneServerPCH.h"
#include "SkillChecker.h"
#include "../../../helper/CastChecker.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/StaticObject.h"
#include "../../../model/gameobject/ability/Moveable.h"
#include "../../../model/gameobject/ability/CastGameTimeable.h"
#include "../../../model/gameobject/ability/ArenaMatchable.h"
#include "../../../world/WorldMap.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/EffectApplyInfo.h>

namespace gideon { namespace zoneserver {


SkillChecker::SkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
    const datatable::SkillEffectTemplate& skillEffectTemplate) :
    caster_(caster),
    shouldCheckSafeRegion_(shouldCheckSafeRegion),
    skillEffectTemplate_(skillEffectTemplate)
{
}
    

bool SkillChecker::shouldCheckByArena() const
{
    go::ArenaMatchable* matchable = caster_.queryArenaMatchable();
    if (! matchable) {
        return false;
    }

    return matchable->getArena() != nullptr;
}


ErrorCode SkillChecker::canCastable() const
{
    if (! canCastableState()) {
        return ecStateCannotSkillCastState;
    }

    const ErrorCode ecSafeRegion =
        caster_.getCastChecker().checkSafeRegion(shouldCheckSafeRegion_, caster_.getPosition());
    if (isFailed(ecSafeRegion)) {
        return ecSafeRegion;
    }   

    go::Knowable* knowable = caster_.queryKnowable();
    assert(knowable != nullptr);
    if (! knowable) {
        return ecServerInternalError;
    }

    const ErrorCode ecCooltime = checkCooltime();
    if (isFailed(ecCooltime)) {
        return ecCooltime;
    }   
    
    go::Moveable* moveable = caster_.queryMoveable();
    if (moveable) {
        const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
        if (! skillTemplate.canCastOnMoving_) {        
            if(moveable->isMoving()) {
                return ecSkillCannotMoveSkill;
            }
        }
    }
    return ecOk;
}


bool SkillChecker::checkDirectTargetingSkillTargetDistance(const go::Entity& target) const
{    
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    if (! isAreaEffectSkill(skillEffectTemplate_.checkInfo_.targetingType_) && 
        ! isDirectCastSkill(skillTemplate.castingInfo_.castType_)) {
        float32_t distanceToTargetSq = 0.0f;
        if (isFailed(caster_.getCastChecker().checkDistance(distanceToTargetSq, 
            skillTemplate.skillCastCheckDistanceInfo_.maxDistance_,
            skillTemplate.skillCastCheckDistanceInfo_.minDistance_, target.getPosition()))) {
                SNE_LOG_INFO("skill casted failed distance!! ObjectInfo(%" PRIu64 ", %u)", 
                    target.getObjectId(), target.getObjectType());
                return false;
        }
    }
    return true;
}


ErrorCode SkillChecker::checkCastComplete(go::Entity* target, const Position& targetPosition,
    float32_t distanceToTargetSq) const
{    
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    if (! skillTemplate.castingInfo_.hasCastingTime()) {
        return ecOk;
    }

    TargetingType targetingType = skillEffectTemplate_.checkInfo_.targetingType_;
    if (isAreaEffectSkill(targetingType) && ! isDirectCastSkill(skillTemplate.castingInfo_.castType_)) {
        // TODO: 코드 검사해봐야 함
        //distanceToTargetSq = 0.0f;
        const ErrorCode errorCode = caster_.getCastChecker().checkDistance(distanceToTargetSq, 
            skillTemplate.skillCastCheckDistanceInfo_.maxDistance_, 
            skillTemplate.skillCastCheckDistanceInfo_.minDistance_, targetPosition);
        if (isFailed(errorCode)) {
            return errorCode;
        }
        return ecOk;
    }

    return caster_.getCastChecker().checkCastComplete(target, 
        targetingType, skillTemplate.skillCastCheckDistanceInfo_.maxDistance_, 
        skillTemplate.skillCastCheckDistanceInfo_.minDistance_, distanceToTargetSq, 
        skillEffectTemplate_.checkInfo_.targetDirection_, 
        skillEffectTemplate_.checkInfo_.targetOrientation_);
}


ErrorCode SkillChecker::checkCooltime() const
{   
    go::CastGameTimeable* castable = caster_.queryCastGameTimeable();
    if (! castable) {
        return ecServerInternalError;
    }
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    if (skillTemplate.coolTimeInfo_.hasGlobalCooldown()) {
        if (castable->isGlobalCooldown(skillTemplate.coolTimeInfo_.globalCooldownTimeIndex_)) {
            return ecSkillTooShortInterval;
        }
    }

    if (castable->isLocalCooldown(skillTemplate.skillCode_)) {
        return ecSkillTooShortInterval;
    }

    return ecOk;
}


ErrorCode SkillChecker::checkDistanceAndDirection(float32_t& distanceToTargetSq,
    go::Entity& target) const
{
    const ObjectPosition posTarget = target.getPosition();
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
        skillTemplate.skillCastCheckDistanceInfo_.maxDistance_, 
        skillTemplate.skillCastCheckDistanceInfo_.minDistance_, target);
    if (isFailed(ecDistance)) {
        return ecDistance;
    }

    const ErrorCode ecDirection = caster_.getCastChecker().checkDirection(target,
        skillEffectTemplate_.checkInfo_.targetingType_,
        skillEffectTemplate_.checkInfo_.targetDirection_, skillEffectTemplate_.checkInfo_.targetOrientation_);
    
    return ecDirection;
}



}} // namespace gideon { namespace zoneserver {
