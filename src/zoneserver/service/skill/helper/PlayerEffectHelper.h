#pragma once

#include "CreatureEffectHelper.h"
#include <gideon/cs/shared/data/EffectStatusInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class CreatureSkillEffectHelper
 *
 * 스킬
 */
class PlayerEffectHelper : public CreatureEffectHelper
{
public:
    PlayerEffectHelper(go::Entity& caster);
    ~PlayerEffectHelper();

private:
    virtual void applyTargetEffect_i(SkillEffectResult& skillEffectResult, SkillType skillType, 
        const EffectValues& effectValues, go::Entity& target, 
        const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType defenceType);

    virtual ErrorCode canTargetEffected(SkillType skillType, bool isCheckSafeRegion, EffectLevel effectLevel,
        const datatable::SkillCastCheckPointInfo& checkPointInfo, const datatable::SkillEffectTemplate& effectTemplate, 
        go::Entity& target) const;

    virtual bool isAvailableTarget(go::Entity& target, AvailableTarget availableTarget) const;
    virtual bool isExceptTarget(go::Entity& target, ExceptTarget exceptTarget) const;

private:
    ErrorCode checkTargetEffectable(SkillType skillType, const datatable::SkillEffectTemplate& effectTemplate,
        bool isCheckSafeRegion, go::Entity& target) const;
    ErrorCode checkAttackOrDebufEffectable(SkillType skillType, 
        TargetingType targetingType, go::Entity& target) const;
    
    bool checkProtectionCharacter(go::Entity& target) const;

private:
    int32_t getValues(EffectStatusType type) const;

private:
    void updateCaotic(SkillType skillType, const datatable::SkillEffectTemplate& effectTemplate, go::Entity& target);

};

}} // namespace gideon { namespace zoneserver {
