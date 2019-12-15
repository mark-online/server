#include "ZoneServerPCH.h"
#include "CreatureSkill.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/StaticObject.h"
#include "../../model/gameobject/ability/Castable.h"
#include "../../model/gameobject/ability/Castnotificationable.h"
#include "../../model/gameobject/ability/Thinkable.h"
#include "../../model/state/CreatureState.h"
#include "../../model/state/SkillCasterState.h"
#include "../../model/state/CombatState.h"
#include "../../controller/callback/SkillCallback.h"
#include "../../controller/PlayerController.h"
#include "../../helper/CastChecker.h"
#include "helper/SkillChecker.h"
#include "../time/GameTimer.h"
#include "../../ai/Brain.h"
#include "../../ai/aggro/AggroList.h"
#include <gideon/cs/shared/data/CastInfo.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/3d/3d.h>
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class CancelSkillCastingEvent
 */
class CancelConcentrationSkillEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CancelConcentrationSkillEvent>
{
public:
    CancelConcentrationSkillEvent(const GameObjectInfo& sourceInfo, SkillCode skillCode) :
        sourceInfo_(sourceInfo),
        skillCode_(skillCode) {}

private:
    virtual void call(go::Entity& entity) {
        gc::SkillCallback* skillCallback = entity.getController().querySkillCallback();
        if (skillCallback != nullptr) {
            skillCallback->activateConcentrationCancelled(sourceInfo_, skillCode_);
        }
    }

private:
    const GameObjectInfo sourceInfo_;
    const SkillCode skillCode_;
};


/**
 * @class CompleteConcentrationSkillEvent
 */
class CompleteConcentrationSkillEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CompleteConcentrationSkillEvent>
{
public:
    CompleteConcentrationSkillEvent(const GameObjectInfo& sourceInfo, SkillCode skillCode) :
        sourceInfo_(sourceInfo),
        skillCode_(skillCode) {}

private:
    virtual void call(go::Entity& entity) {
        gc::SkillCallback* skillCallback = entity.getController().querySkillCallback();
        if (skillCallback != nullptr) {
            skillCallback->activateConcentrationCompleted(sourceInfo_, skillCode_);
        }
    }

private:
    const GameObjectInfo sourceInfo_;
    const SkillCode skillCode_;
};


/**
 * @class ActivateConcentrationSkillEvent
 */
class ActivateConcentrationSkillEvent: public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ActivateConcentrationSkillEvent>
{
public:
    ActivateConcentrationSkillEvent(CreatureSkill& skill, go::Entity* target, const Position& targetPosition) :
        skill_(skill),
        target_(target),
        targetPosition_(targetPosition) {}

private:
    virtual void run() {
        skill_.activateConcentrationCasted(target_, targetPosition_);
    }

private:
    CreatureSkill& skill_;
    go::Entity* target_;
    const Position targetPosition_;
};

/**
 * @class ConcentrationSkillActiveTask
 */
class ConcentrationSkillActiveTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ConcentrationSkillActiveTask>
{
public:
    ConcentrationSkillActiveTask(Skill& skill, go::Entity* target, 
        const Position& targetPosition, EffectDefenceType effectDefenceType) :
        skill_(skill),
        target_(target),
        targetPosition_(targetPosition),
        effectDefenceType_(effectDefenceType) {}

private:
    virtual void run() {
        skill_.casted(target_, targetPosition_, effectDefenceType_);
    }

private:
    Skill& skill_;
    go::Entity* target_;
    const Position targetPosition_;
    EffectDefenceType effectDefenceType_;
};

} // namespace {

// = Skill

CreatureSkill::CreatureSkill(go::Entity& caster, bool shouldCheckSafeRegion,
	const datatable::SkillEffectTemplate& effectTemplate) :
    Skill(caster, shouldCheckSafeRegion, effectTemplate),
    concentrationSkillCount_(0)
{
}


CreatureSkill::~CreatureSkill()
{
}



void CreatureSkill::activateConcentrationCasted(go::Entity* target, const Position& targetPosition)
{
	if (! getSkillChecker().checkConcentrationSkill(target, targetPosition)) {
		cancelConcentrationSkill(true);
		return;
	}


    Position newPosition = targetPosition;
    if (! isAreaSkill(effectTemplate_.checkInfo_.targetingType_)) {
        if (target) {
            newPosition = target->getPosition();
        }
    }

    const datatable::CreatureActiveSkillTemplate& skillTemplate = 
        static_cast<const datatable::CreatureActiveSkillTemplate&>(getActiveSkillTemplate());
    if (skillTemplate.skillUseableState_ != susAlaways) {
        if (caster_.queryCombatState()->isCombating() && skillTemplate.skillUseableState_ == susPeace) {
            cancelConcentrationSkill(true);
            return; 
        }
        else if (! caster_.queryCombatState()->isCombating() && skillTemplate.skillUseableState_ == susCombat) {
            cancelConcentrationSkill(true);
            return; 
        }
        else if (! caster_.queryCreatureState()->isHidden() && skillTemplate.skillUseableState_ == susHidden) {
            cancelConcentrationSkill(true);
            return; 
        }
    }

    EffectDefenceType effectDefenceType = edtNone;
    if (ttTarget == effectTemplate_.checkInfo_.targetingType_ && ! isBuffSkillType(skillTemplate.skillType_) && target) {
        effectDefenceType = caster_.getCastChecker().checkTargetDefence(*target, skillTemplate.skillType_, 
            skillTemplate.isCommonProgectileAttackMagicSkill(), effectTemplate_.attribute_);
    }

	Skill::casted(target, newPosition, effectDefenceType);

	
	if (concentrationSkillCount_ > 0) {
		--concentrationSkillCount_;
	}

	if (concentrationSkillCount_ <= 0) {
		auto event = std::make_shared<CompleteConcentrationSkillEvent>(
            caster_.getGameObjectInfo(), getSkillCode());
		caster_.queryKnowable()->broadcast(event);
		cancelConcentrationSkill(false);
	}
}


