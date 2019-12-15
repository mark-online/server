#pragma once

#include "EffectHelper.h"

namespace gideon { namespace zoneserver {

/**
 * @class CreatureEffectHelper
 *
 * 스킬
 */
class CreatureEffectHelper : public EffectHelper
{
public:
    CreatureEffectHelper(go::Entity& caster);
    ~CreatureEffectHelper();

protected:
    virtual void applyTargetEffect_i(SkillEffectResult& skillEffectResult, SkillType skillType, 
        const EffectValues& effectValues, go::Entity& target, 
        const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType effectDefenceType);

};

}} // namespace gideon { namespace zoneserver {
