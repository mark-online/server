#include "ZoneServerPCH.h"
#include "CreatureEffectScriptCommand.h"
#include "EffectScriptApplier.h"
#include "../../state/CreatureState.h"
#include "../../../service/party/Party.h"
#include "../../../service/time/GameTimer.h"
#include "../../../service/teleport/TeleportService.h"
#include "../Creature.h"
#include "../ability/Achievementable.h"
#include "../ability/Chaoable.h"
#include "../ability/Rewardable.h"
#include "../ability/Partyable.h"
#include "../ability/Inventoryable.h"
#include "../ability/Knowable.h"
#include "../ability/AggroSendable.h"
#include "../ability/Thinkable.h"
#include "../status/CreatureStatus.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/LifeStatusCallback.h"
#include "../../../controller/callback/EffectCallback.h"
#include "../../../controller/callback/CreatureEffectCallback.h"
#include "../../../controller/callback/StatsCallback.h"
#include "../../../controller/callback/RewardCallback.h"
#include "../../../controller/callback/InventoryCallback.h"
#include "../../../controller/callback/TeleportCallback.h"
#include "../../../world/WorldMap.h"
#include "../../../ai/Brain.h"
#include "../../../ai/aggro/AggroList.h"
#include <gideon/3d/3d.h>

namespace gideon { namespace zoneserver { namespace go {

const float32_t upRevisionValue = 0.5f; 

void CreatureEffectScriptCommand::updateThreatToMyTarget(const GameObjectInfo& senderInfo, uint32_t aggroValue, 
    bool isDivision, bool isRemove)
{
    owner_.queryAggroSendable()->updateTagetThreat(senderInfo, aggroValue, isDivision, isRemove);
}


void CreatureEffectScriptCommand::updateThreatToMe(const GameObjectInfo& senderInfo, uint32_t aggroValue, 
    bool isApplyScore, bool isRemove)
{
    go::Entity* sender = owner_.queryKnowable()->getEntity(senderInfo);
    if (! sender) {
        return;
    }
    const uint32_t score = aggroValue; 
    go::AggroSendable* sendalbe = sender->queryAggroSendable();
    if (sendalbe) {
        aggroValue = sendalbe->getApplyThreat(aggroValue);
    }

    go::Thinkable* thinkable = owner_.queryThinkable();
    if (thinkable != nullptr) {
        ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
        if (aggroList != nullptr) {
            if (isRemove) {
                aggroList->downThreat(*sender, aggroValue);
            }
            else {
                aggroList->addThreat(*sender, aggroValue, isApplyScore ? score : 0);
            }
        }
    }
}

void CreatureEffectScriptCommand::removeThreatToTarget(const GameObjectInfo& enemyInfo)
{
    go::Entity* enemy = owner_.queryKnowable()->getEntity(enemyInfo);
    if (! enemy) {
        return;
    }

    go::Thinkable* thinkable = enemy->queryThinkable();
    if (thinkable != nullptr) {
        ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
        if (aggroList != nullptr) {
            aggroList->removeThreat(owner_.getGameObjectInfo());            
        }
    }
}


void CreatureEffectScriptCommand::setOneThreatToTarget(const GameObjectInfo& enemyInfo)
{
    go::Entity* enemy = owner_.queryKnowable()->getEntity(enemyInfo);
    if (! enemy) {
        return;
    }

    go::Thinkable* thinkable = enemy->queryThinkable();
    if (thinkable != nullptr) {
        ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
        if (aggroList != nullptr) {
            aggroList->setThreat(owner_, 1);            
        }
    }
}

// = SkillLifePointsStatsEffectCommand

void SkillLifePointsStatsEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    if (isRemove || owner_.getCreatureStatus().isDied()) {
        return;
    }
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
    HitPoint applyHp = hpMin;
    ManaPoint applyMp = mpMin;

