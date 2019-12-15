#include "ZoneServerPCH.h"
#include "CastableItem.h"
#include "../state/CreatureState.h"
#include "../time/CastingTimer.h"
#include "../time/CoolDownTimer.h"
#include "../state/CastState.h"
#include "../gameobject/Entity.h"
#include "../gameobject/Npc.h"
#include "../gameobject/ability/Castable.h"
#include "../gameobject/ability/CastGameTimeable.h"
#include "../gameobject/ability/Castnotificationable.h"
#include "../gameobject/ability/Protectionable.h"
#include "../gameobject/ability/ArenaMatchable.h"
#include "../gameobject/ability/Partyable.h"
#include "../gameobject/ability/Guildable.h"
#include "../gameobject/ability/CombatStateable.h"
#include "../state/FieldDuelState.h"
#include "../gameobject/ability/impl/KnowableQueryFilter.h"
#include "../../helper/CastChecker.h"
#include "../../service/time/GameTimer.h"
#include "../../service/party/Party.h"
#include "../../service/skill/helper/EffectHelper.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/EffectCallback.h"
#include "../../controller/callback/EntityStatusCallback.h"
#include <gideon/cs/shared/data/CastInfo.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/3d/3d.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class ItemCastingTask
 */
class ItemCastingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ItemCastingTask>
{
public:
    ItemCastingTask(CastableItem& item, go::Entity* target,
        const Position& targetPosition, float32_t distanceToTargetSq) :
        item_(item),
        target_(target),
        targetPosition_(targetPosition),
        distanceToTargetSq_(distanceToTargetSq) {}

private:
    virtual void run() {
        const ErrorCode errorCode = item_.castingCompleted(target_,
            targetPosition_, distanceToTargetSq_);
        if (isFailed(errorCode)) {
            // TODO: 에러 리턴??
        }
    }

private:
    CastableItem& item_;
    go::Entity* target_;
    const Position targetPosition_;
    float32_t distanceToTargetSq_;
};


} // namespace



CastableItem::CastableItem(go::Entity& caster, ObjectId itemId, DataCode itemCode,
    const datatable::ItemActiveSkillTemplate& itemSkillTemplate,
    const datatable::SkillEffectTemplate& effectTemplate) :
    caster_(caster),
    itemId_(itemId),
    itemCode_(itemCode),
    itemSkillTemplate_(itemSkillTemplate),
    effectTemplate_(effectTemplate)
{
}


CastableItem::~CastableItem()
{
}



ErrorCode CastableItem::castTo(const GameObjectInfo& targetInfo)
{
    go::Entity* target = nullptr;
    ErrorCode errorCode = getTarget(target, targetInfo);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    float32_t distanceToTargetSq = 0.0f;
    if (! caster_.isSame(*target)) {
        const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
            itemSkillTemplate_.skillCastCheckDistanceInfo_.maxDistance_, 
            itemSkillTemplate_.skillCastCheckDistanceInfo_.minDistance_, target->getPosition());
        if (isFailed(ecDistance)) {
            return ecDistance;
        }
         
        const ErrorCode ecTarget = caster_.getEffectHelper().canTargetEffected(itemSkillTemplate_.skillType_, 
            false, itemSkillTemplate_.effectLevel_, datatable::SkillCastCheckPointInfo(), effectTemplate_, *target);
        if (isFailed(ecTarget)) {
            return ecTarget;
        }
    }
    assert(target != nullptr);

    if (effectTemplate_.checkInfo_.exceptTarget_ == etSelf && effectTemplate_.checkInfo_.targetingType_ != ttSelfArea) {
        if (static_cast<go::Entity&>(caster_).isSame(*target)) {        
            return ecSkillSelfExceptSkill;
        }
    }

    const ErrorCode scriptEc = checkItemScriptUsable();
    if (isFailed(scriptEc)) {
        return scriptEc;
    }

    return startCasting(target, target->getPosition(), distanceToTargetSq);
}


ErrorCode CastableItem::castAt(const Position& targetPosition)
{
    sne::server::Profiler profiler(__FUNCTION__);
    
    if (! isAreaSkill(effectTemplate_.checkInfo_.targetingType_)) {
        return ecSkillNotAvailable;
    }

    float32_t distanceToTargetSq = 0.0f;
    const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
        itemSkillTemplate_.skillCastCheckDistanceInfo_.maxDistance_, 
        itemSkillTemplate_.skillCastCheckDistanceInfo_.minDistance_, targetPosition);
    if (isFailed(ecDistance)) {
        return ecDistance;
    }

    return startCasting(nullptr, targetPosition, distanceToTargetSq);
}


