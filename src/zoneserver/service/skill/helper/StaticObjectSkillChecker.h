#pragma once

#include "SkillChecker.h"


namespace gideon { namespace datatable {
struct SOActiveSkillTemplate;
}} // namespace gideon { namespace datatable {


namespace gideon { namespace zoneserver {


class StaticObjectSkillChecker : public SkillChecker
{
public:
    StaticObjectSkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
        const datatable::SkillEffectTemplate& effectTemplate, const datatable::SOActiveSkillTemplate& skillTemplate);
    
    virtual ErrorCode canCastable() const;
    virtual bool checkConcentrationSkill(go::Entity* target, const Position& targetPosition) const;

protected:
    virtual bool canCastableState() const;
    
    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const;    

private:
    ErrorCode checkCasterPointStats() const;

protected:
    const datatable::SOActiveSkillTemplate& skillTemplate_;
    
};

}} // namespace gideon { namespace zoneserver {