    switch (effect.script_) {
    case estHpAbsorb:
    case estMpAbsorb:
        absorb(effect, effectPtr->gameObjectInfo_);    
        break;
    case estHpGive:
    case estMpGive:
        give(effect, effectPtr->gameObjectInfo_);
        break;
    case estHpUp:
        owner_.getCreatureStatus().fillHp(applyHp, toHitPoint(effect.value_));
        updateThreatToMyTarget(effectPtr->gameObjectInfo_, static_cast<uint32_t>(applyHp * upRevisionValue), true, false);
        break;
    case estHpDown:
        owner_.getCreatureStatus().reduceHp(applyHp, toHitPoint(effect.value_), true);
        updateThreatToMe(effectPtr->gameObjectInfo_, applyHp, true, false);
        break;
    case estMpUp:
        owner_.getCreatureStatus().fillMp(applyMp, toManaPoint(effect.value_));
        updateThreatToMyTarget(effectPtr->gameObjectInfo_, static_cast<uint32_t>(applyMp * upRevisionValue), true, false);;
        break;
    case estMpDown:
        owner_.getCreatureStatus().reduceMp(applyMp, toManaPoint(effect.value_));
        updateThreatToMe(effectPtr->gameObjectInfo_, applyMp, true, false);
        break;
    }

    if (owner_.getCreatureStatus().isDied()) {
        CreatureState* creatureState = owner_.queryCreatureState();
        if (creatureState && creatureState->isDied()) {
            return;
        }
        gc::LifeStatusCallback* lifeStatusCallback = 
            owner_.getController().queryLifeStatusCallback();
        if (lifeStatusCallback) {
            go::Entity* entity = owner_.queryKnowable()->getEntity(effectPtr->gameObjectInfo_);  
            lifeStatusCallback->died(entity);
        }
    }
}


void SkillLifePointsStatsEffectCommand::absorb(ServerEffectInfo::Effect& effect, const GameObjectInfo& casterInfo)
{
    go::Entity* caster = 
        owner_.queryKnowable()->getEntity(casterInfo);
    if (! caster) {
        return;
    }
    if (caster->isCreature()) {
        const UnionEntityInfo& entityInfo = owner_.getUnionEntityInfo();
        const CreatureInfo& creatureInfo = entityInfo.asCreatureInfo();
        if (effect.script_ == estHpAbsorb) {
            HitPoint applyPoint = hpMin;
            HitPoint tempApplyPoint = hpMin;
            if (creatureInfo.currentPoints_.hp_ >= effect.value_) {                
                static_cast<go::Creature&>(*caster).getCreatureStatus().fillHp(tempApplyPoint, toHitPoint(effect.value_));
                owner_.getCreatureStatus().reduceHp(applyPoint, toHitPoint(effect.value_), true);
            }
            else {
                static_cast<go::Creature&>(*caster).getCreatureStatus().fillHp(tempApplyPoint, creatureInfo.currentPoints_.hp_);
                owner_.getCreatureStatus().reduceHp(applyPoint, creatureInfo.currentPoints_.hp_, true);                
            }
            updateThreatToMe(casterInfo, applyPoint, true, false);
        }
        else if (effect.script_ == estMpAbsorb) {
            ManaPoint applyPoint = mpMin;
            ManaPoint tempApplyPoint = mpMin;
            if (creatureInfo.currentPoints_.mp_ >= effect.value_) {
                static_cast<go::Creature&>(*caster).getCreatureStatus().fillMp(tempApplyPoint, toManaPoint(effect.value_));
                owner_.getCreatureStatus().reduceMp(applyPoint, toManaPoint(effect.value_));
            }
            else {
                static_cast<go::Creature&>(*caster).getCreatureStatus().fillMp(tempApplyPoint, creatureInfo.currentPoints_.mp_);
                owner_.getCreatureStatus().reduceMp(applyPoint, creatureInfo.currentPoints_.mp_);
            }
            updateThreatToMe(casterInfo, applyPoint, true, false);
        }
    }    
}


void SkillLifePointsStatsEffectCommand::give(ServerEffectInfo::Effect& effect, const GameObjectInfo& casterInfo)
{
    go::Entity* caster = 
        owner_.queryKnowable()->getEntity(casterInfo);
    if (! caster) {
        return;
    }
    if (caster->isCreature()) {
        const UnionEntityInfo& entityInfo = caster->getUnionEntityInfo();
        const CreatureInfo& creatureInfo = entityInfo.asCharacterInfo();
        if (effect.script_ == estHpGive) {
            HitPoint applyPoint = hpMin;
            if (creatureInfo.currentPoints_.hp_ >= effect.value_) {
                static_cast<go::Creature&>(*caster).getCreatureStatus().reduceHp(toHitPoint(effect.value_), true);
                owner_.getCreatureStatus().fillHp(applyPoint, toHitPoint(effect.value_));
            }
            else {
                static_cast<go::Creature&>(*caster).getCreatureStatus().reduceHp(creatureInfo.currentPoints_.hp_, true);
                owner_.getCreatureStatus().fillHp(applyPoint, creatureInfo.currentPoints_.hp_);
            }
            updateThreatToMyTarget(casterInfo, static_cast<uint32_t>(applyPoint * upRevisionValue), true, false);
        }
        else if (effect.script_ == estMpGive) {
            ManaPoint applyPoint = mpMin;
            if (creatureInfo.currentPoints_.mp_ >= effect.value_) {
                static_cast<go::Creature&>(*caster).getCreatureStatus().reduceMp(toManaPoint(effect.value_));
                owner_.getCreatureStatus().fillMp(applyPoint, toManaPoint(effect.value_));
            }
            else {
                static_cast<go::Creature&>(*caster).getCreatureStatus().reduceMp(creatureInfo.currentPoints_.mp_);
                owner_.getCreatureStatus().fillMp(applyPoint, creatureInfo.currentPoints_.mp_);
            }
            updateThreatToMyTarget(casterInfo, static_cast<uint32_t>(applyPoint * upRevisionValue), true, false);
        }
    }   
}

// = SkillLifeCapacityStatsEffectCommand

void SkillLifeCapacityStatsEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
	if (estHpCapacityUp == effect.script_) {
		if (isRemove) {
			downLifeStatsCapacity(effect, ptHp, isRemove);
		}
		else {
			upLifeStatsCapacity(effect, ptHp, isRemove);
			owner_.getCreatureStatus().fillHp(toHitPoint(effect.value_));
		}
		return;
	}
	else if (estMpCapacityUp == effect.script_) {
		if (isRemove) {
			downLifeStatsCapacity(effect, ptMp, isRemove);
		}
		else {
			upLifeStatsCapacity(effect, ptMp, isRemove);
			owner_.getCreatureStatus().fillMp(toManaPoint(effect.value_));
		}
		return;
	}
    else if (estHpShield == effect.script_ || estHpShield == effect.script_) {
        if (isRemove) {
            owner_.getCreatureStatus().destoryShield(effect.script_ == estHpShield ? ptHp : ptMp);
        }
        else {
            owner_.getCreatureStatus().createShield(effect.script_ == estHpShield ? ptHp : ptMp,
                uint32_t(effect.value_));
        }
        return;
    }
    