ErrorCode CastableItem::startCasting(go::Entity* target, const Position& targetPosition,
    float32_t distanceToTargetSq)
{
    target, targetPosition, distanceToTargetSq;
	const ErrorCode coolEc = checkCooltime();
	if (isFailed(coolEc)) {
		return coolEc;
	}

    const ErrorCode errorCode = checkItem();
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Castable* castable = caster_.queryCastable();
    go::CastNotificationable* notificationable = caster_.queryCastNotificationable();
	if (! castable && ! notificationable) {
        return ecServerInternalError;
    }

    setCooldown();

    if (! itemSkillTemplate_.castingInfo_.hasCastingTime()) {
        return castingCompleted(target, targetPosition, distanceToTargetSq);
    }

    auto task = std::make_unique<ItemCastingTask>(*this, target, targetPosition, distanceToTargetSq);
    castable->startCasting(itemCode_, itemSkillTemplate_.canCastOnMoving_, std::move(task), itemSkillTemplate_.castingInfo_.castTime_);

    const GameObjectInfo targetInfo  = 
        target == nullptr ? GameObjectInfo() : target->getGameObjectInfo();
    StartCastResultInfo startCastInfo;
    startCastInfo.set(uctItemTo, caster_.getGameObjectInfo(), targetInfo, itemCode_);
    notificationable->notifyStartCasting(startCastInfo);

    return ecOk;
}


ErrorCode CastableItem::castingCompleted(go::Entity* target, Position targetPosition,
    float32_t distanceToTargetSq)
{
    distanceToTargetSq;
    go::Castable* castable = caster_.queryCastable();
	go::CastNotificationable* notificationable = caster_.queryCastNotificationable();
    if (! castable && ! notificationable) {
        return ecServerInternalError;
    }
    if (effectTemplate_.getTargetingType() == ttTargetArea) {
        if (target) {
            targetPosition = target->getPosition();
        }
    }
    castable->releaseCastState(false);
    TargetingType targetingType = effectTemplate_.checkInfo_.targetingType_;
    const UnionCastType uct = isAreaEffectSkill(targetingType) ? uctSkillAt : uctItemTo;
    const ErrorCode errorCode = caster_.getCastChecker().checkCastComplete(target, 
        targetingType, itemSkillTemplate_.skillCastCheckDistanceInfo_.maxDistance_, 
        itemSkillTemplate_.skillCastCheckDistanceInfo_.minDistance_, distanceToTargetSq);
    if (isFailed(errorCode)) {
        FailCompletedCastResultInfo failCastInfo;
        failCastInfo.setDataInfo(uct, errorCode, caster_.getGameObjectInfo(), itemCode_);
        notificationable->notifyCompleteFailedCasting(failCastInfo);
    
        return errorCode;
    }

    releaseHide();

    CompleteCastResultInfo completeCastInfo;
    EffectDefenceType effectDefenceType = edtNone;
    if (ttTarget == effectTemplate_.checkInfo_.targetingType_ && ! isBuffSkillType(itemSkillTemplate_.skillType_) && target) {
        effectDefenceType = caster_.getCastChecker().checkTargetDefence(*target, itemSkillTemplate_.skillType_, 
            itemSkillTemplate_.isCommonProgectileAttackMagicSkill(), effectTemplate_.attribute_);
    }

    if (isAreaEffectSkill(targetingType)) {
        completeCastInfo.set(uct, caster_.getGameObjectInfo(), targetPosition, getItemCode());
    }
    else if (isToIndividualEffectSkill(targetingType)) {
        completeCastInfo.set(uct, caster_.getGameObjectInfo(), target->getGameObjectInfo(),
            getItemCode(), effectDefenceType);
    }
    notificationable->notifyCompleteCasting(completeCastInfo);

    SNE_LOG_INFO("GameLog CastableItem::castingCompleted(PID:%" PRIu64 ", itemCode:%u",
		caster_.getObjectId(), itemCode_);

    casted(target, targetPosition, effectDefenceType);    

    return ecOk;
}


