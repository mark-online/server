#include "ZoneServerPCH.h"
#include "EntityEffectScriptApplier.h"
#include "CreatureEffectScriptCommand.h"
#include "../../../service/time/GameTimer.h"
#include "../Creature.h"
#include "../Entity.h"
#include "../Player.h"
#include "../stats/CreatureLifeStats.h"
#include "../../../service/party/Party.h"
#include "../../../world/World.h"
#include "../../../world/WorldMap.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/LifeStatusCallback.h"
#include "../../../controller/callback/EffectCallback.h"
#include "../../../controller/callback/CreatureEffectCallback.h"
#include "../../../controller/callback/StatsCallback.h"
#include "../../../controller/callback/RewardCallback.h"
#include "../../../controller/callback/AnchorAbiilityEffectCallback.h"
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon { namespace zoneserver { namespace go {

namespace {

inline EffectInfoPtr createKeepEffectInfoPtr(keepConcentration kc, float32_t keepValue, GameTime activeTime)
{
	const GameTime stopTime = activeTime + GAME_TIMER->msec();
	EffectScriptType et = estUnknown;
	if (kc == kcCp) {
		et = estCpCapacityDown;
	}

	return EffectInfoPtr(new EffectInfo (sgNone, et, keepValue,
		activeTime, stopTime, 0, 0, 0, GameObjectInfo(), Position()));
}

}

CreatureEffectScriptApplier::CreatureEffectScriptApplier(go::Entity& owner, 
    GameStats& gameStats, GameStats& originalGameStats) :
    EffectScriptApplier(owner),
    gameStats_(gameStats),
    gameOriginalStats_(originalGameStats)
{
    initialzeEffectCommand();
}


CreatureEffectScriptApplier::~CreatureEffectScriptApplier()
{
}


void CreatureEffectScriptApplier::initialzeEffectCommand()
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetLifeStats, new SkillLifePointsStatsEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetLifeCapacityStats, new SkillLifeCapacityStatsEffectCommand(owner, gameStats_, false)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetAttributeRateStats, new SkillAttributeRateStatsEffectCommand(owner, gameStats_, gameOriginalStats_, false)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetPowerStats, new SkillPowerStatsEffectCommand(owner, gameStats_, gameOriginalStats_, false)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetGraveStone, new SkillGraveStoneEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetStun, new SkillStunEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetSleep, new SkillSleepEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetMutation, new SkillMutationEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetHide, new SkillHideEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetFrenzy, new SkillFrenzyEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetParalyze, new SkillParalyzeEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetMoveSpeedDown, new SkillMoveDownEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetMoveSpeedUp, new SkillMoveUpEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetKnockback, new SkillKnockbackEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetShackles, new SkillShacklesEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetInvincible, new InvincibleEffectCommand(owner, gameStats_)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetBindRecall, new BindRecallEffectCommand(owner, gameStats_)));
}



void CreatureEffectScriptApplier::applyKeepConcentration(const SkillEffectResult& skillEffectResult)
{
	for (int i = eanStart; i < eanCount; ++i) {
		const SkillEffectResult::Effect& effectResult = skillEffectResult.effects_[i];
		if (shouldKeepConcentration(effectResult.kc_)) {
			EffectInfoPtr effectInfo = createKeepEffectInfoPtr(effectResult.kc_,
                effectResult.keepConcentrationValue_, effectResult.activateTimer_);
			{
				ACE_GUARD(go::Entity::LockType, lock, owner_.getLock());
				effectInfos_.push_back(effectInfo);
			}			
			EffectScriptCommand* command = getEffectInfoCommand(effectInfo->type_);
			if (command) {
				command->execute(effectInfo, false);
				startEffectInfoTimer();
			}
		}
	}   
}


void CreatureEffectScriptApplier::excuteEffect(EffectInfoPtr effectInfo)
{
    const bool isBeforDie = getOwnerAs<go::Creature>().getLifeStats().isDied();
    if (effectInfo->type_ == estHideDamageRelease) {
        cancelSpellGroupEffect(sgHideDamage);
    }
    else {
        EffectScriptCommand* command = getEffectInfoCommand(effectInfo->type_);
        if (command) {
            command->execute(effectInfo, false);        
            if (effectInfo->type_ == estHpDamage) {
                if (! canEffectExcute()) {
                    return;                
                }
                cancelSpellGroupEffect(sgSleep);
                cancelSpellGroupEffect(sgMutation);
                if (! owner_.isSame(effectInfo->gameObjectInfo_)) {
                    cancelSpellGroupEffect(sgHide);
                }
            }
        }
    }
    const bool isAfterDie = getOwnerAs<go::Creature>().getLifeStats().isDied();
    if (! isBeforDie && isAfterDie) {
        died();
    }
}



void CreatureEffectScriptApplier::notifyEffectScriptAdded(EffectScriptType type)
{
    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->addEffectScipt(type);
    }
}


void CreatureEffectScriptApplier::notifyEffectScriptRemoved(const EffectScriptTypes& scipts)
{
    if (! scipts.empty()) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->removeEffectsScipt(scipts);
        }
    }
}


void CreatureEffectScriptApplier::died()
{
    ACE_GUARD(go::Entity::LockType, lock, owner_.getLock());

    BOOST_FOREACH(EffectInfoMap::value_type& value, effectInfoMap_) {
        EffectInfoPtr effectInfo = value.second;
        EffectScriptCommand* command = getEffectInfoCommand(effectInfo->type_);
        if (command) {
            command->execute(effectInfo, true);
        }
    }

    BOOST_FOREACH(EffectInfoPtr effectInfo, effectInfos_) {        
        EffectScriptCommand* command = getEffectInfoCommand(effectInfo->type_);
        if (command) {
            command->execute(effectInfo, true);
        }
    }

    effectInfoMap_.clear();
    effectInfos_.clear();
    stopEffectInfoTimer();
}


bool CreatureEffectScriptApplier::canEffectExcute() const
{
    return ! getOwnerAs<go::Creature>().getLifeStats().isDied();
}



}}} // namespace gideon { namespace zoneserver { namespace go {