    switch (effect.script_) {
    case estHpCapacityDown:
        isRemove ? upLifeStatsCapacity(effect, ptHp, isRemove) : downLifeStatsCapacity(effect, ptHp, isRemove);
        break;
    case estMpCapacityDown:
        isRemove ? upLifeStatsCapacity(effect, ptMp, isRemove) : downLifeStatsCapacity(effect, ptMp, isRemove);
        break;
    }
}


void SkillLifeCapacityStatsEffectCommand::upLifeStatsCapacity(ServerEffectInfo::Effect& effect, PointType pointType, bool isRemove)
{
    uint32_t realPoint =
        isRemove ? uint32_t(effect.accumulationValue_) : uint32_t(effect.value_);
     
    switch (pointType) {
    case ptHp:
        owner_.getCreatureStatus().upHpCapacity(toHitPoint(realPoint));
        break;
    case ptMp:
        owner_.getCreatureStatus().upMpCapacity(toManaPoint(realPoint));
        break;
    default:
        assert(false);
        return;
    }

    if (isRemove) {
        effect.accumulationValue_ = 0;
    }
    else {
        effect.accumulationValue_ += realPoint;
    }
}


void SkillLifeCapacityStatsEffectCommand::downLifeStatsCapacity(ServerEffectInfo::Effect& effect, PointType pointType, bool isRemove)
{
    uint32_t realValue =
        isRemove ? uint32_t(effect.accumulationValue_) : uint32_t(effect.value_);
  
    switch (pointType) {
    case ptHp:
        realValue = owner_.getCreatureStatus().downHpCapacity(toHitPoint(realValue));
        break;
    case ptMp:
        realValue = owner_.getCreatureStatus().downMpCapacity(toManaPoint(realValue));
        break;
    default:
        assert(false);
        return;
    }

    if (isRemove) {
        effect.accumulationValue_ = 0;
    }
    else {
        effect.accumulationValue_ += realValue;
    }
}

// = SkillAttackPowerEffectCommand
void SkillAttackPowerEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
    switch (effect.script_)
    {
    case estPhysicalUp:
        isRemove ? owner_.getCreatureStatus().downAttackPowers(apiPhysical, effect.value_) :
            owner_.getCreatureStatus().upAttackPowers(apiPhysical, effect.value_);
        break;
    case estMagicUp:
        isRemove ? owner_.getCreatureStatus().downAttackPowers(apiMagic, effect.value_) :
            owner_.getCreatureStatus().upAttackPowers(apiMagic, effect.value_);
        break;
    case estPhysicalDown:
        isRemove ? owner_.getCreatureStatus().upAttackPowers(apiPhysical, effect.value_) :
            owner_.getCreatureStatus().downAttackPowers(apiPhysical, effect.value_);
        break;
    case estMagicDown:
        isRemove ? owner_.getCreatureStatus().upAttackPowers(apiMagic, effect.value_) :
            owner_.getCreatureStatus().downAttackPowers(apiMagic, effect.value_);
        break;
    }
}



