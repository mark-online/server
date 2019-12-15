#pragma once

#include "CreatureEffectHelper.h"

namespace gideon { namespace zoneserver {

/**
 * @class CreatureSkillEffectHelper
 *
 * 스킬
 */
class NpcEffectHelper : public CreatureEffectHelper
{
public:
    NpcEffectHelper(go::Entity& caster);
    ~NpcEffectHelper();

private:
    virtual ErrorCode canTargetEffected(SkillType skillType, bool isCheckSafeRegion, EffectLevel effectLevel,
        const datatable::SkillCastCheckPointInfo& checkPointInfo, const datatable::SkillEffectTemplate& effectTemplate, 
        go::Entity& target) const;

private:
    virtual bool isAvailableTarget(go::Entity& target, AvailableTarget availableTarget) const;
    virtual bool isExceptTarget(go::Entity& target, ExceptTarget exceptTarget) const;

protected:
    ErrorCode checkTargetEffectable(SkillType skillType, go::Entity& target) const;


};

}} // namespace gideon { namespace zoneserver {
