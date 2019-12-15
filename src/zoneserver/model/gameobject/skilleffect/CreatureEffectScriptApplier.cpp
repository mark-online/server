#include "ZoneServerPCH.h"
#include "CreatureEffectScriptApplier.h"
#include "CreatureEffectScriptCommand.h"
#include "../../../service/time/GameTimer.h"
#include "../Creature.h"
#include "../status/CreatureStatus.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/CreatureEffectCallback.h"


namespace gideon { namespace zoneserver { namespace go {

CreatureEffectScriptApplier::CreatureEffectScriptApplier(go::Entity& owner) :
    EffectScriptApplier(owner)
{
}


CreatureEffectScriptApplier::~CreatureEffectScriptApplier()
{
}


const DebuffBuffEffectInfoSet& CreatureEffectScriptApplier::getDebuffBuffEffectInfoSet() const
{
    return debuffBuffEffectInfoSet_;
}


void CreatureEffectScriptApplier::initialzeEffectCommand()
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetLifeStats, new SkillLifePointsStatsEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetLifeCapacityStatus, new SkillLifeCapacityStatsEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetAttributeRateStatus, new SkillAttributeEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetAttackPowerStatus, new SkillAttackPowerEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetAttackStatus, new SkillAttackStatusEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetDefenceStatus, new SkillDefenceStatusEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetExecution, new SkillExecutionEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetStun, new SkillStunEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetSleep, new SkillSleepEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetMutation, new SkillMutationEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetHide, new SkillHideEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetFrenzy, new SkillFrenzyEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetParalyze, new SkillParalyzeEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetMoveSpeedDown, new SkillMoveDownEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetMoveSpeedUp, new SkillMoveUpEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetKnockback, new SkillKnockbackEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetShackles, new SkillShacklesEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetSpawnProtection, new SpawnProtectionEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetInvincible, new InvincibleEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetBindRecall, new BindRecallEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
		ssetRelease, new SkillReleaseScriptCommand(owner)));
	commandMap_.insert(EffectSciptCommandMap::value_type(
		ssetPlayerRecall, new PlayerRecallCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetDisableMagic, new DisableMagicCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetDash, new SkillDashEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetPanic, new SkillPanicEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetRevive, new SkillReviveEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetDirectThreat, new DirectThreatEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetThreat, new ThreatEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetThreatApplyAvoid, new ThreatApplyAvoidEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetForceVictim, new ForceVictimEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetBackStep, new BackStepEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetCastingUp, new SkillCastingDelayUpEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetCastingDown, new SkillCastingDelayDownEffectCommand(owner)));

}


void CreatureEffectScriptApplier::excuteEffect(ServerEffectPtr effectPtr)
{
    const bool isBeforDie = getOwnerAs<go::Creature>().getCreatureStatus().isDied();
    if (isBeforDie && (! (effectPtr->hasEffect(estExecution) || effectPtr->hasEffect(estRevive)))) {
        return;
    }
    
    for (int i = 0; i < eanCount; ++i) {
        if (! effectPtr->effects_[i].shouldExcute_ || effectPtr->effects_[i].isRemove_|| ! isValid(effectPtr->effects_[i].script_)) {
            continue;
        }        
        
        EffectScriptCommand* command = getEffectInfoCommand(effectPtr->effects_[i].script_);
        if (command) {
            command->execute(effectPtr, toEffectApplyNum(i), false);
            if (! effectPtr->isTimeEffect(toEffectApplyNum(i)) && effectPtr->isInstantEffect(toEffectApplyNum(i))) {
                effectPtr->effects_[i].isRemove_ = true;
            }
            effectPtr->effects_[i].shouldExcute_ = false;
            if (! canExecuteEffect()) {
                return;                
            }
        }
    }
    
    const bool isAfterDie = getOwnerAs<go::Creature>().getCreatureStatus().isDied();
    if (! isBeforDie && isAfterDie) {
        died();
    }
}



void CreatureEffectScriptApplier::notifyEffectAdded(const DebuffBuffEffectInfo& info, bool isNotify)
{
    if (! isValidDataCode(info.code_)) {
        return;
    }
    if (isNotify) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->addEffect(info);
        }
    }
    
    debuffBuffEffectInfoSet_.insert(info);
}


void CreatureEffectScriptApplier::notifyEffectRemoved(DataCode dataCode, bool isCaster)
{
    if (! isValidDataCode(dataCode)) {
        return;
    }
    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->removeEffect(dataCode, isCaster);
    }
    
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        debuffBuffEffectInfoSet_.erase(DebuffBuffEffectInfo(dataCode, isCaster));    

    }
}


void CreatureEffectScriptApplier::died()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    for (ServerEffectMap::value_type& value : effectMap_) {
        ServerEffectPtr effectPtr = value.second;
        cancelEffect(effectPtr);        
    }

    for (ServerEffectPtr effectPtr : effects_) {        
        cancelEffect(effectPtr);        
    }

    effectMap_.clear();
    effects_.clear();
}


bool CreatureEffectScriptApplier::canExecuteEffect() const
{
    return ! getOwnerAs<go::Creature>().getCreatureStatus().isDied();
}



}}} // namespace gideon { namespace zoneserver { namespace go {
