#include "ZoneServerPCH.h"
#include "Skill.h"
#include "helper/SkillChecker.h"
#include "helper/EffectHelper.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/StaticObject.h"
#include "../../model/gameobject/ability/Castable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/CreatureStatusable.h"
#include "../../model/gameobject/ability/CastGameTimeable.h"
#include "../../model/gameobject/ability/Castnotificationable.h"
#include "../../model/gameobject/ability/Moveable.h"
#include "../../model/gameobject/ability/FieldDuelable.h"
#include "../../model/gameobject/ability/Skillable.h"
#include "../../model/gameobject/ability/CombatStateable.h"
#include "../../model/state/FieldDuelState.h"
#include "../../model/state/SkillCasterState.h"
#include "../../model/state/CastState.h"
#include "../../model/state/CombatState.h"
#include "../../model/state/CreatureState.h"
#include "../../model/gameobject/ability/impl/KnowableQueryFilter.h"
#include "../../world/WorldMap.h"
#include "../../world/World.h"
#include "../../helper/CastChecker.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/CastCallback.h"
#include "../../controller/callback/EntityStatusCallback.h"
#include <gideon/cs/datatable/WorldMapTable.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/shared/data/Rate.h>
#include <gideon/3d/3d.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver {

namespace {

inline bool isBegginerMap()
{
    const gdt::map_t* mapTemplate = WORLDMAP_TABLE->getMap(WORLD->getWorldMapCode());
    assert(mapTemplate != nullptr);
    if (wtBeginner == WorldType(mapTemplate->world_type())) {
        return true;
    }
    return false;
}

inline msec_t getProjectileDuration(float32_t distanceToTargetSq, float32_t projectileSpeed)
{
    const float32_t projectileSpeedSq = (float32_t(projectileSpeed) * projectileSpeed);
    return msec_t((distanceToTargetSq / projectileSpeedSq) * 1000.0f);
}


/**
 * @class SkillCastingTask
 */
class SkillCastingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillCastingTask>
{
public:
    SkillCastingTask(Skill& skill, go::Entity* target, const Position& targetPosition,
        float32_t distanceToTargetSq) :
        skill_(skill),
        target_(target),
        targetPosition_(targetPosition),
        distanceToTargetSq_(distanceToTargetSq) {}

private:
    virtual void run() {
        const ErrorCode errorCode = skill_.castingCompleted(target_, 
            targetPosition_, distanceToTargetSq_);
        if (isFailed(errorCode)) {
            // TODO: 에러 리턴??
        }
    }

private:
    Skill& skill_;
    go::Entity* target_;
    const Position targetPosition_;
    float32_t distanceToTargetSq_;
};


/**
 * @class FireProjectileTask
 */
class FireProjectileTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<FireProjectileTask>
{
public:
    FireProjectileTask(Skill& skill, go::Entity* target,
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


} //


Skill::Skill(go::Entity& caster, bool shouldCheckSafeRegion,
    const datatable::SkillEffectTemplate& effectTemplate) :
    shouldCheckSafeRegion_(shouldCheckSafeRegion),
    caster_(caster),
    effectTemplate_(effectTemplate)
{
}


Skill::~Skill()
{
}


ErrorCode Skill::castTo(const GameObjectInfo& targetInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);    
    
    if (! isTargetingSkill(effectTemplate_.checkInfo_.targetingType_)) {
        return ecSkillNotAvailable;
    }

    const ErrorCode ecGeneral = getSkillChecker().canCastable();
    if (isFailed(ecGeneral)) {
        return ecGeneral;
    }

    
    go::Entity* target = nullptr;
    ErrorCode errorCode = getTarget(target, targetInfo);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    assert(target != nullptr);
    float32_t distanceToTargetSq = 0.0f;
    if (! caster_.isSame(*target)) {
        const ErrorCode ecDistance = getSkillChecker().checkDistanceAndDirection(distanceToTargetSq, *target);
        if (isFailed(ecDistance)) {
            return ecDistance;
        }
    }

    if (effectTemplate_.checkInfo_.targetingType_ != ttSelfArea) {
        const SkillType skillType = getActiveSkillTemplate().skillType_;  
        const EffectLevel effectLevel = getActiveSkillTemplate().effectLevel_;    
        const ErrorCode ecTarget = caster_.getEffectHelper().canTargetEffected(skillType, shouldCheckSafeRegion_, 
            effectLevel, getSkillCastCheckPointInfo(), effectTemplate_, *target);
        if (isFailed(ecTarget)) {
            return ecTarget;
        }
    }
    
