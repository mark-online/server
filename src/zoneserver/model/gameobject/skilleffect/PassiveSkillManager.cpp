#include "ZoneServerPCH.h"
#include "PassiveSkillManager.h"
#include "PassiveSkill.h"
#include "../ability/SkillCastable.h"
#include "../../../service/time/GameTimer.h"
#include "../Creature.h"
#include "../Entity.h"
#include "../status/CreatureStatus.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/LifeStatusCallback.h"
#include "../../../controller/callback/EffectCallback.h"
#include "../../../controller/callback/PassiveSkillCallback.h"
#include "../../../controller/callback/CreatureEffectCallback.h"
#include "../../../controller/callback/StatsCallback.h"
#include <gideon/cs/datatable/PlayerPassiveSkillTable.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace go {

namespace {


inline void removeTimeoutEffect(ServerEffects& effects, PassiveSkillManager::ServerEffectMap& targetEffects)
{
    GameTime now = GAME_TIMER->msec();
    PassiveSkillManager::ServerEffectMap::iterator pos = targetEffects.begin();
    while (pos != targetEffects.end()) {
        const ServerEffectPtr effectPtr = (*pos).second;
        ServerEffectInfo::Effects serverEffects = effectPtr->effects_;
        for (int i = 0; i < eanCount; ++i) {
            if (isTimeoutEffect(effectPtr, now, toEffectApplyNum(i))) {
                serverEffects[i].isRemove_ = true;
            }
        }

        if (serverEffects[0].isRemove_ && serverEffects[1].isRemove_) {
            effects.push_back(effectPtr);
            pos = targetEffects.erase(pos);
            continue;
        }
        ++pos;
    }
}

inline bool fillServerEffectInfo(const datatable::PassiveSkillTemplate& skillTemplate,
    const datatable::SkillEffectTemplate& effectTemplate,
    ServerEffectInfo& serverEffectInfo)
{
    const GameTime currentTime = GAME_TIMER->msec();
 
    serverEffectInfo.dataCode_ = skillTemplate.skillCode_;
    serverEffectInfo.isCaster_ = true;
    serverEffectInfo.startTime_ = getTime();
    serverEffectInfo.category_ = effectTemplate.category_;
    serverEffectInfo.effectLevel_ = skillTemplate.effectLevel_;
    for (int i = 0; i < eanCount; ++i) {      
        const GameTime activeTime = skillTemplate.casterSkillActivateTimeInfos_[i].activateMilSec_;
        const GameTime intervalTime = skillTemplate.casterSkillActivateTimeInfos_[i].activateIntervalMilSec_;            
        if (activeTime > 0) {
            serverEffectInfo.effects_[i].stopTime_ = activeTime + currentTime;
            serverEffectInfo.effects_[i].activateTime_ = activeTime;
        }
        if (intervalTime > 0 && activeTime > intervalTime) {	
            serverEffectInfo.effects_[i].nextIntervalTime_ = intervalTime + currentTime;
            serverEffectInfo.effects_[i].callCount_ = (activeTime / intervalTime);
        }

        serverEffectInfo.effects_[i].script_ = effectTemplate.casterEffects_[i].script_;
        if (serverEffectInfo.effects_[i].script_ == estUnknown) {
            serverEffectInfo.effects_[i].isRemove_ = true;
        }
        else {
            serverEffectInfo.effects_[i].shouldExcute_ = true;
        }
    }
    
    return serverEffectInfo.isValid();
}


inline ServerEffectPtr createServerEffectPtr(const datatable::PassiveSkillTemplate& skillTemplate,
    const datatable::SkillEffectTemplate& effectTemplate,
    const SkillEffectResult& effectResult, const GameObjectInfo& fromObjectInfo)
{    
    ServerEffectInfo serverEffectInfo; 
    if (! fillServerEffectInfo(skillTemplate, effectTemplate, serverEffectInfo)) {
        assert(false);
        return ServerEffectPtr();
    }

    for (int i = 0; i < eanCount; ++i) {        
        serverEffectInfo.effects_[i].value_ = effectResult.effects_[i].value_;        
        serverEffectInfo.effects_[i].position_ = effectResult.effects_[i].position_;
    }

    serverEffectInfo.gameObjectInfo_ = fromObjectInfo;
    return std::make_shared<ServerEffectInfo>(serverEffectInfo);
}

} // namespace {


