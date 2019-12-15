#include "ZoneServerPCH.h"
#include "CreatureEffectHelper.h"
#include "../../../model/gameobject/Creature.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/CreatureStatusable.h"
#include "../../../model/gameobject/ability/Thinkable.h"
#include "../../../ai/Brain.h"
#include "../../../ai/aggro/AggroList.h"
#include "../../../helper/SkillTableHelper.h"
#include <gideon/cs/shared/data/LevelInfo.h>
#include <gideon/cs/shared/data/CombatRatingInfo.h>
#include <gideon/cs/shared/data/MonsterInfo.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver {

namespace {

inline bool shouldBlockRate(bool isBigBlcok)
{
    uint32_t defenceTypeValue = esut::random(1, 3);
    return (isBigBlcok && 1 < defenceTypeValue ) || (! isBigBlcok && 1 == defenceTypeValue);
}

inline bool isPoisonCategory(EffectStackCategory category)
{
    return escPoison == category;
}

} // namespace {

CreatureEffectHelper::CreatureEffectHelper(go::Entity& caster) :
    EffectHelper(caster)
{

}


CreatureEffectHelper::~CreatureEffectHelper()
{

}


void CreatureEffectHelper::applyTargetEffect_i(SkillEffectResult& skillEffectResult,
    SkillType skillType, const EffectValues& effectValues, go::Entity& target, 
    const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType effectDefenceType)
{    
    for (int i = 0; i < eanCount; ++i) {
        const datatable::EffectInfo* effectInfo = nullptr;
        if (skillEffectResult.isCasterEffect_) {
            effectInfo = &effectTemplate.casterEffects_[i];
        } 
        else {
            effectInfo = &effectTemplate.casteeEffects_[i];            
        }

        if (effectInfo && ! effectInfo->isValid()) {
            continue;
        }

        if (canImmune(effectInfo->script_)) {
            if (target.isNpcOrMonster()) {
                const MonsterGrade mosterGrade = static_cast<go::Npc&>(target).getNpcTemplate().getMonsterGrade();
                if (isBossMonster(mosterGrade)) {
                    skillEffectResult.effects_[i].isImmune_ = true;
                    continue;
                }
            }
        }
        
        const go::CreatureStatusable* statusable = getCaster().queryCreatureStatusable();
        const CreatureStatusInfo& casterStatusInfo = statusable->getCurrentCreatureStatusInfo();
        int32_t effectValue = effectValues[i]; 
        if (effectInfo->statusType_ != esttNone) {
            if (isStatusType(effectInfo->statusType_) && effectInfo->isPercent_) {
                if (target.isCreature()) {
                    if (effectInfo->target_ == utstCaster) {
                        effectValue = getCalcPercentStatusValue(casterStatusInfo, *effectInfo, effectValue);
                    }
                    else if (effectInfo->target_ == utstCastee) {
                        const go::CreatureStatusable* targetStatusable = target.queryCreatureStatusable();
                        const CreatureStatusInfo& targetStatusInfo = targetStatusable->getCurrentCreatureStatusInfo();
                        effectValue = getCalcPercentStatusValue(targetStatusInfo, *effectInfo, effectValue);
                    }
                    else {
                        assert(false);
                    }
                }
            }
            else if (isCalcAttackType(effectInfo->statusType_)) {
                int32_t attributeValue = 0;
                if (isValid(effectTemplate.attribute_) && effectDefenceType == edtResisted) {
                    skillEffectResult.effects_[i].value_ = 0;
                    continue;
                }
                else {
                    if (isValid(effectTemplate.attribute_)) {
                        attributeValue = (casterStatusInfo.attackPowers_[getAttackPowerIndex(effectInfo->statusType_)] * 
                            casterStatusInfo.attributes_[getAttributeRateIndex(effectTemplate.attribute_)]) / 1000;
                    }

                    if (effectInfo->isPercent_) {
                        effectValue = (casterStatusInfo.attackPowers_[getAttackPowerIndex(effectInfo->statusType_)] * effectValue) / 1000; 
                    }                
                    effectValue += static_cast<int32_t>(casterStatusInfo.attackPowers_.getRandomCalcAttackValue(getAttackPowerIndex(effectInfo->statusType_), attributeValue) * 1.5);
                }                
            }

            if (effectValue < 0) {
                assert(false);
                SNE_LOG_ERROR("Critical Error Apply SkillEffect(%d, caster_id:%" PRIu64 ", castee_id:%" PRIu64 ", %d)",
                    skillEffectResult.dataCode_, getCaster().getObjectId(), target.getObjectId(), effectValue);
                continue;
            }

            if (effectInfo->shouldCritical_) {
                const bips_t levelBonus = getCaster().getLevelBonus(target);
                const bips_t criticalChance = isPhysicalAttack(skillType) ?
                    getCaster().getPhysicalCriticalChance() : getCaster().getMagicCriticalChance();
                const bips_t roll = esut::random(0, 10000);
                if ((criticalChance - levelBonus) >= roll) {
                    effectValue = static_cast<int32_t>(esut::random(effectValue * 1.2f, effectValue * 1.4f));
                    skillEffectResult.effects_[i].isCritical_ = true;
                }
            }

            if (effectDefenceType == edtBlocked) {
                const float32_t blockValue = 0.3f; // 30% 감소
                const int32_t damageReduction = static_cast<int32_t>(effectValue * blockValue);
                skillEffectResult.effects_[i].decreaseValue_ += damageReduction;
                effectValue -= damageReduction;
                if (effectValue <= 0) {
                    effectValue = 1;
                }
                if (effectValue <= 1) {
                    continue;; // 더이상 처리해봐야 의미가 없으므로
                }
            }

            const bool poisonDot = isPoisonCategory(effectTemplate.category_) ? 
                isDotEffect(skillEffectResult.isCasterEffect_, skillEffectResult.dataCode_, toEffectApplyNum(i)) : false;

            const go::CreatureStatusable* targetStatusable = target.queryCreatureStatusable();
            if (targetStatusable && isAttackOrDebuffSkillType(skillType) && isDownPointTypeScript(effectInfo->script_) && ! poisonDot) {                
                const CreatureStatusInfo& targetStatusInfo = targetStatusable->getCurrentCreatureStatusInfo();
                int32_t damageReduction = 0;
                if (isPhysicalAttack(skillType)) {
                    const CreatureLevel attackerLevel = getCasterAs<go::Creature>().getCreatureLevel();
                    const float32_t damageReductionRate = getDamageReductionRate(targetStatusInfo.defenceStatus_[dsiDefence], attackerLevel);
                    damageReduction = static_cast<int32_t>(effectValue * damageReductionRate);
                }
                else if (isValid(effectTemplate.attribute_)) {
                    const float32_t resistRate = casterStatusInfo.attributeResists_[getAttributeRateIndex(effectTemplate.attribute_)] / 1000.0f;
                    damageReduction = static_cast<int32_t>(effectValue * resistRate);
                }
                if (damageReduction > 0) {
                    skillEffectResult.effects_[i].decreaseValue_ += damageReduction;
                    effectValue -= damageReduction;
                    if (effectValue <= 0) {
                        effectValue = 1;
                    }
                }
            }

            if (target.isGraveStone()) {
                effectValue = effectValue / 10; 
            }

            if ((effectInfo->script_ == estHpGive || effectInfo->script_ == estHpAbsorb) && effectValue < 1500) {
                effectValue = 1500;
            }
            else if ((effectInfo->script_ == estMpGive || effectInfo->script_ == estMpAbsorb) && effectValue < 250) {
                effectValue = 250;
            }
            
            skillEffectResult.effects_[i].value_ = effectValue;
        }
        else if (effectInfo->statusType_ == esttNone) {
            skillEffectResult.effects_[i].value_ = 0;
            if (isDirectThreatEffect(effectInfo->script_)) {
                go::Thinkable* thinkable = target.queryThinkable();
                if (thinkable) {
                    if (effectInfo->isPercent_) {
                        ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
                        if (aggroList) {
                            uint32_t threat = thinkable->getBrain().getAggroList()->getThreat(getCaster().getGameObjectInfo());
                            skillEffectResult.effects_[i].value_ = (threat * effectValues[i]) / 1000;
                        }
                    }
                    else {
                        skillEffectResult.effects_[i].value_ = effectValues[i];
                    }
                }                
            }
            else {
                skillEffectResult.effects_[i].value_ = effectValues[i];
            }
        }
    }    
}


}} // namespace gideon { namespace zoneserver {

