#pragma once

#include "../../zoneserver_export.h"
#include "../../helper/CastEventCallback.h"
#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/skillInfo.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/EffectInfo.h>
#include <gideon/cs/shared/data/SkillCastableEquipItemTypeInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/datatable/EffectApplyInfo.h>
#include <sne/base/concurrent/Future.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon {
struct SkillEffectResult;    
} // namespace gideon {

namespace gideon { namespace datatable {
struct SkillEffectTemplate;    
struct ActiveSkillTemplate;
struct SkillCastCheckPointInfo;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

class SkillChecker;

namespace go {
class Entity;
} // namespace go {

/**
 * @class Skill
 *
 * 스킬
 */
class ZoneServer_Export Skill
{
public:
    Skill(go::Entity& caster, bool shouldCheckSafeRegion, 
        const datatable::SkillEffectTemplate& effectTemplate);
    virtual ~Skill();

public:
    ErrorCode castTo(const GameObjectInfo& targetInfo);
    ErrorCode castAt(const Position& targetPosition);
    ErrorCode castingCompleted(go::Entity* target, Position targetPosition,
        float32_t distanceToTargetSq);

    void casted(go::Entity* target, const Position& targetPosition, EffectDefenceType effectDefenceType);


    float32_t getMaxDistance() const;    
    bool canCast() const;
    bool isUsing() const;

public:
    // TODO 수정
    virtual void cancel();
    virtual void cancelConcentrationSkill(bool isNotify = true) { isNotify; }

protected:
    virtual ErrorCode startCasting(go::Entity* target, const Position& targetPosition,
        float32_t distanceToTargetSq);
    
    void fireProjectile(go::Entity* target, const Position& targetPosition,
        float32_t projectileSpeed, EffectDefenceType effectDefenceType);

protected:
    virtual void active(go::Entity* target, const Position& targetPosition) = 0;
    
protected:
    ErrorCode getTarget(go::Entity*& target, const GameObjectInfo& targetInfo) const;
    ErrorCode checkTargetPosition(const Position& targetPosition) const;

protected:
    virtual void setCooldown();
    virtual void cancelCooldown();

protected:
    SkillCode getSkillCode() const;

    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const = 0;
    virtual const datatable::SkillCastCheckPointInfo& getSkillCastCheckPointInfo() const = 0;
    virtual const SkillChecker& getSkillChecker() const = 0;
    virtual bool hasDashEffect() const = 0;

protected:
    bool hasCastingTime() const;
    bool isDeffenceSucceeded(go::Entity& target) const;

protected:
    bool shouldCheckSafeRegion_;	
    const datatable::SkillEffectTemplate& effectTemplate_;
    sne::base::Future::WeakRef projectailTask_;

	go::Entity& caster_;
};

}} // namespace gideon { namespace zoneserver {