// = SkillAttackStatusEffectCommand
void SkillAttackStatusEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
    switch (effect.script_)
    {
    case estPhysicalCriticalRateUp:
        isRemove ? owner_.getCreatureStatus().downAttackStatus(atsiPhysicalCriticalRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upAttackStatus(atsiPhysicalCriticalRate, static_cast<int16_t>(effect.value_));
        break;
    case estMagicCriticalRateUp:
        isRemove ? owner_.getCreatureStatus().downAttackStatus(atsiMagicCriticalRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upAttackStatus(atsiMagicCriticalRate, static_cast<int16_t>(effect.value_));
        break;
    case estHitRateUp:
        isRemove ? owner_.getCreatureStatus().downAttackStatus(atsiHitRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upAttackStatus(atsiHitRate, static_cast<int16_t>(effect.value_));
        break;
    case estPhysicalCriticalRateDown:
        isRemove ? owner_.getCreatureStatus().downAttackStatus(atsiPhysicalCriticalRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upAttackStatus(atsiPhysicalCriticalRate, static_cast<int16_t>(effect.value_));
        break;
    case estMagicCriticalRateDown:
        isRemove ? owner_.getCreatureStatus().downAttackStatus(atsiMagicCriticalRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upAttackStatus(atsiMagicCriticalRate, static_cast<int16_t>(effect.value_));
        break;
    case estHitRateDown:
        isRemove ? owner_.getCreatureStatus().downAttackStatus(atsiHitRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upAttackStatus(atsiHitRate, static_cast<int16_t>(effect.value_));
        break;
    }
}


// = SkillDefenceStatusEffectCommand
void SkillDefenceStatusEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
    switch (effect.script_)
    {
    case estBlockRateUp:
        isRemove ? owner_.getCreatureStatus().downDefenceStatus(dsiBlockRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upDefenceStatus(dsiBlockRate, static_cast<int16_t>(effect.value_));
        break;
    case estDodgeRateUp:
        isRemove ? owner_.getCreatureStatus().downDefenceStatus(dsiDodgeRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upDefenceStatus(dsiDodgeRate, static_cast<int16_t>(effect.value_));
        break;
    case estDefenceUp:
        isRemove ? owner_.getCreatureStatus().downDefenceStatus(dsiDefence, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().upDefenceStatus(dsiDefence, static_cast<int16_t>(effect.value_));
        break;
    case estBlockRateDown:
        isRemove ? owner_.getCreatureStatus().upDefenceStatus(dsiBlockRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().downDefenceStatus(dsiBlockRate, static_cast<int16_t>(effect.value_));
        break;
    case estDodgeRateDown:
        isRemove ? owner_.getCreatureStatus().upDefenceStatus(dsiDodgeRate, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().downDefenceStatus(dsiDodgeRate, static_cast<int16_t>(effect.value_));
        break;
    case estDefenceDown:
        isRemove ? owner_.getCreatureStatus().upDefenceStatus(dsiDefence, static_cast<int16_t>(effect.value_)) :
            owner_.getCreatureStatus().downDefenceStatus(dsiDefence, static_cast<int16_t>(effect.value_));
        break;
    }
}


// = SkillAttributeRateStatsEffectCommand

void SkillAttributeEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
    switch (effect.script_)
    {
    case estLightUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiLight, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().upAttribute(abiLight, static_cast<int16_t>(effect.value_), false);
        break;
    case estDarkUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiDark, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().upAttribute(abiDark, static_cast<int16_t>(effect.value_), false);
        break;
    case estFireUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiFire, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().upAttribute(abiFire, static_cast<int16_t>(effect.value_), false);
        break;
    case estIceUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiIce, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().upAttribute(abiIce, static_cast<int16_t>(effect.value_), false);
        break;
    case estAllAttributeUp:
        isRemove ? owner_.getCreatureStatus().downAllAttribute(static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().upAllAttribute(static_cast<int16_t>(effect.value_), false);
        break;
    case estLightDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiLight, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().downAttribute(abiLight, static_cast<int16_t>(effect.value_), false);
        break;
    case estDarkDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiDark, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().downAttribute(abiDark, static_cast<int16_t>(effect.value_), false);
        break;
    case estFireDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiFire, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().downAttribute(abiFire, static_cast<int16_t>(effect.value_), false);
        break;
    case estIceDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiIce, static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().downAttribute(abiIce, static_cast<int16_t>(effect.value_), false);
        break;
    case estAllAttributeDown:
        isRemove ? owner_.getCreatureStatus().upAllAttribute(static_cast<int16_t>(effect.value_), false) :
            owner_.getCreatureStatus().downAllAttribute(static_cast<int16_t>(effect.value_), false);
        break;
    case estLightResistUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiLight, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().upAttribute(abiLight, static_cast<int16_t>(effect.value_), true);
        break;
    case estDarkResistUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiDark, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().upAttribute(abiDark, static_cast<int16_t>(effect.value_), true);
        break;
    case estFireResistUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiFire, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().upAttribute(abiFire, static_cast<int16_t>(effect.value_), true);
        break;
    case estIceResistUp:
        isRemove ? owner_.getCreatureStatus().downAttribute(abiIce, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().upAttribute(abiIce, static_cast<int16_t>(effect.value_), true);
        break;
    case estAllAttributeResistUp:
        isRemove ? owner_.getCreatureStatus().downAllAttribute(static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().upAllAttribute(static_cast<int16_t>(effect.value_), true);
        break;
    case estLightResistDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiLight, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().downAttribute(abiLight, static_cast<int16_t>(effect.value_), true);
        break;
    case estDarkResistDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiDark, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().downAttribute(abiDark, static_cast<int16_t>(effect.value_), true);
        break;
    case estFireResistDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiFire, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().downAttribute(abiFire, static_cast<int16_t>(effect.value_), true);
        break;
    case estIceResistDown:
        isRemove ? owner_.getCreatureStatus().upAttribute(abiIce, static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().downAttribute(abiIce, static_cast<int16_t>(effect.value_), true);
        break;
    case estAllAttributeResistDown:
        isRemove ? owner_.getCreatureStatus().upAllAttribute(static_cast<int16_t>(effect.value_), true) :
            owner_.getCreatureStatus().downAllAttribute(static_cast<int16_t>(effect.value_), true);
        break;
    }
    
}


// = SkillExecutionEffectCommand

void SkillExecutionEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (estExecution != effect.script_ || isRemove) {
        return;
    }
    
    if (owner_.isPlayer()) {
        if (owner_.getCreatureStatus().isDied()) {
            rewardExp(effectPtr->gameObjectInfo_);
        }
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->standGraveStone();
        }

        go::Entity* executor = owner_.queryKnowable()->getEntity(effectPtr->gameObjectInfo_);
        if (executor) {
            go::Achievementable* achievementable =executor->queryAchievementable();
            if (achievementable) {
                achievementable->updateAchievement(acmtExecution, &owner_);
            }
        }
    }
}



void SkillExecutionEffectCommand::rewardExp(const GameObjectInfo& skillCasterInfo)
{
    go::Entity* skillCaster = owner_.queryKnowable()->getEntity(skillCasterInfo);
    if (! skillCaster) {
		return;
	}

    ExpPoint rewardExp = owner_.queryRewardable()->getRewardExpPoint();
    bool shouldReward = true;
    go::Partyable* partyable = skillCaster->queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->rewardExp(rewardExp, owner_.getCurrentWorldMap()->getMapCode(), owner_.getPosition());
            shouldReward = false;
        }
    }

    if (shouldReward) {
        gc::RewardCallback* rewardCallbck = skillCaster->getController().queryRewardCallback();
        if (rewardCallbck) {
            rewardCallbck->expRewarded(rewardExp);
        }
    }   
}


// = SkillStunEffectCommand

void SkillStunEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estStun) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztStun, ! isRemove);
            if (! isRemove) {
                setOneThreatToTarget(effectPtr->gameObjectInfo_);
            }
        }
    }                    
}

// = SkillSleepEffectCommand

void SkillSleepEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estSleep) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztSleep, ! isRemove);
            if (! isRemove) {
                setOneThreatToTarget(effectPtr->gameObjectInfo_);
            }
        }
    }
}