void CastableItem::cancel()
{
}


void CastableItem::casted(go::Entity* target, const Position& targetPosition, EffectDefenceType effectDefenceType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    TargetingType targetingType = effectTemplate_.checkInfo_.targetingType_;;
    if (target != nullptr) { // TODO: target의 유효성 문제
        if (! target->isValid()) {
            return;
        }

        if (! isAreaEffectSkill(targetingType) && ! isDirectCastSkill(itemSkillTemplate_.castingInfo_.castType_)) {
            float32_t distanceToTargetSq = 0.0f;

            if (isFailed(caster_.getCastChecker().checkDistance(distanceToTargetSq, itemSkillTemplate_.skillCastCheckDistanceInfo_.maxDistance_, 
                itemSkillTemplate_.skillCastCheckDistanceInfo_.minDistance_, *target))) {
                SNE_LOG_INFO("skill casted failed distance!! ObjectInfo(%" PRIu64 ", %u)", 
                    target->getObjectId(), target->getObjectType());
                return;
            }
        }
    }

    caster_.getEffectHelper().applyCasterEffect(itemSkillTemplate_.skillCode_, 
        itemSkillTemplate_.skillType_, itemSkillTemplate_.casterEffectValues_, 
        target == nullptr ? caster_ : *target, effectTemplate_);

    if (isToIndividualEffectSkill(targetingType)) {
        if (shouldApplyEffect(effectDefenceType)) {
            caster_.getEffectHelper().applyTargetEffect(itemSkillTemplate_.skillCode_, itemSkillTemplate_.skillType_, 
                itemSkillTemplate_.casterEffectValues_, *target, effectTemplate_, effectDefenceType);
        }  
        if (isAttackSkillType(itemSkillTemplate_.skillType_)) {
            if (target && target->isPlayer() && target->queryCombatStateable()->updateAttackTime()) {
                gc::EntityStatusCallback* callback = target->getController().queryEntityStatusCallback();
                if (callback) {
                    callback->playerCombatStateChanged(true);
                }
            }		
        }
    }
    else if (isAreaEffectSkill(targetingType)) {
        caster_.getEffectHelper().applyAreaTargetEffect(itemSkillTemplate_.skillCode_, itemSkillTemplate_.skillType_, itemSkillTemplate_.effectLevel_,
            false, itemSkillTemplate_.casteeEffectValues_, datatable::SkillCastCheckPointInfo(), targetPosition, effectTemplate_);
    }

    useItem();
}


void CastableItem::setCooldown()
{
    go::CastGameTimeable* castable = caster_.queryCastGameTimeable();
    if (castable) {
        castable->setCooldown(itemCode_, itemSkillTemplate_.coolTimeInfo_.cooldownMilSec_,
            itemSkillTemplate_.coolTimeInfo_.globalCooldownTimeIndex_, 
            itemSkillTemplate_.coolTimeInfo_.globalCooldownMilSec_);
    }
}


void CastableItem::cancelCooldown()
{
    go::CastGameTimeable* castable = caster_.queryCastGameTimeable();
    if (castable) {
        castable->cancelCooldown(itemCode_, itemSkillTemplate_.coolTimeInfo_.globalCooldownTimeIndex_);
    }
}


void CastableItem::releaseHide()
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


ErrorCode CastableItem::checkCooltime() const
{
	go::CastGameTimeable* castable = caster_.queryCastGameTimeable();
	if (! castable) {
		return ecServerInternalError;
	}
	if (itemSkillTemplate_.coolTimeInfo_.hasGlobalCooldown()) {
		if (castable->isGlobalCooldown(itemSkillTemplate_.coolTimeInfo_.globalCooldownTimeIndex_)) {
			return ecSkillTooShortInterval;
		}
	}

	if (castable->isLocalCooldown(itemCode_)) {
		return ecSkillTooShortInterval;
	}

	return ecOk;
}


ErrorCode CastableItem::getTarget(go::Entity*& target, const GameObjectInfo& targetInfo) const
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

    if (caster_.isSame(targetInfo)) {
        return ecSkillNotAvailable;
    }

    target = caster_.queryKnowable()->getEntity(targetInfo);
    if (! target) {
        return ecSkillTargetNotFound;
    }

    return ecOk;    
}



}} // namespace gideon { namespace zoneserver {
