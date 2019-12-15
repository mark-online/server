#pragma once

#include "SkillChecker.h"

namespace gideon { namespace datatable {
struct PlayerActiveSkillTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {


class PlayerSkillChecker : public SkillChecker
{
public:
    PlayerSkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
        const datatable::SkillEffectTemplate& effectTemplate,
        const datatable::PlayerActiveSkillTemplate& skillTemplate);

    virtual ErrorCode canCastable() const;
    virtual bool checkConcentrationSkill(go::Entity* target, const Position& targetPosition) const;


protected:
    // SkillChecker overriding;    
    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const;

    virtual bool canCastableState() const;


private:
    ErrorCode checkCastableCurrentState() const;
    ErrorCode checkCasterPointStats() const;

    ErrorCode checkCastableStatState(go::Entity& target) const;
    
    bool checkProtectionCharacter(go::Entity& target) const;
    bool hasConsumeMaterialItem() const;

private:
    bool isAreaAttackOrDebuffSkill() const;
    bool isAttackSkill() const;
    bool isDebuffSkill() const;
    
private:
    const datatable::PlayerActiveSkillTemplate& skillTemplate_;
};

}} // namespace gideon { namespace zoneserver {