void CreatureSkill::activateConcentrationSkill(go::Entity* target, const Position& targetPosition)
{
    const datatable::CreatureActiveSkillTemplate& skillTemplate = 
        static_cast<const datatable::CreatureActiveSkillTemplate&>(getActiveSkillTemplate());

	if (skillTemplate.channelSkillTimeInfo_.channelCastingMilSec_ > 0 && skillTemplate.channelSkillTimeInfo_.channelIntervalMilSec_ > 0) {
		concentrationSkillCount_ = skillTemplate.channelSkillTimeInfo_.channelApplyCount_;
		SkillCasterState* skillCasterState = caster_.querySkillCasterState();
		if (skillCasterState) {        
			skillCasterState->concentrationSkillActivated(getSkillCode(), skillTemplate.canCastOnMoving_);
		}

		go::Castable* castable = caster_.queryCastable();
		if (castable) {
			auto task = std::make_unique<ActivateConcentrationSkillEvent>(*this, target, targetPosition);
			castable->activateConcentrationSkill(std::move(task),
                skillTemplate.channelSkillTimeInfo_.channelIntervalMilSec_);	        
		}
	}            
}


void CreatureSkill::cancel()
{
    Skill::cancel();
	cancelConcentrationSkill();
}


void CreatureSkill::cancelConcentrationSkill(bool isNotify)
{
	SkillCasterState* state = caster_.querySkillCasterState();
	if (state) {
		if (state->isActivateConcentrationSkill()) {
			state->concentrationSkillReleased();
		}
	}

	go::Castable* castable = caster_.queryCastable();
	if (castable) {
		castable->cancelConcentrationSkill();	        
	}

	if (isNotify) {
		auto event = std::make_shared<CancelConcentrationSkillEvent>(
            caster_.getGameObjectInfo(), getSkillCode());
		caster_.queryKnowable()->broadcast(event);
	}
}


void CreatureSkill::active(go::Entity* target, const Position& targetPosition)
{
    releaseHide();

    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    CompleteCastResultInfo castInfo;
    // 1:1 상황에서만 동기화를 위해서 광역스킬이 아니면 미리 디펜스 검사를 한다.
    EffectDefenceType effectDefenceType = edtNone;
    if (ttTarget == effectTemplate_.checkInfo_.targetingType_ && ! isBuffSkillType(skillTemplate.skillType_) && target) {
        effectDefenceType = caster_.getCastChecker().checkTargetDefence(*target, skillTemplate.skillType_, 
            skillTemplate.isCommonProgectileAttackMagicSkill(), effectTemplate_.attribute_);
        go::Thinkable* thinkable = target->queryThinkable();
        if (thinkable) {
            ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
            if (aggroList != nullptr) {
                aggroList->addThreat(caster_, 10, 0);
            }
        }
    }

    if (isTargetCastSkill(effectTemplate_.checkInfo_.targetingType_)) {
		castInfo.set(uctSkillTo, caster_.getGameObjectInfo(), target->getGameObjectInfo(),
			getSkillCode(), effectDefenceType);
	}
	else {
		castInfo.set(uctSkillAt, caster_.getGameObjectInfo(), targetPosition, getSkillCode());
    }

    caster_.queryCastNotificationable()->notifyCompleteCasting(castInfo);

    const CastType castType = skillTemplate.castingInfo_.castType_;
    if (isDirectCastSkill(castType)) {
        Skill::casted(target, targetPosition, effectDefenceType);
    }    
    else if (isProjectileCastSkill(castType)) {
        fireProjectile(target, targetPosition, skillTemplate.projectileSpeed_, effectDefenceType);
    }

	if (isConcentrationSkill(castType)) {
		activateConcentrationSkill(target, targetPosition);
	}
}


void CreatureSkill::releaseHide()
{
    if (caster_.isCreature()) {
        CreatureState* state = caster_.queryCreatureState();
        if (state->isHidden()) {
            gc::EffectCallback* effectCallback = caster_.getController().queryEffectCallback();
            if (effectCallback) {
                effectCallback->cancelEffect(escHide);
            }
        }
    }	
}


bool CreatureSkill::hasDashEffect() const
{
    return effectTemplate_.hasDashEffect();
}
}} // namespace gideon { namespace zoneserver {
