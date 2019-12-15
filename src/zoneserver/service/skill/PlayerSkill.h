#pragma once

#include "CreatureSkill.h"
#include "helper/PlayerSkillChecker.h"

namespace gideon { namespace zoneserver {

/**
 * @class CreatureSkill
 *
 * 스킬
 */
class PlayerSkill :
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerSkill>,
    public CreatureSkill
{
public:
    PlayerSkill(go::Entity& caster, bool shouldCheckSafeRegion,
		const datatable::SkillEffectTemplate& skillEffectTemplate,
        const datatable::PlayerActiveSkillTemplate& skillTemplate);
    virtual ~PlayerSkill();

private:
	virtual ErrorCode startCasting(go::Entity* target, const Position& targetPosition,
		float32_t distanceToTargetSq);

    virtual void active(go::Entity* target, const Position& targetPosition);
    
    virtual const SkillChecker& getSkillChecker() const {
        return playerSkillChecker_;
    }
private:
    void changeCharacterState();
    void consumePoints();
    void consumeMaterialItem();
    
private:
    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const;
    virtual const datatable::SkillCastCheckPointInfo& getSkillCastCheckPointInfo() const;
    virtual bool isUsing() const;

private:
	const datatable::PlayerActiveSkillTemplate& skillTemplate_;
    PlayerSkillChecker playerSkillChecker_;
};

}} // namespace gideon { namespace zoneserver {
