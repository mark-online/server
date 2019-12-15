#include "ZoneServerPCH.h"
#include "EffectHelper.h"
#include "../../../model/gameobject/ability/impl/KnowableQueryFilter.h"
#include "../../../model/gameobject/ability/CombatStateable.h"
#include "../../../model/gameobject/skilleffect/EffectScriptApplier.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/Creature.h"
#include "../../../model/gameobject/StaticObject.h"
#include "../../../model/gameobject/ability/CastCheckable.h"
#include "../../../model/state/CreatureState.h"
#include "../../../model/gameobject/ability/Thinkable.h"
#include "../../../model/gameobject/status/StaticObjectStatus.h"
#include "../../../ai/Brain.h"
#include "../../../ai/aggro/AggroList.h"
#include "../../../helper/CastChecker.h"
#include "../../../helper/SkillTableHelper.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/EffectCallback.h"
#include "../../../controller/callback/PassiveEffectCallback.h"
#include "../../../controller/callback/EntityStatusCallback.h"
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/SkillCommonInfo.h>

namespace gideon { namespace zoneserver {

namespace {


bool applyKnockback(Position& position, uint32_t distance,
    const go::Entity& caster, const go::Entity& target)
{ 
    const float32_t knockbackDistance = distance / 100.0f;
    const Position targetPosition = target.getPosition();
    const Vector2 direction = getDirection(targetPosition, caster.getPosition());

    position = targetPosition;
    position.x_ += direction.x * knockbackDistance;
    position.y_ += direction.y * knockbackDistance;
    return true;
}


bool applyDash(Position& position,
    go::Entity& caster, const Position& centerPosition)
{
    const float32_t dashIntervalDistance = 1.0f;
    const Vector2 direction = getDirection(centerPosition, caster.getPosition());

    position = centerPosition;
    position.x_ -= direction.x * dashIntervalDistance;
    position.y_ -= direction.y * dashIntervalDistance;
    position.z_ = caster.getPosition().z_;

    return true;
}

}

int32_t getCalcPercentStatusValue(const CreatureStatusInfo& statusInfo, const datatable::EffectInfo& effectInfo,
    int32_t value)
{
    if (esttHp == effectInfo.statusType_ && effectInfo.isPercent_) {
        value = (statusInfo.points_.hp_ * value) / 1000;
    }
    else if (esttMp == effectInfo.statusType_ && effectInfo.isPercent_) {
        value = (statusInfo.points_.mp_ * value) / 1000;
    }
    else if (isAttackPowerStatusType(effectInfo.statusType_) && effectInfo.isPercent_) {
        value = (statusInfo.attackPowers_[getAttackPowerIndex(effectInfo.statusType_)] * value) / 1000;
    }
    else if (isBaseStatusType(effectInfo.statusType_) && effectInfo.isPercent_) {
        value = (statusInfo.baseStatus_[getBaseStatusIndex(effectInfo.statusType_)] * value) / 1000;
    }
    else if (isAttackStatusType(effectInfo.statusType_) && effectInfo.isPercent_) {
        value = (statusInfo.attackStatus_[getAttackStatusIndex(effectInfo.statusType_)] * value) / 1000;
    }
    else if (isDefenceStatusType(effectInfo.statusType_) && effectInfo.isPercent_) {
        value = (statusInfo.defenceStatus_[getDefenceStatusIndex(effectInfo.statusType_)] * value) / 1000;
    }
    else if (isAttributeRateType(effectInfo.statusType_) && effectInfo.isPercent_) {
        if (isAttributeRateResist(effectInfo.statusType_)) {
            value = (statusInfo.attributeResists_[getDefenceStatusIndex(effectInfo.statusType_)] * value) / 1000;
        }
        else {
            value = (statusInfo.attributes_[getDefenceStatusIndex(effectInfo.statusType_)] * value) / 1000;
        }
    }
    return value;
}



ErrorCode EffectHelper::applyAreaTargetEffect(DataCode dataCode, SkillType skillType, EffectLevel effectLevel,
    bool isCheckSafeRegion, const EffectValues& effectValues, const datatable::SkillCastCheckPointInfo& checkPointInfo, 
    const Position& center, const datatable::SkillEffectTemplate& effectTemplate)
{
    assert(isAreaEffectSkill(effectTemplate.checkInfo_.targetingType_));

    WorldMap* worldMap = getCaster().getCurrentWorldMap();
    if (! worldMap) {
        return ecServerInternalError;
    }

    const go::EffectInfoableEntityQueryFilter queryFilter(caster_, effectTemplate.checkInfo_);
    go::Knowable* knowable = caster_.queryKnowable();
    if (! knowable) {
        return ecServerInternalError;
    }

    const GameObjects targets =
        knowable->getKnownEntitiesInArea(center, effectTemplate.checkInfo_.effectDistance_, &queryFilter);
    for (const GameObjectInfo& goi : targets) {
        go::Entity* target = knowable->getEntity(goi);
        if (! target) {
            continue;
        }

        if (isFailed(canTargetEffected(skillType, isCheckSafeRegion, effectLevel, checkPointInfo, effectTemplate, *target))) {
            continue;
        }

        if (isAttackSkillType(skillType)) {
            if (target && target->isPlayer() && target->queryCombatStateable()->updateAttackTime()) {
                gc::EntityStatusCallback* callback = target->getController().queryEntityStatusCallback();
                if (callback) {
                    callback->playerCombatStateChanged(true);
                }
            }		
        }

        (void) applyTargetEffect(dataCode, skillType, effectValues, *target, effectTemplate, edtNone);
    }
    return ecOk;
}


ErrorCode EffectHelper::applyTargetEffect(DataCode dataCode, SkillType skillType, 
    const EffectValues& effectValues, go::Entity& target, 
    const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType effectDefenceType)
{ 
    gc::EffectCallback* effectCallback = target.getController().queryEffectCallback();
    if (! effectCallback) {
        return ecSkillInvalidAvailableTarget;
    }

    if (! effectTemplate.hasEffects(false)) {
        return ecOk;
    }

    caster_.getController().queryEffectCallback()->hitEffect(target, dataCode);
    
    SkillEffectResult skillEffectResult(dataCode);
    applyTargetEffect_i(skillEffectResult, skillType, effectValues, target, effectTemplate, effectDefenceType);

    if (effectTemplate.hasKnockbackEffect()) {
        CreatureState* state = target.queryCreatureState();
        if (! state) {
            return ecSkillInvalidAvailableTarget;
        }
        if (! state->isKnockback()) {
            for (int i = eanStart; i < eanCount; ++i) {
                if (isKnockbackScript(effectTemplate.casteeEffects_[i].script_) && ! skillEffectResult.effects_[i].isImmune_) {
                    applyKnockback(skillEffectResult.effects_[i].position_, effectValues[i], caster_, target);
                    break;
                }                
            }
        }
    }

    CreatureState* creatureState = target.queryCreatureState();
    if ((creatureState != nullptr) && creatureState->isInvincible()) {
        if (isAttackOrDebuffSkillType(skillType)) {
            skillEffectResult.effects_[eanEffect_1].value_ = 0;
            skillEffectResult.effects_[eanEffect_2].value_ = 0;
        }
    }

    effectCallback->applyEffect(caster_, skillEffectResult);

    const permil_t topThreatPerValue = getTopThreatPerValue(dataCode);
    if (topThreatPerValue > 0) {
        go::Thinkable* thinkable = target.queryThinkable();
        if (thinkable != nullptr) {
            ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
            if (aggroList != nullptr) {
                aggroList->addThreatByTopThreat(target, topThreatPerValue);
            }
        }
    }
    return ecOk;
}


ErrorCode EffectHelper::applyCasterEffect(DataCode dataCode, SkillType skillType, const EffectValues& effectValues, 
    go::Entity& target, const datatable::SkillEffectTemplate& effectTemplate)
{
    gc::EffectCallback* effectCallback = caster_.getController().queryEffectCallback();
    if (! effectCallback) {
        return ecSkillInvalidAvailableTarget;
    }

    if (! effectTemplate.hasEffects(true)) {
        return ecOk;
    }

    SkillEffectResult skillEffectResult(dataCode);
    skillEffectResult.isCasterEffect_ = true;
    applyTargetEffect_i(skillEffectResult, skillType, effectValues, target, effectTemplate, edtNone);
    if (effectTemplate.hasDashEffect()) {
        for (int i = eanStart; i < eanCount; ++i) {
            if (isDashScript(effectTemplate.casterEffects_[i].script_)) {
                applyDash(skillEffectResult.effects_[i].position_, caster_, target.getPosition());
                break;
            }                
        }
    }

    effectCallback->applyEffect(caster_, skillEffectResult);
    return ecOk;
}


void EffectHelper::applyCasterPassiveEffect(SkillCode skillCode, SkillType skillType, 
    const EffectValues& effectValues, const datatable::SkillEffectTemplate& effectTemplate)
{
    gc::PassiveEffectCallback* effectCallback = caster_.getController().queryPassiveEffectCallback();
    if (! effectCallback) {
        return;
    }

    if (! effectTemplate.hasEffects(true)) {
        return;
    }

    SkillEffectResult skillEffectResult(skillCode);
    skillEffectResult.isCasterEffect_ = true;
    applyTargetEffect_i(skillEffectResult, skillType, effectValues, caster_, effectTemplate, edtNone);

    effectCallback->addCasterEffect(skillEffectResult);
}


void EffectHelper::releaseCasterPassiveEffect(SkillCode skillCode)
{
    gc::PassiveEffectCallback* effectCallback = caster_.getController().queryPassiveEffectCallback();
    if (! effectCallback) {
        return;
    }

    effectCallback->removeCasterEffect(skillCode);
}


ErrorCode EffectHelper::canTargetEffected(SkillType skillType, bool /*isCheckSafeRegion*/, 
    EffectLevel effectLevel, const datatable::SkillCastCheckPointInfo& checkPointInfo, 
    const datatable::SkillEffectTemplate& effectTemplate, go::Entity& target) const
{
    if (target.isHarvest() || target.isTreasure() || target.isDungeon()) {
        return ecSkillInvalidAvailableTarget;
    }

    if (target.isGraveStone()) {
        if (static_cast<go::StaticObject&>(target).getStaticObjectStatus().isMinHp()) {
            return ecGraveStoneMinHp;
        }
    }

    if (isBuffSkillType(skillType) && target.isCreature()) {
        if (! target.getEffectScriptApplier().isApplyEffect(effectTemplate.category_, effectLevel)) {
            return ecSkillSameCagegory;
        }
    }

    if (! isAvailableTarget(target, effectTemplate.checkInfo_.availableTarget_)) {
        return ecSkillInvalidAvailableTarget;
    }

    CreatureState* creatureState = target.queryCreatureState();
    if (creatureState && creatureState->isSpawnProtection()) {
        return ecSkillTargetSpawnProtection;
    }

    if (effectTemplate.checkInfo_.exceptTarget_ == etSelf && 
        effectTemplate.checkInfo_.targetingType_ != ttSelfArea) {
        if (getCaster().isSame(target)) {        
            return ecSkillSelfExceptSkill;
        }
    }

    if (isExceptTarget(target, effectTemplate.checkInfo_.exceptTarget_)) {
        return ecSkillNotAvailable;
    }

    if (checkPointInfo.checkStatTarget_ == csttCastee) {
        return checkCastableTargetStatus(checkPointInfo, target);
    }

    return ecOk;
}


ErrorCode EffectHelper::checkCastableTargetStatus(const datatable::SkillCastCheckPointInfo& checkPointInfo, go::Entity& target) const
{    
    if (target.isAnchorOrBuilding() || target.isGraveStone()) {
        return ecOk;
    }

    go::CastCheckable* castCheckable = target.queryCastCheckable();
    if (! castCheckable) {
        return ecSkillInvalidAvailableTarget;
    }

    return castCheckable->checkCastablePoints(checkPointInfo.pointType_,
        checkPointInfo.isUsePercent_, checkPointInfo.isUpCheckValue_,
        checkPointInfo.checkValue_);
}

}} // namespace gideon { namespace zoneserver {