// = SkillPanicEffectCommand

void SkillPanicEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estPanic) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->panic(effect.activateTime_, ! isRemove);
            if (! isRemove) {
                setOneThreatToTarget(effectPtr->gameObjectInfo_);
            }
        }    
    }
}

// = SkillMutationEffectCommand

void SkillMutationEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estMutation) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->mutate(effect.value_, effect.activateTime_, ! isRemove);
        }
    }

}

// = SkillTransformationEffectCommand

void SkillTransformationEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estTransformation) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->transform(NpcCode(effect.value_), ! isRemove);
            if (! isRemove) {
                setOneThreatToTarget(effectPtr->gameObjectInfo_);
            }
        }
    }

}

// = SkillHideEffectCommand

void SkillHideEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estHide) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztHide, ! isRemove);
        }
    }
}

// = SkillShacklesEffectCommand

void SkillShacklesEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estShackles) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztShackles, ! isRemove);
            if (! isRemove) {
                setOneThreatToTarget(effectPtr->gameObjectInfo_);
            }
        }
    }

}

// = SkillMoveUpEffectCommand

void SkillMoveUpEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
    Moveable* moveable = owner_.queryMoveable();
    assert(moveable != nullptr);

    const float32_t oldSpeed = moveable->getSpeed();

    if (isRemove) {
        moveable->setSpeedRate(permil_t(-effect.accumulationValue_));
        effect.accumulationValue_ = 0;
    }
    else {
        effect.accumulationValue_ += effect.value_;
        moveable->setSpeedRate(permil_t(effect.accumulationValue_));
    }

    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->changeMoveSpeed(oldSpeed);
    }
}

