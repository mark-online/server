#include "ZoneServerPCH.h"
#include "CastChecker.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/Npc.h"
#include "../model/gameobject/AbstractAnchor.h"
#include "../model/state/CreatureState.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/ability/ItemCastable.h"
#include "../model/gameobject/ability/StaticObjectOwerable.h"
#include "../model/gameobject/ability/Partyable.h"
#include "../model/gameobject/ability/CastGameTimeable.h"
#include "../model/gameobject/ability/Protectionable.h"
#include "../model/gameobject/ability/Guildable.h"
#include "../model/gameobject/ability/ArenaMatchable.h"
#include "../model/gameobject/ability/WeaponUseable.h"
#include "../model/state/FieldDuelState.h"
#include "../model//state/CastState.h"
#include "../service/party/Party.h"
#include "../service/party/PartyService.h"
#include "../service/time/GameTimer.h"
#include "../controller/EntityController.h"
#include "../controller/callback/CastCallback.h"
#include "../world/WorldMap.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/shared/data/CombatRatingInfo.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon { namespace zoneserver {

namespace {

inline msec_t getProjectileDuration(float distanceToTargetSq, uint8_t projectileSpeed)
{
    const float32_t projectileSpeedSq = (float32_t(projectileSpeed) * projectileSpeed);
    return msec_t((distanceToTargetSq / projectileSpeedSq) * 1000.0f);
}


} // namespace {

CastChecker::CastChecker(go::Entity& caster) :
    caster_(caster)
{
}
 

CastChecker::~CastChecker()
{
}


ErrorCode CastChecker::checkSelfTargeting(go::Entity*& target,
    const GameObjectInfo& targetInfo, TargetingType targetingType) const
{
    if (targetingType == ttSelf) {
        if (! (caster_.isSame(targetInfo))) {
            return ecSkillNotAvailable;
        }
        target = &caster_;
    }
    else if (targetingType == ttSelfArea) {
        if (! (caster_.isSame(targetInfo))) {
            return ecSkillNotAvailable;
        }
        target = &caster_;
    }
    else if (targetingType== ttTargetArea) {

    }
    return ecOk;
}


ErrorCode CastChecker::checkDistance(float32_t& distanceToTargetSq,
    float32_t maxDistance, float32_t minDistance, const Position& targetPosition) const
{
    // FYI: 플레이어만 거리를 검사한다
    if (! caster_.isPlayer()) {
        return ecOk;
    }

    const float32_t maxDistanceSq = maxDistance * maxDistance;
    distanceToTargetSq = caster_.getSquaredLength(targetPosition);
    if (maxDistanceSq > 0) {
        if (distanceToTargetSq > maxDistanceSq) {
            return ecSkillTargetTooFar;
        }
    }

    // TODO: 높이 검사
    //const float32_t height = getRelativeHeight(caster_Position, targetPosition);
    const float32_t minDistanceSq = minDistance * minDistance;
    if (minDistanceSq > 0) {
        if (distanceToTargetSq < minDistanceSq) {
            return ecSkillTargetTooNear;
        }
        //if (height > rangeAttackAllowedMaxHeight) {
        //    return ecSkillTargetTooFar;
        //}
    }
    else {
        //if (height > meleeAttackAllowedMaxHeight) {
        //    return ecSkillTargetTooFar;
        //}
    }

    return ecOk;
}


ErrorCode CastChecker::checkDistance(float32_t& distanceToTargetSq,
    float32_t maxDistance, float32_t minDistance, const go::Entity& target) const
{
    distanceToTargetSq = caster_.getSquaredLength(target.getPosition());
    if (maxDistance > 0) {
        maxDistance = maxDistance + target.getModelingRadiusSize();
        const float32_t maxDistanceSq = (maxDistance * maxDistance);
        
        if (distanceToTargetSq > maxDistanceSq) {
            return ecSkillTargetTooFar;
        }
    }

    // TODO: 높이 검사
    //const float32_t height = getRelativeHeight(caster_Position, targetPosition);

    if (minDistance > 0) {
        const float32_t minDistanceSq = minDistance * minDistance;
        if (distanceToTargetSq < minDistanceSq) {
            return ecSkillTargetTooNear;
        }
        //if (height > rangeAttackAllowedMaxHeight) {
        //    return ecSkillTargetTooFar;
        //}
    }
    else {
        //if (height > meleeAttackAllowedMaxHeight) {
        //    return ecSkillTargetTooFar;
        //}
    }

    return ecOk;
}


ErrorCode CastChecker::checkDirection(const go::Entity& target,
    TargetingType targetingType, TargetDirection targetDirection) const
{
    if (targetingType != ttTarget) {
        return ecOk;
    }

    ErrorCode errorCode = ecOk;
    const ObjectPosition poscaster_ = caster_.getPosition();
    const ObjectPosition posTarget = target.getPosition();
    const Vector2 hcaster_ = getDirection(poscaster_.heading_);
    if (shouldCheckTargetDirection(targetDirection)) {
        const Vector2 dirTarget = getDirection(posTarget, poscaster_);
        const bool isForward = isInForward(dirTarget, hcaster_);
        if (targetDirection == tdFront) {
            if (! isForward) {
                errorCode = ecSkillInvalidTargetDirection;// 뒤에 있네
            }
        }
        else { //if (targetDirection_ == tdRear) {
            if (isForward) {
                errorCode = ecSkillInvalidTargetDirection;// 앞에 있네
            }
        }
    }
    return errorCode;
}


