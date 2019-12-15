#pragma once

#include "EffectHelper.h"

namespace gideon { namespace zoneserver { namespace go {
class Npc;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

/**
 * @class SoEffectHelper
 * 스킬
 */
class SoEffectHelper : public EffectHelper
{
public:
    SoEffectHelper(go::Entity& entity);
    ~SoEffectHelper();

private:
    virtual void applyTargetEffect_i(SkillEffectResult& skillEffectResult, SkillType skillType, 
        const EffectValues& effectValues, go::Entity& target, 
        const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType defenceType);

    virtual bool isAvailableTarget(go::Entity& target, AvailableTarget availableTarget) const;
    virtual bool isExceptTarget(go::Entity& target, ExceptTarget exceptTarget) const;

private:
    bool isMyPartyMember(go::Entity& target) const;
    bool isMyGuildMember(go::Entity& target) const;

};

}} // namespace gideon { namespace zoneserver {