// = SkillMoveDownEffectCommand

void SkillMoveDownEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.value_ == 0) {
        return;
    }
    Moveable* moveable = owner_.queryMoveable();
    assert(moveable != nullptr);

    const float32_t oldSpeed = moveable->getSpeed();

    if (isRemove) {
        moveable->setSpeedRate(permil_t(effect.accumulationValue_));
        effect.accumulationValue_ = 0;
    }
    else {
        effect.accumulationValue_ += effect.value_;
        moveable->setSpeedRate(permil_t(-effect.accumulationValue_));
    }       

    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->changeMoveSpeed(oldSpeed);
    }
}


// = SkillFrenzyEffectCommand

void SkillFrenzyEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estFrenzy) {
        owner_.frenze(effect.value_, isRemove);

        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztFrenzy, ! isRemove);
        }   
    }
}

// = SkillParalyzeEffectCommand

void SkillParalyzeEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estParalyze) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztParalyze, ! isRemove);
        }     
    }
}

// = SkillCastingDelayUpEffectCommand

void SkillCastingDelayUpEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{    
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estCastTimeUp) {
        if (isRemove) {
            owner_.downCastingDelay(effect.value_);
        }
        else {
            owner_.upCastingDelay(effect.value_);
        }
    }
}

// = SkillCastingDelayDownEffectCommand

void SkillCastingDelayDownEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estCastTimeDown) {
        if (isRemove) {
            owner_.upCastingDelay(effect.accumulationValue_);             
        }
        else {
            effect.accumulationValue_ = owner_.downCastingDelay(effect.value_);
        }
    }
}

// = SkillKnockbackEffectCommand

void SkillKnockbackEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];

    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        if (! isRemove) {
            callback->knockback(effect.position_);
            setOneThreatToTarget(effectPtr->gameObjectInfo_);
        }
        else {
            callback->knockbackRelease();
        }

        // TODO: 알릴 필요 있나?
        //Moveable* moveable = owner_.queryMoveable();
        //assert(moveable != nullptr);
        //callback->changeMoveSpeed(moveable->getSpeed());
    }
}

// = SpawnProtectionEffectCommand

void SpawnProtectionEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estSpawnProtection) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztSpawnProtection, ! isRemove);        
        }
    } 
}


// = InvincibleEffectCommand

void InvincibleEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estInvincible) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztInvincible, ! isRemove);        
        }
    } 
}

// = BindRecallEffectCommand

void BindRecallEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    effect, isRemove;
    WorldMap* worldMap = owner_.getCurrentWorldMap();
    if (! worldMap) {
        return;
    }

    bool isChao = false;
    
    Chaoable* chaoable = owner_.queryChaoable();
    if (chaoable) {
        isChao = chaoable->isChao();
    }
    const WorldPosition spawnPosition(
        worldMap->getNearestGravePosition(owner_.getPosition(), isChao),
        worldMap->getMapCode());
    const ErrorCode errorCode =
        TELEPORT_SERVICE->teleportPosition(spawnPosition, owner_);
    if (isFailed(errorCode)) {
        // TODO: 클라이언트에게 어떻게 알리나?
        return;
    }
}

// = SkillReleaseScriptCommand

void SkillReleaseScriptCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool /*isRemove*/)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    EffectStackCategory category = toEffectStackCategory(effect.value_);
    owner_.getEffectScriptApplier().cancelRemoveEffect(category);    
}

// = PlayerRecallCommand

void PlayerRecallCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool /*isRemove*/)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estPlayerRecall) {
        gc::TeleportCallback* callback = owner_.getController().queryTeleportCallback();
        if (callback) {
            callback->recallReadied();
        }
	}	
}


void DisableMagicCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estDisableMagic) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->effectiveMesmerization(meztDisableMagic, ! isRemove);
        }  
    }
}


// = SkillDashEffectCommand

void SkillDashEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool /*isRemove*/)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->dash(effect.position_);
    }
}

// = SkillReviveEffectCommand

void SkillReviveEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool /*isRemove*/)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->reviveAtOnce(static_cast<permil_t>(effect.value_));
    }
}


void DirectThreatEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool /*isRemove*/)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    if (effect.script_ == estDirectThreatUp) {
        updateThreatToMe(effectPtr->gameObjectInfo_, effect.value_, false, false);
    }
    if (effect.script_ == estDirectThreatDown) {
        updateThreatToMe(effectPtr->gameObjectInfo_, effect.value_, false, true);
    }
}


void ThreatEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];
    
    if (isRemove) {
        isRemove = effect.script_ == estThreatUp ? true : false;     
    }
    else {
        isRemove = effect.script_ == estThreatDown ? false : true; 
    }

    go::AggroSendable* sendalbe = owner_.queryAggroSendable();
    if (sendalbe) {
        sendalbe->updateAdditionThreat(effect.value_, isRemove);
    }
}


void ThreatApplyAvoidEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];

    go::Thinkable* thinkable = owner_.queryThinkable();
    if (thinkable != nullptr) {
        ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
        if (aggroList && isRemove) {
            aggroList->downThreatApplyAvoidTarget(effectPtr->gameObjectInfo_, effect.value_);
        }
        else if (aggroList && ! isRemove) {
            aggroList->upThreatApplyAvoidTarget(effectPtr->gameObjectInfo_, effect.value_);
        }
    }
}


void ForceVictimEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum /*num*/, bool isRemove)
{
    go::Thinkable* thinkable = owner_.queryThinkable();
    if (thinkable != nullptr) {
        ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
        if (aggroList && isRemove) {
            aggroList->releaseForceVictim(effectPtr->gameObjectInfo_);
        }
        else if (aggroList && ! isRemove) {
            aggroList->setForceVictim(effectPtr->gameObjectInfo_);
        }
    }
}


void BackStepEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool /*isRemove*/)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];

    go::Entity* caster = owner_.queryKnowable()->getEntity(effectPtr->gameObjectInfo_);
    if (! caster) {
        return;
    }

    const float32_t distance = (effect.value_ / 100.0f);
    ObjectPosition newPosition = owner_.getPosition();
    const Vector2 moveDirection = -getDirection(newPosition.heading_);
    newPosition.x_ += moveDirection.x * distance;
    newPosition.y_ += moveDirection.y * distance;
    TELEPORT_SERVICE->teleportPosition(WorldPosition(newPosition,
        owner_.getCurrentWorldMap()->getMapCode()), *caster);
    
}

}}} // namespace gideon { namespace zoneserver { namespace go {