PassiveSkillManager::PassiveSkillManager(Creature& owner) :
    owner_(owner)
{
    conditionSkillMap_.emplace(pccPoints, PassiveSkills());
    conditionSkillMap_.emplace(pccUseableState, PassiveSkills());
    conditionSkillMap_.emplace(pccEquip, PassiveSkills());
    conditionSkillMap_.emplace(pccLearnSkill, PassiveSkills());
    conditionSkillMap_.emplace(pccTimer, PassiveSkills());

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
        ssetMoveSpeedDown, new SkillMoveDownEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetMoveSpeedUp, new SkillMoveUpEffectCommand(owner)));
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetHide, new SkillHideEffectCommand(owner)));

}


PassiveSkillManager::~PassiveSkillManager()
{
}


void PassiveSkillManager::finalize()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    passiveSkillMap_.clear();
    passiveSkillMap_.clear();
    conditionSkillMap_.clear();
    commandMap_.clear();
}


ErrorCode PassiveSkillManager::learn(SkillCode skillCode)
{
    SkillIndex index = getSkillIndex(skillCode);

    const datatable::PassiveSkillTemplate* skillTemplate = 
        PLAYER_PASSIVE_SKILL_TABLE->getPlayerSkill(skillCode);
    if (! skillTemplate) {
        assert(false);
        return ecSkillInvalidSkill;
    }

    const datatable::SkillEffectTemplate* effectTemplate = 
        SKILL_EFFECT_TABLE->getSkillEffectTemplate(skillTemplate->effectIndex_);
    if (! effectTemplate) {
        assert(false);
        return ecSkillInvalidSkill;
    }

    SkillCode passiveSkillCode = getSkillCode(index);
    if (passiveSkillCode == skillCode) {
        return ecSkillAlreayRegistPassiveSkill;
    }    
    
    if (isValidSkillCode(passiveSkillCode)) {
        const ErrorCode errorCode = unlearn(passiveSkillCode);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }

    addSkill(*skillTemplate, *effectTemplate);
    return ecOk;
}


ErrorCode PassiveSkillManager::unlearn(SkillCode skillCode)
{    
    PassiveSkillPtr passiveSkill = getPassiveSkill(skillCode);
    if (passiveSkill.get() == nullptr) {
        return ecSkillInvalidSkill;
    }  

    removeSkill(skillCode);
    return ecOk;
}


void PassiveSkillManager::applySkill(const SkillEffectResult& skillEffectResult)
{
    PassiveSkillPtr passiveSkill = getPassiveSkill(skillEffectResult.dataCode_);
    if (passiveSkill.get() == nullptr) {
        assert(false);
        return;
    }


    ServerEffectPtr effectPtr = createServerEffectPtr(passiveSkill->getPassiveSkillTemplate(),
        passiveSkill->getSkillEffectTemplate(), skillEffectResult, owner_.getGameObjectInfo());
    if (effectPtr.get() == nullptr) {
        return;
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (effectPtr->hasTimeEffect() && effectPtr->hasOnceEffect()) {
            assert(false);
        }
        else if (effectPtr->hasTimeEffect()) {
            timeEffectMap_.emplace(effectPtr->dataCode_, effectPtr);
            notifyEffectAdded(DebuffBuffEffectInfo(effectPtr->dataCode_, true, getTime(), effectPtr->effects_[eanEffect_1].value_,
                effectPtr->effects_[eanEffect_2].value_));
            if (effectPtr->hasInstantEffect()) {
                excuteEffect(effectPtr);
            }
        }        
        else if (effectPtr->hasInstantEffect()) {
            excuteEffect(effectPtr);
            onceApplyEffectMap_.emplace(effectPtr->dataCode_, effectPtr);
        }
    }    
}


void PassiveSkillManager::revert()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    for (ServerEffectMap::value_type& value : timeEffectMap_) {
        cancelEffect(value.second);
    }
    timeEffectMap_.clear();
}


void PassiveSkillManager::cancelSkill(SkillCode skillCode)
{
    ServerEffectPtr effectPtr;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        const ServerEffectMap::iterator tPos = timeEffectMap_.find(skillCode);
        if (tPos != timeEffectMap_.end()) {
            effectPtr = (*tPos).second;
            timeEffectMap_.erase(tPos);
        }
        const ServerEffectMap::iterator oPos = onceApplyEffectMap_.find(skillCode);
        if (oPos != onceApplyEffectMap_.end()) {
            effectPtr = (*oPos).second;
            onceApplyEffectMap_.erase(oPos);
        }
    }
    if (effectPtr.get()) {
        cancelEffect(effectPtr);
    }
}


void PassiveSkillManager::tick()
{
    removeTimeoutEffects();
    executeEffects();
}