    return startCasting(target, target->getPosition(), distanceToTargetSq);
}


ErrorCode Skill::castAt(const Position& targetPosition)
{
    sne::server::Profiler profiler(__FUNCTION__);

    const ErrorCode ecGeneral = getSkillChecker().canCastable();
    if (isFailed(ecGeneral)) {
        return ecGeneral;
    }

    TargetingType targetingType = effectTemplate_.checkInfo_.targetingType_;
    if (! isAreaSkill(targetingType)) {
        return ecSkillNotAvailable;
    }

    const ErrorCode ecTarget = checkTargetPosition(targetPosition);
    if (isFailed(ecTarget)) {
        return ecTarget;
    }

    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    float32_t distanceToTargetSq = 0.0f;
    const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
        skillTemplate.skillCastCheckDistanceInfo_.maxDistance_, 
        skillTemplate.skillCastCheckDistanceInfo_.minDistance_, targetPosition);
    if (isFailed(ecDistance)) {
        return ecDistance;
    }

    return startCasting(nullptr, targetPosition, distanceToTargetSq);
}


ErrorCode Skill::castingCompleted(go::Entity* target, Position targetPosition,
    float32_t distanceToTargetSq)
{
    go::Castable* castable = caster_.queryCastable();
    if (castable != nullptr) {
		castable->releaseCastState(false);
    }
    const CreatureState* targetState = target ? target->queryCreatureState() : nullptr;
    if (effectTemplate_.getTargetingType() == ttTargetArea) {
        if (target) {
            targetPosition = target->getPosition();
        }
    }
    ErrorCode errorCode = ecOk;   
    if ((target != nullptr) && (! target->isValid())) {
        errorCode = ecSkillTargetNotFound;
    }
    else if (targetState && targetState->isDied()) {
        if ((! effectTemplate_.hasExecutionEffect()) && (! effectTemplate_.hasReviveEffect())) {
            // 집행이나 부활 외에 스킬은 죽은 애한테 쓸 수 없다
            errorCode =  ecSkillCannotUseToDeadTarget;
        }
    }
    else {
        errorCode = getSkillChecker().checkCastComplete(target, targetPosition, distanceToTargetSq);
    }

    if (isSucceeded(errorCode)) {
        active(target, targetPosition);
    }
    else {
		go::CastNotificationable* notificationable = caster_.queryCastNotificationable();
		if (notificationable) {
			const UnionCastType uct = isTargetCastSkill(effectTemplate_.checkInfo_.targetingType_) ? 
				uctSkillAt : uctSkillTo;
			FailCompletedCastResultInfo castInfo;
			castInfo.setDataInfo(uct, errorCode, caster_.getGameObjectInfo(), getSkillCode());
			notificationable->notifyCompleteFailedCasting(castInfo);       
		}
		cancelCooldown();
    }

    return errorCode;
}


void Skill::casted(go::Entity* target, const Position& targetPosition, EffectDefenceType effectDefenceType)
{
    sne::server::Profiler profiler(__FUNCTION__);
    TargetingType targetingType = effectTemplate_.checkInfo_.targetingType_;
    if (target != nullptr) { 
        // TODO: target의 유효성 문제
        if (! target->isValid()) {
            return;
        }
        if (! getSkillChecker().checkDirectTargetingSkillTargetDistance(*target)) {
			cancelCooldown();
            return;
        }            
    }

    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    caster_.getEffectHelper().applyCasterEffect(getSkillCode(), skillTemplate.skillType_, skillTemplate.casterEffectValues_, 
        target == nullptr ? caster_ : *target, effectTemplate_);

    if (isToIndividualEffectSkill(targetingType)) {
        if (shouldApplyEffect(effectDefenceType)) {
            caster_.getEffectHelper().applyTargetEffect(getSkillCode(), skillTemplate.skillType_, 
                skillTemplate.casteeEffectValues_, *target, effectTemplate_, effectDefenceType);
        }
        if (isAttackSkillType(skillTemplate.skillType_)) {
            if (target && target->isPlayer() && target->queryCombatStateable()->updateAttackTime()) {
                gc::EntityStatusCallback* callback = target->getController().queryEntityStatusCallback();
                if (callback) {
                    callback->playerCombatStateChanged(true);
                }
            }		
        }
    }
    else if (isAreaEffectSkill(targetingType)) {
        caster_.getEffectHelper().applyAreaTargetEffect(getSkillCode(), getActiveSkillTemplate().skillType_, getActiveSkillTemplate().effectLevel_,
            shouldCheckSafeRegion_, getActiveSkillTemplate().casteeEffectValues_, getSkillCastCheckPointInfo(), targetPosition, effectTemplate_);
    }
    else {
        assert(false);
    }
}


float32_t Skill::getMaxDistance() const 
{
    return getActiveSkillTemplate().skillCastCheckDistanceInfo_.maxDistance_;
}


bool Skill::canCast() const
{
    return isSucceeded(getSkillChecker().checkCooltime());
}


bool Skill::isUsing() const
{
    sne::base::Future::Ref projectailTask = projectailTask_.lock();
    if (! projectailTask.get()) {
        return false;
    }

    return (! projectailTask->isCancelled()) && (! projectailTask->isDone());
}


