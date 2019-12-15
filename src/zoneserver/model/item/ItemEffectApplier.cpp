#include "ZoneServerPCH.h"
#include "ItemEffectApplier.h"
#include "Inventory.h"
#include "EquipInventory.h"
#include "QuestInventory.h"
#include "../state/CreatureState.h"
#include "../../helper/CastingHelper.h"
#include "../time/CastingTimer.h"
#include "../time/CoolDownTimer.h"
#include "../../service/time/GameTimer.h"
#include "../gameobject/Player.h"
#include <gideon/cs/datatable/QuestItemTable.h>
#include <gideon/server/3d/G3dHelpers.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver {

namespace {


/**
 * @class QuestItemCastingTask
 */
class QuestItemCastingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<QuestItemCastingTask>
{
public:
    QuestItemCastingTask(CastableItem& item, go::Entity* target,
        const datatable::QuestItemTemplate* questItemTemplate,
        const Position& targetPosition, float32_t distanceToTargetSq) :
        item_(item),
        target_(target),
        questItemTemplate_(questItemTemplate),
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
    const datatable::QuestItemTemplate* questItemTemplate_;
    const Position targetPosition_;
    float32_t distanceToTargetSq_;
};


} // namespace

ItemEffectApplier::ItemEffectApplier(go::Player& owner, CastingTimer& castingTimer,
    CoolDownTimer& cooldDownTimer) :
    caster_(owner),
    castingTimer_(castingTimer),
    cooldDownTimer_(cooldDownTimer),
    castingHelper_(new CastingHelper(owner))
{
}