void PassiveSkillManager::notifyChangeCondition(PassiveCheckCondition condition)
{
	sne::core::Vector<SkillCode> skillCodes;
			
	PassiveSkills passiveSkills = getPassiveSkills(condition);
	for (PassiveSkillPtr passiveSkill : passiveSkills) {
		bool preSuccessCondition = passiveSkill->isSuccessCondition();
		passiveSkill->changeCondition(condition);
		if (passiveSkill->hasTimeSkill()) {
			if (! passiveSkill->isCooldown() && passiveSkill->isSuccessCondition()) {
				passiveSkill->activate();
			}
		}
		else {
			if (! preSuccessCondition && passiveSkill->isSuccessCondition()) {
				passiveSkill->activate();
			}
			else if (preSuccessCondition && ! passiveSkill->isSuccessCondition()){
				passiveSkill->deactivate();
			}
		}        
	}
}


void PassiveSkillManager::addSkill(const datatable::PassiveSkillTemplate& skillTemplate,
    const datatable::SkillEffectTemplate& effectTemplate)
{        
    auto passiveSkill = std::make_shared<PassiveSkill>(owner_, effectTemplate, skillTemplate);

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (skillTemplate.shouldCheckSelfPoints()) {
            addConditionSkill(passiveSkill, pccPoints);
        }
        if (skillTemplate.shouldCheckUsableState()) {
            addConditionSkill(passiveSkill, pccUseableState);
        }
        if (skillTemplate.shouldCheckEquip()) {
            addConditionSkill(passiveSkill, pccEquip);
        }
        if (skillTemplate.shouldCheckLeanedSkill()) {
            addConditionSkill(passiveSkill, pccLearnSkill);
        }
        if (skillTemplate.hasCooldown()) {
            addConditionSkill(passiveSkill, pccTimer);
        }

        passiveSkillMap_.emplace(skillTemplate.skillCode_, passiveSkill);
    }

    if (passiveSkill->isSuccessCondition()) {
        passiveSkill->activate();
    }
}


void PassiveSkillManager::removeSkill(SkillCode skillCode)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    PassiveSkillPtr passiveSkill = getPassiveSkill(skillCode);
    if (passiveSkill.get() != nullptr) {
        // 작동되고 있으면 풀어준다.
        const datatable::PassiveSkillTemplate& skillTemplate = passiveSkill->getPassiveSkillTemplate();
        if (skillTemplate.shouldCheckSelfPoints()) {
            removeConditionSkill(skillCode, pccPoints);
        }
        if (skillTemplate.shouldCheckUsableState()) {
            removeConditionSkill(skillCode, pccUseableState);
        }
        if (skillTemplate.shouldCheckEquip()) {
            removeConditionSkill(skillCode, pccEquip);
        }
        if (skillTemplate.shouldCheckLeanedSkill()) {
            removeConditionSkill(skillCode, pccLearnSkill);
        }
        if (skillTemplate.hasCooldown()) {
            removeConditionSkill(skillCode, pccTimer);
        }

        passiveSkillMap_.erase(skillCode);
    }
    cancelSkill(skillCode);
}


void PassiveSkillManager::addConditionSkill(PassiveSkillPtr passiveSkill, PassiveCheckCondition condition)
{
    PassiveSkillsMap::iterator pos = conditionSkillMap_.find(condition);
    if (pos != conditionSkillMap_.end()) {
        PassiveSkills& passiveSkills = (*pos).second;
        passiveSkills.push_back(passiveSkill);
    }
}


void  PassiveSkillManager::removeConditionSkill(SkillCode skillCode, PassiveCheckCondition condition)    
{
    PassiveSkillsMap::iterator pos = conditionSkillMap_.find(condition);
    if (pos != conditionSkillMap_.end()) {
        PassiveSkills& passiveSkills = (*pos).second;
        PassiveSkills::iterator vecPos = passiveSkills.begin();
        PassiveSkills::iterator end = passiveSkills.end();
        for (; vecPos != end; ++vecPos) {
            if ((*vecPos)->getSkillCode() == skillCode) {
                passiveSkills.erase(vecPos);
                return;
            }
        }
    }
}


void PassiveSkillManager::removeTimeoutEffects()
{
    ServerEffects effects;
    effects.reserve(timeEffectMap_.size());

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        removeTimeoutEffect(effects, timeEffectMap_);
    }

    
    for (ServerEffectPtr effectPtr : effects) {
        cancelEffect(effectPtr);
    }    
}


