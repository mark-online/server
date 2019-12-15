#pragma once

#include <gideon/cs/shared/data/EffectInfo.h>
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace datatable {
struct SkillEffectTemplate;
struct SkillCastCheckPointInfo;
struct EffectInfo;
}} // namespace gideon { namespace datatable {

namespace gideon {
struct SkillEffectResult;
} // namespace gideon { 

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {


int32_t getCalcPercentStatusValue(const CreatureStatusInfo& statusInfo, const datatable::EffectInfo& effectInfo, int32_t value);

/**
 * @class EffectHelper
 *
 * 스킬
 */
class EffectHelper
{
public:
    EffectHelper(go::Entity& caster) :
        caster_(caster)
    {
    }
        
    ErrorCode applyAreaTargetEffect(DataCode dataCode, SkillType skillType, EffectLevel effectLevel, bool isCheckSafeRegion,
        const EffectValues& effectValues, const datatable::SkillCastCheckPointInfo& checkPointInfo, 
        const Position& targetPosition, const datatable::SkillEffectTemplate& effectTemplate);
    ErrorCode applyTargetEffect(DataCode dataCode, SkillType skillType, const EffectValues& effectValues, go::Entity& target, 
        const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType effectDefenceType);
    ErrorCode applyCasterEffect(DataCode dataCode, SkillType skillType, const EffectValues& effectValues, 
        go::Entity& target, const datatable::SkillEffectTemplate& effectTemplate);

    void applyCasterPassiveEffect(SkillCode skillCode, SkillType skillType, const EffectValues& effectValues, 
        const datatable::SkillEffectTemplate& effectTemplate);
    void releaseCasterPassiveEffect(SkillCode skillCode);

public:
    virtual ErrorCode canTargetEffected(SkillType skillType, bool isCheckSafeRegion, EffectLevel effectLevel,
        const datatable::SkillCastCheckPointInfo& checkPointInfo, const datatable::SkillEffectTemplate& effectTemplate, 
        go::Entity& target) const;

protected:
    virtual bool isAvailableTarget(go::Entity& target, AvailableTarget availableTarget) const = 0;
    virtual bool isExceptTarget(go::Entity& target, ExceptTarget exceptTarget) const = 0;

protected:
    go::Entity& getCaster() {
        return caster_;
    }
    const go::Entity& getCaster() const {
        return caster_;
    }

    template <class T>
    T& getCasterAs() {
        return static_cast<T&>(caster_);
    }

    template <class T>
    const T& getCasterAs() const {
        return static_cast<const T&>(caster_);
    }

protected:
    virtual void applyTargetEffect_i(SkillEffectResult& skillEffectResult, SkillType skillType, 
        const EffectValues& effectValues, go::Entity& target, 
        const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType effectDefenceType) = 0;

protected:
    ErrorCode checkCastableTargetStatus(const datatable::SkillCastCheckPointInfo& checkPointInfo, go::Entity& target) const;    

private:
    go::Entity& caster_;
};


/**
 * @class SkillEffectHelper
 *
 * 스킬
 */
class EntityEffectHelper :public EffectHelper
{
public:
    EntityEffectHelper(go::Entity& caster) :
        EffectHelper(caster)
    {
    }

protected:
    virtual void applyTargetEffect_i(SkillEffectResult& /*skillEffectResult*/, 
        SkillType /*skillType*/, const EffectValues& /*effectValues*/, 
        go::Entity& /*target*/, const datatable::SkillEffectTemplate& /*effectTemplate*/, EffectDefenceType /*type*/) {

    }
    virtual bool isAvailableTarget(go::Entity& /*target*/, AvailableTarget /*availableTarget*/) const {
        return false;
    }

    virtual bool isExceptTarget(go::Entity& /*target*/, ExceptTarget /*exceptTarget*/) const {
        return false;
    }

};
}} // namespace gideon { namespace zoneserver {