ErrorCode CastChecker::checkDirection(const go::Entity& target,
    TargetingType targetingType, TargetDirection targetDirection,
    TargetOrientation targetOrientation) const
{
    // FYI: 플레이어만 방향을 검사한다
    if (! caster_.isPlayer()) {
        return ecOk;
    }

    if ((! target.isCreature()) || (targetingType != ttTarget)) {
        return ecOk;
    }

    if ((! shouldCheckTargetDirection(targetDirection)) &&
        (! shouldCheckTargetOrientation(targetOrientation))) {
        return ecOk;
    }

    ErrorCode errorCode = ecOk;
    const ObjectPosition poscaster_ = caster_.getPosition();
    const ObjectPosition posTarget = target.getPosition();
    const Vector2 hcaster_ = getDirection(poscaster_.heading_);
    if (shouldCheckTargetDirection(targetDirection)) {
        const Vector2 dirTarget = getDirection(posTarget, poscaster_);
        const bool isForward = isInForward(dirTarget, hcaster_);
        if (targetDirection == tdFront) {
            if (! isForward) {
                 errorCode = ecSkillInvalidTargetDirection;// 뒤에 있네
            }
        }
        else { //if (targetDirection_ == tdRear) {
            if (isForward) {
                errorCode = ecSkillInvalidTargetDirection;// 앞에 있네
            }
        }
    }

    if (isSucceeded(errorCode) && shouldCheckTargetOrientation(targetOrientation)) {
        const bool isForward = isInForward(hcaster_, posTarget.heading_);
        if (toFront == targetOrientation) {
            if (isForward) {
                errorCode = ecSkillInvalidTargetOrientation;// 상대방이 뒤를 바라보고있다
            }
        }
        else { //if (toRear == targetOrientation_) {
            if (! isForward) {
                errorCode = ecSkillInvalidTargetOrientation;// 상대방이 자신을 바라보고 있다.
            }
        }
    }

    return errorCode;
}


ErrorCode CastChecker::checkCastComplete(go::Entity* target, TargetingType targetingType,
    float32_t maxDistance, float32_t minDistance, float32_t distanceToTargetSq,
    TargetDirection targetDirection, TargetOrientation targetOrientation) const
{
    if (! target) {
        return ecOk;
    }
    const ErrorCode errorCode = checkCastComplete(target, targetingType,
        maxDistance, minDistance, distanceToTargetSq);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    
    return checkDirection(*target, targetingType, targetDirection, targetOrientation);
}


ErrorCode CastChecker::checkCastComplete(go::Entity* target, TargetingType targetingType,
    float32_t maxDistance, float32_t minDistance, float32_t distanceToTargetSq) const
{    
    if (! target) {
        return ecOk;
    }

    if (! target->isValid()) {            
        return ecSkillTargetNotFound;
    }

    const ObjectPosition posTarget = target->getPosition();
    const ErrorCode ecDistance = checkDistance(distanceToTargetSq, maxDistance, minDistance, *target);
    if (isFailed(ecDistance)) {
        return ecDistance;
    }

    if (targetingType == ttTarget) {
        CreatureState* state = target->queryCreatureState();
        if (state && state->isHidden() && ! target->isSame(caster_.getGameObjectInfo())) {
            return ecSkillInvalidAvailableTarget;
        }
    }               

    return ecOk;
}


ErrorCode CastChecker::checkSafeRegion(bool isCheckSafeRegion, const Position& position) const
{
    if (! isCheckSafeRegion) {
        return ecOk;
    }

    const WorldMap* worldMap = caster_.getCurrentWorldMap();
    if (! worldMap) {
        return ecWorldMapNotEntered;
    }

    if (worldMap->isInSafeRegion(position)) {
        return ecSkillCannotCastInSafeRegion;
    }

    return ecOk;
}


EffectDefenceType CastChecker::checkTargetDefence(go::Entity& target, SkillType skillType, bool isTargetProgectailCommonMaigic,
    AttributeRateType attributeType) const
{
    if (caster_.isCreature() && target.isCreature()) {
        const bips_t levelBonus = caster_.getLevelBonus(target);
        const bips_t roll = esut::random(0, 10000);
        bips_t sumChance = 0;

        // 근접/원거리 물리 공격 스킬 or 발사체 대상 공통 마법스킬
        if (isPhysicalAttack(skillType) || isTargetProgectailCommonMaigic) {
            const bips_t missChance = caster_.getMissChance(target);
            sumChance += clampBips(missChance + levelBonus);
            if (sumChance >= roll) {
                return edtMissed;
            }
        }

        if (target.queryCreatureState()->isCrowdControl()) {
            return edtNone;
        }

        // 플레이어만 방향을 검사한다
        const bool isFrontTarget = (! target.isPlayer()) || isSucceeded(checkDirection(target, ttTarget, tdFront, toFront));

        // 근접/원거리 물리 공격 스킬 or 발사체 대상 공통 마법스킬
        if (isPhysicalAttack(skillType) || isTargetProgectailCommonMaigic) {
            const bips_t dodgeChance = target.getDodgeChance();
            sumChance += clampBips(dodgeChance + levelBonus);
            if (sumChance >= roll) {
                return edtDodged;
            }

            if (isFrontTarget) {
                const bips_t blockChance = target.getBlockChance();
                if (blockChance > 0) {
                    sumChance += clampBips(blockChance + levelBonus);
                    if (sumChance >= roll) {
                        return edtBlocked;
                    }
                }
            }

            // 근접 물리 공격 스킬
            if (sktAttackMelee == skillType) {
                if (isFrontTarget) {
                    const bips_t parryChance = target.getParryChance();
                    if (parryChance > 0) {
                        sumChance += clampBips(parryChance + levelBonus);
                        if (sumChance >= roll) {
                            return edtParried;
                        }
                    }
                }
            }
        }

        const bips_t resistChance = target.getResistChance(attributeType);
        sumChance += resistChance;
        if (sumChance >= roll) {
            return edtResisted;
        }
    }

    return edtNone;
}

}} // namespace gideon { namespace zoneserver {