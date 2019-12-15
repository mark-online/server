#pragma once

#include "Skill.h"
#include "helper/StaticObjectSkillChecker.h"
#include <gideon/cs/datatable/SoActiveSkillTable.h>
#include <sne/base/concurrent/Future.h>


namespace gideon { namespace datatable {
struct SOActiveSkillTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

/**
 * @class CreatureSkill
 *
 * 스킬
 */
class StaticObjectSkill :
    public Skill
{
public:
    StaticObjectSkill(go::Entity& caster, bool shouldCheckSafeRegion, 
        const datatable::SkillEffectTemplate& skillEffectTempate, 
        const datatable::SOActiveSkillTemplate& skillTemplate);
    virtual ~StaticObjectSkill();

protected:
    virtual void active(go::Entity* target, const Position& targetPosition);
    
private:
    void consumePoints();
    void consumeMaterialItem();

protected:    
    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const;
    virtual const datatable::SkillCastCheckPointInfo& getSkillCastCheckPointInfo() const;

    virtual const SkillChecker& getSkillChecker() const {
        return skillChecker_;
    }

    virtual bool hasDashEffect() const {
        return false;
    }
 
private:
	const datatable::SOActiveSkillTemplate& skillTemplate_;
    StaticObjectSkillChecker skillChecker_;
};

}} // namespace gideon { namespace zoneserver {
