#pragma once

#include <gideon/server/data/ServerSkillEffect.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/SkillCastableEquipItemTypeInfo.h>

namespace gideon { namespace datatable {
struct SkillEffectTemplate;
struct PassiveSkillTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace go {

class Creature;

/**
 * @class PassiveSkill
 */
class PassiveSkill
{
public:
    PassiveSkill(Creature& caster, const datatable::SkillEffectTemplate& effectTemplate,
        const datatable::PassiveSkillTemplate& passiveSkillTemplate);
	virtual ~PassiveSkill();

    SkillCode getSkillCode() const;

	bool isSuccessCondition() const;
    bool isCooldown() const;
    bool hasTimeSkill() const;

    void activate();
    void deactivate();

    void changeCondition(PassiveCheckCondition condition);

    const datatable::PassiveSkillTemplate& getPassiveSkillTemplate() const {
        return skillTemplate_;
    }
    const datatable::SkillEffectTemplate& getSkillEffectTemplate() const {
        return effectTemplate_;
    }

protected:
    void setCondition();

protected:
    typedef std::array<bool, pccCount> PassiveCheckConditions;
	PassiveCheckConditions conditions_;
    const datatable::SkillEffectTemplate& effectTemplate_;
    const datatable::PassiveSkillTemplate& skillTemplate_;
    Creature& caster_;
    bool isConditionSuccess_;    
};


}}} // namespace gideon { namespace zoneserver { namespace go {