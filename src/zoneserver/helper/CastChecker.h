#pragma once

#include <gideon/cs/shared/data/EffectInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}}

namespace gideon { namespace zoneserver {

/***
 * @class CastChecker
 ***/
class CastChecker
{
public:
    CastChecker(go::Entity& caster);
    ~CastChecker();

    ErrorCode checkSelfTargeting(go::Entity*& target,
        const GameObjectInfo& targetInfo, TargetingType targetingType) const;

    // 거리 체크
    ErrorCode checkDistance(float32_t& distanceToTargetSq,
        float32_t maxDistance, float32_t minDistance, const Position& targetPosition) const;
    ErrorCode checkDistance(float32_t& distanceToTargetSq,
        float32_t maxDistance, float32_t minDistance, const go::Entity& target) const;

    // 방향 체크
    ErrorCode checkDirection(const go::Entity& target,
        TargetingType targetingType, TargetDirection targetDirection) const; 
    ErrorCode checkDirection(const go::Entity& target,
        TargetingType targetingType, TargetDirection targetDirection,
        TargetOrientation targetOrientation) const;

    
    ErrorCode checkCastComplete(go::Entity* target, TargetingType targetingType,
        float32_t maxDistance, float32_t minDistance, float32_t distanceToTargetSq,
        TargetDirection targetDirection, TargetOrientation targetOrientation) const;
    ErrorCode checkCastComplete(go::Entity* target, TargetingType targetingType,
        float32_t maxDistance, float32_t minDistance, float32_t distanceToTargetSq) const;

    ErrorCode checkSafeRegion(bool isCheckSafeRegion, const Position& position) const;
    
    EffectDefenceType checkTargetDefence(go::Entity& target, SkillType skillType, bool isTargetProgectailCommonMaigic,
        AttributeRateType attributeType) const;
private:
    go::Entity& caster_;
};

}} // namespace gideon { namespace zoneserver {