ErrorCode Skill::startCasting(go::Entity* target, const Position& targetPosition,
    float32_t distanceToTargetSq)
{
    setCooldown();

    if (! hasCastingTime()) {
        return castingCompleted(target, targetPosition, distanceToTargetSq);
    }
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    go::Castable* castable = caster_.queryCastable();
    if (castable) {
        GameTime castingTime = skillTemplate.castingInfo_.castTime_;
        go::Skillable* skillable = caster_.querySkillable();
        if (skillable) {
            castingTime = skillable->getCalcCastingTime(castingTime);
        }

		auto task = std::make_unique<SkillCastingTask>(*this,
            target, targetPosition, distanceToTargetSq);
		castable->startCasting(getSkillCode(), skillTemplate.canCastOnMoving_,
            std::move(task), castingTime);
    }

	go::CastNotificationable* notificationable = caster_.queryCastNotificationable();
	if (notificationable) {
		const GameObjectInfo targetInfo  = 
			target == nullptr ? GameObjectInfo() : target->getGameObjectInfo();
		StartCastResultInfo startCastInfo;
		startCastInfo.set(uctSkillTo, caster_.getGameObjectInfo(), targetInfo, getSkillCode());

		notificationable->notifyStartCasting(startCastInfo);
	}

    return ecOk;
}


void Skill::cancel()
{
    sne::base::Future::Ref projectailTask = projectailTask_.lock();
    if (projectailTask.get() != nullptr) {
        projectailTask->cancel();
        projectailTask_.reset();
    }
}


void Skill::fireProjectile(go::Entity* target, const Position& targetPosition,
    float32_t projectileSpeed, EffectDefenceType effectDefenceType)
{
	const float32_t distanceToTargetSq = (target != nullptr) ?
		caster_.getSquaredLength(target->getPosition()) : caster_.getSquaredLength(targetPosition);
    const msec_t duration = getProjectileDuration(distanceToTargetSq, projectileSpeed);

    projectailTask_ = TASK_SCHEDULER->schedule(
        std::make_unique<FireProjectileTask>(*this, target, targetPosition, effectDefenceType),
        duration);
}


ErrorCode Skill::getTarget(go::Entity*& target, const GameObjectInfo& targetInfo) const
{
    target = nullptr;
    TargetingType targetingType = effectTemplate_.checkInfo_.targetingType_;
    if (targetingType == ttSelf || targetingType == ttSelfArea) {
        target = &caster_;
        return ecOk;
    }

    if (targetingType== ttArea) {
        return ecServerInternalError;
    }

    if (effectTemplate_.checkInfo_.exceptTarget_ == etSelf && caster_.isSame(targetInfo)) {
        return ecSkillInvalidAvailableTarget;
    }
    
    target = caster_.queryKnowable()->getEntity(targetInfo);
    if (! target) {
        return ecSkillTargetNotFound;
    }

    return ecOk;    
}


ErrorCode Skill::checkTargetPosition(const Position& targetPosition) const
{
    // TODO: 현재는 에어리어 스킬만 가능
    assert(effectTemplate_.checkInfo_.targetingType_ == ttArea);

    WorldMap* worldMap = caster_.getCurrentWorldMap();
    if (! worldMap) {
        return ecSkillTargetNotFound;
    }

    if (! worldMap->isInMap(targetPosition)) {
        return ecSkillTargetTooFar;
    }

    return ecOk;
}


void Skill::setCooldown()
{
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    go::CastGameTimeable* castable = caster_.queryCastGameTimeable();
    if (castable) {
        const datatable::SkillCoolTimeInfo& cooltimeInfo = skillTemplate.coolTimeInfo_;
        castable->setCooldown(getSkillCode(), cooltimeInfo.cooldownMilSec_, 
            cooltimeInfo.globalCooldownTimeIndex_, cooltimeInfo.globalCooldownMilSec_);
    }
}


void Skill::cancelCooldown()
{
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    go::CastGameTimeable* castable = caster_.queryCastGameTimeable();
    if (castable) {
        castable->cancelCooldown(getSkillCode(), skillTemplate.coolTimeInfo_.globalCooldownTimeIndex_);
    }
}


SkillCode Skill::getSkillCode() const 
{
    return getActiveSkillTemplate().skillCode_;
}


bool Skill::hasCastingTime() const 
{
    const datatable::ActiveSkillTemplate& skillTemplate = getActiveSkillTemplate();
    return skillTemplate.castingInfo_.castTime_ > 0;
}



bool Skill::isDeffenceSucceeded(go::Entity& target) const
{
    target;
    if (hasDashEffect()) {
        return false;
    }
    return false;//isSuccessDeffence(target, getSkillType());
}

}} // namespace gideon { namespace zoneserver {