void PassiveSkillManager::executeEffects()
{
    ServerEffects effects;
    GameTime now = GAME_TIMER->msec();
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        for (ServerEffectMap::value_type& value : timeEffectMap_) {
            ServerEffectPtr effectPtr = value.second;
            for (int i = 0; i < eanCount; ++i) {
                if (isExcuteEffect(effectPtr, now, toEffectApplyNum(i))) {
                    fillNextEffectTime(effects, effectPtr, toEffectApplyNum(i));
                }
            }            
        }
    }

    for (ServerEffectPtr effectPtr : effects) {
        excuteEffect(effectPtr);
        if (owner_.getCreatureStatus().isDied()) {
            break;
        }
    }
}


void PassiveSkillManager::cancelHasTimeEffect()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    for (ServerEffectMap::value_type& value : timeEffectMap_) {
        ServerEffectPtr effectPtr = value.second;
        cancelEffect(effectPtr);
    }

    timeEffectMap_.clear();
}


void PassiveSkillManager::cancelEffect(ServerEffectPtr effectPtr)
{
    for (int i = 0; i < eanCount; ++i) {
        EffectScriptCommand* command = getEffectInfoCommand(effectPtr->effects_[i].script_);
        if (command) {
            command->execute(effectPtr, toEffectApplyNum(i), true);
            effectPtr->effects_[i].isRemove_ = true;
        }
    }
    if (effectPtr->shouldRemove() && effectPtr->hasTimeEffect()) {
        notifyEffectRemoved(effectPtr->dataCode_);
    }
}


void PassiveSkillManager::excuteEffect(ServerEffectPtr effectPtr)
{
    const bool isBeforDie = owner_.getCreatureStatus().isDied();
    if (isBeforDie) {
        return;
    }

    for (int i = 0; i < eanCount; ++i) {
        if (! effectPtr->effects_[i].shouldExcute_ || effectPtr->effects_[i].isRemove_ || ! isValid(effectPtr->effects_[i].script_)) {
            continue;
        } 
        EffectScriptCommand* command = getEffectInfoCommand(effectPtr->effects_[i].script_);
        if (command) {
            command->execute(effectPtr, toEffectApplyNum(i), false);
            effectPtr->effects_[i].shouldExcute_ = false;
            if (owner_.getCreatureStatus().isDied()) {
                return;                
            }
        }
    }
    
    const bool isAfterDie = owner_.getCreatureStatus().isDied();
    if (! isBeforDie && isAfterDie) {
        cancelHasTimeEffect();
    }
}


void PassiveSkillManager::notifyEffectAdded(const DebuffBuffEffectInfo& info)
{
    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->addEffect(info);
    }

    debuffBuffEffectInfoSet_.insert(info);
}


void PassiveSkillManager::notifyEffectRemoved(DataCode dataCode)
{
    gc::CreatureEffectCallback* callback = 
        owner_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->removeEffect(dataCode, true);
    }

    debuffBuffEffectInfoSet_.erase(DebuffBuffEffectInfo(dataCode, true));
}


PassiveSkillManager::PassiveSkillPtr PassiveSkillManager::getPassiveSkill(SkillCode skillCode)
{
    PassiveSkillMap::iterator pos = passiveSkillMap_.find(skillCode);
    if (pos != passiveSkillMap_.end()) {
        return (*pos).second;
    }
    return PassiveSkillPtr();
}


PassiveSkillManager::PassiveSkills& PassiveSkillManager::getPassiveSkills(PassiveCheckCondition condition)
{
    PassiveSkillsMap::iterator pos = conditionSkillMap_.find(condition);
    if (pos != conditionSkillMap_.end()) {
        return (*pos).second;
    }
    static PassiveSkills nullObj;
    return nullObj;
}


SkillCode PassiveSkillManager::getSkillCode(SkillIndex index) const
{
    for (PassiveSkillMap::value_type value : passiveSkillMap_) {
        SkillCode skillCode = value.first;
        if (index == getSkillIndex(skillCode)) {
            return skillCode;
        }
    }
    return invalidSkillCode;
}


EffectScriptCommand* PassiveSkillManager::getEffectInfoCommand(EffectScriptType type)
{
    if (type == estUnknown) {
        return nullptr;
    }
    ServerEffectType findType = toServerEffectType(type);
    EffectSciptCommandMap::iterator pos = commandMap_.find(findType);
    if (pos != commandMap_.end()) {
        return (*pos).second;
    }
    return nullptr;    
}


}}} // namespace gideon { namespace zoneserver { namespace go {
