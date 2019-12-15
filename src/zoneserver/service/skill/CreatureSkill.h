#pragma once

#include "Skill.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <sne/base/concurrent/Future.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon { namespace zoneserver {


namespace go {
class Entity;
} // namespace go {

/**
 * @class CreatureSkill
 *
 * 스킬
 */
class CreatureSkill : public Skill
{
public:
    enum {
        /// 근접 공격일 경우 허용하는 최대 높이(TODO: XML이나 config으로 옮길 것)
        meleeAttackAllowedMaxHeight = 10,
        /// 원거리 공격일 경우 허용하는 최대 높이(TODO: XML이나 config으로 옮길 것)
        rangeAttackAllowedMaxHeight = 20
    };
public:
    CreatureSkill(go::Entity& caster, bool shouldCheckSafeRegion,
        const datatable::SkillEffectTemplate& effectTemplate);
    virtual ~CreatureSkill();

public:
	/// @internal
	void activateConcentrationCasted(go::Entity* target, const Position& targetPosition);
	void activateConcentrationSkill(go::Entity* target, const Position& targetPosition);

private:
	/// 활성화 중인 집중 시킬 상태를 취소한다.
	virtual void cancel();
	virtual void cancelConcentrationSkill(bool isNotify = true);

protected:
    virtual void active(go::Entity* target, const Position& targetPosition);

private:
    void releaseHide();

    virtual bool hasDashEffect() const;

protected:
	sne::base::Future::WeakRef activeConcentrationTask_;

	uint32_t concentrationSkillCount_;

};

}} // namespace gideon { namespace zoneserver {
