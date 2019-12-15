#pragma once

#include "SkillChecker.h"

namespace gideon { namespace datatable {
struct NpcActiveSkillTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {


class NpcSkillChecker : public SkillChecker
{
public:
    NpcSkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
        const datatable::SkillEffectTemplate& effectTemplate,
        const datatable::NpcActiveSkillTemplate& skillTemplate);
    
    virtual ErrorCode canCastable() const;
    virtual bool checkConcentrationSkill(go::Entity* target, const Position& targetPosition) const;

protected:
    // SkillChecker overriding;
    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const;
    
    virtual bool canCastableState() const;
    
private:
    ErrorCode checkCastableStatState(go::Entity& target) const;

protected:
    const datatable::NpcActiveSkillTemplate& skillTemplate_;
    
};

}} // namespace gideon { namespace zoneserver {