ErrorCode ItemEffectApplier::castToTargetQuestItem(ObjectId itemId, const GameObjectInfo& targetInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    const datatable::QuestItemTemplate* questItemTemplate = nullptr;
    ErrorCode errorCode = checkCastingQuestItem(questItemTemplate, itemId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    
    const datatable::BaseCastInfo& userInfo = questItemTemplate->getQuestItemUseInfo();
    go::Entity* target = nullptr;
    errorCode = castingHelper_->checkSelfTargeting(target, targetInfo, userInfo.targetingType_);
    if (isFailed(errorCode)) {
        return errorCode;
    }    

    if (! target) {
        const ErrorCode ecTarget = castingHelper_->checkTarget(target, targetInfo);
        if (isFailed(ecTarget)) {
            return ecTarget;
        }
    }
    assert(target != nullptr);

    if (userInfo.exceptTarget_ == setSelfExcept && userInfo.targetingType_ != ttSelfArea) {
        if (static_cast<go::Entity&>(caster_).isSame(*target)) {        
            return ecSkillSelfExceptSkill;
        }
    }

    const ObjectPosition posTarget = target->getPosition();
    float32_t distanceToTargetSq = 0.0f;
    const ErrorCode ecDistance = castingHelper_->checkDistance(distanceToTargetSq,
        userInfo.maxDistance_, userInfo.minDistance_, posTarget);
    if (isFailed(ecDistance)) {
        return ecDistance;
    }

    return startCasting(target, questItemTemplate, posTarget, distanceToTargetSq);
}


ErrorCode ItemEffectApplier::castToAreaQuestItem(ObjectId itemId, const Position& targetPosition)
{
    sne::server::Profiler profiler(__FUNCTION__);

    const datatable::QuestItemTemplate* questItemTemplate = nullptr;
    ErrorCode errorCode = checkCastingQuestItem(questItemTemplate, itemId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const datatable::BaseCastInfo& userInfo = questItemTemplate->getQuestItemUseInfo();
    if (! isAreaSkill(userInfo.targetingType_)) {
        return ecSkillNotAvailable;
    }

    UnionEntityInfo entityInfo = caster_.getUnionEntityInfo();

    assert(isValid(userInfo.effectRangeDirection_));

    const ErrorCode ecTarget = castingHelper_->checkTargetPosition(targetPosition);
    if (isFailed(ecTarget)) {
        return ecTarget;
    }

    float32_t distanceToTargetSq = 0.0f;
    const ErrorCode ecDistance = castingHelper_->checkDistance(distanceToTargetSq,
        userInfo.maxDistance_, userInfo.minDistance_, targetPosition);
    if (isFailed(ecDistance)) {
        return ecDistance;
    }

    return startCasting(nullptr, questItemTemplate,
        targetPosition, distanceToTargetSq);
}


ErrorCode ItemEffectApplier::startCasting(go::Entity* target,
    const datatable::QuestItemTemplate* questItemTemplate, const Position& targetPosition,
    float32_t distanceToTargetSq)
{
    const ErrorCode ecSafeRegion = castingHelper_->checkSafeRegion(
        target != nullptr ? target->getPosition() : targetPosition);
    if (isFailed(ecSafeRegion)) {
        return ecSafeRegion;
    }

    const datatable::BaseCastInfo& userInfo = questItemTemplate->getQuestItemUseInfo();

    const GameTime currentTime = GAME_TIMER->msec();
    //lastActiveTime_ = currentTime;
    castingHelper_->setGlobalCooltime(userInfo.useGlobalTimeIndex_, userInfo.globalCooldownTime_);

    if (userInfo.castingTime_> 0) {
        castingHelper_->notifyStartCastingTo(target, userInfo.targetingType_, questItemTemplate->getQuestItemCode());        

        // TODO: 추후 수정
 /*       SkillCasterState* state = caster_.querySkillCasterState();
        if (state) {        
            state->skillCasted(skillCode_, skillCommonInfo_.canCastOnMoving_);
        }*/

        sne::base::RunnablePtr task(
            new QuestItemCastingTask(*this, target, questItemTemplate, targetPosition, distanceToTargetSq));
        castingTask_ = TASK_SCHEDULER->schedule(task, userInfo.castingTime_);
        return ecOk;
    }

    return castingCompleted(target, targetPosition, distanceToTargetSq);
}



ErrorCode ItemEffectApplier::castingCompleted(go::Entity* target, const Position& targetPosition,
    const datatable::QuestItemTemplate* questItemTemplate, float32_t distanceToTargetSq)
{
    // TODO 추후 수정
    //SkillCasterState* state = caster_.querySkillCasterState();
    //if (state) {        
    //    state->skillCastReleased();
    //}
    const datatable::BaseCastInfo& userInfo = questItemTemplate->getQuestItemUseInfo();

    const ErrorCode errorCode = castingHelper_->checkTargetAfterCompleteCasting(target, 
        userInfo.targetingType_, userInfo.maxDistance_, userInfo.minDistance_, distanceToTargetSq);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    //if (caster_.isCreature()) {
    //    CreatureState* state = caster_.queryCreatureState();
    //    if (state->isHidden()) {
    //        gc::SkillEffectCallback* skillEffectCallback =
    //            caster_.getController().queryEffectInfoCallback();
    //        skillEffectCallback->cancelSkillEffect(sgHide);
    //    }
    //}

    // TODO: 아이템 삭제

    castingHelper_->notifyCastingComplete(target, userInfo.targetingType_, targetPosition,
        questItemTemplate->getQuestItemCode());   

    casted(target, targetPosition);    

    return ecOk;
}


void ItemEffectApplier::casted(go::Entity* target, const Position& targetPosition,
    const datatable::QuestItemTemplate* questItemTemplate)
{
    sne::server::Profiler profiler(__FUNCTION__);
    const datatable::BaseCastInfo& userInfo = questItemTemplate->getQuestItemUseInfo();

    TargetingType targetingType = userInfo.targetingType_;
    if (target != nullptr) { // TODO: target의 유효성 문제
        if (! target->isValid()) {
            return;
        }

        if (! isAreaEffectSkill(targetingType) && ! isDirectCastSkill(userInfo.castType_)) {
            float32_t distanceToTargetSq = 0.0f;
            
            if (isFailed(castingHelper_->checkDistance(distanceToTargetSq, userInfo.maxDistance_,
                userInfo.minDistance_, target->getPosition()))) {
                SNE_LOG_INFO3("skill casted failed distance!! ObjectInfo(%u, %u)", 
                    target->getObjectId(), target->getObjectType());
                return;
            }
        }
    }

    if (isToIndividualEffectSkill(targetingType)) {		
        individualEffected(*target, targetPosition);
    }
    else if (isAreaEffectSkill(targetingType)) {
        areaEffected(targetPosition);
    }
    else {
        assert(false);
    }    
}


void ItemEffectApplier::individualEffected(go::Entity& target, const Position& centerPosition)
{
    // TODO: target 유효성 문제
    gc::SkillEffectCallback* skillEffectCallback =
        target.getController().queryEffectInfoCallback();
    if (! skillEffectCallback) {
        SNE_LOG_ERROR3("skill not find skill effect callback!! ObjectInfo(%u, %u)", 
            target.getObjectId(), target.getObjectType());
        return;
    }

    if (! caster_.isSame(target) && skillTemplate_.isTargetsAttackSkill()) {
        go::Chaoable* casterChaoable = caster_.queryChaoable();
        if (casterChaoable) {
            go::Chaoable* targetChaoable = target.queryChaoable();
            if (targetChaoable) {
                if (! casterChaoable->isRealChao() && ! targetChaoable->isChao()) {
                    casterChaoable->hitToPlayer();
                }
            }
        }
    }
    else if (! caster_.isSame(target) && skillTemplate_.isGraveStoneSkill()) {
        go::Chaoable* casterChaoable = caster_.queryChaoable();
        if (casterChaoable) {
            go::Chaoable* targetChaoable = target.queryChaoable();
            if (targetChaoable) {
                if (! targetChaoable->isChao()) {
                    casterChaoable->standGraveStone();
                }
            }
        }
    }

    SkillEffectResult skillEffectResult(skillCode_);
    const go::GameStatsable* statsable = target.queryGameStatsable();        
    if (statsable && ! ttSelf == skillCommonInfo_.targetingType_) {
        const GameStats& tagetGameStats = statsable->getGameStats();
        const AttributeRate deffenceRate = isAttackSkillType(skillCommonInfo_.skillType_) ? 
            tagetGameStats.attributeRates_[ariBlock] : 
        tagetGameStats.attributeRates_[ariAvoid];
        skillEffectResult.isDefenced_ = isSuccessRate(deffenceRate);
    }

    if (! skillEffectResult.isDefenced_) {
        applyEffectInfo(skillEffectResult, target, isCasterEffect);  
    }
    else {
        SNE_LOG_INFO3("skill defence succcess!! ObjectInfo(%u, %u)", 
            target.getObjectId(), target.getObjectType());
    }

    // 넉백 처리
    if (! isCasterEffect && skillEffectResult.isValid() && skillTemplate_.hasKnockbackEffect()) {        
        CreatureState* state = target.queryCreatureState();
        if (! state->isknockback()) {
            for (int i = eanStart; i < eanCount; ++i) {
                calcKnockback(skillEffectResult, target, centerPosition, static_cast<EffectApplyNum>(i));
            }
        }
    }

    skillEffectCallback->applyEffectInfo(caster_, skillEffectResult);
}


void ItemEffectApplier::cancelCastingUseItem()
{
    sne::base::Future::Ref castingTask = castingTask_.lock();
    if (! castingTask.get()) {
        return;
    }

    if (castingTask->isCancelled() || castingTask->isDone()) {
        return;
    }

    castingTask->cancel();
    castingTask_.reset();
}



ErrorCode ItemEffectApplier::checkCastingQuestItem(const datatable::QuestItemTemplate*& questItemTemplate,
    ObjectId questItemId) const
{
    QuestItemInfo questItemInfo = caster_.getQuestInventory().getItemInfo(questItemId);
    if (! questItemInfo.isValid()) {
        return ecInventoryItemNotFound;
    }
    
    questItemTemplate = QUEST_ITEM_TABLE->getQuestItemTemplate(questItemInfo.questCode_);
    if (! questItemTemplate) {
        return ecItemNotQuestItem;
    }

    return ecOk;
}

}} // namespace gideon { namespace zoneserver {
