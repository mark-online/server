#pragma once

#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/EffectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace datatable {
struct ActiveSkillTemplate;
struct SkillEffectTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

// TODO 1:1 대전 스톱처리

namespace gideon { namespace zoneserver {


class SkillChecker
{
public:
    SkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
        const datatable::SkillEffectTemplate& skillEffectTemplate);
    
    bool shouldCheckByArena() const;  

    virtual ErrorCode canCastable() const;
    virtual bool checkConcentrationSkill(go::Entity* target, const Position& targetPosition) const = 0;

    bool checkDirectTargetingSkillTargetDistance(const go::Entity& target) const;
    
    ErrorCode checkCastComplete(go::Entity* target, const Position& targetPosition,
        float32_t distanceToTargetSq) const;
    ErrorCode checkCooltime() const;
    
    ErrorCode checkDistanceAndDirection(float32_t& distanceToTargetSq,
        go::Entity& target) const;

protected:
    virtual bool canCastableState() const = 0;

    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const = 0;


protected:
    go::Entity& caster_;
    const datatable::SkillEffectTemplate& skillEffectTemplate_;
    bool shouldCheckSafeRegion_;
};

}} // namespace gideon { namespace zoneserver {