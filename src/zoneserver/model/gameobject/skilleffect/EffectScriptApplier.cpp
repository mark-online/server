#include "ZoneServerPCH.h"
#include "EffectScriptApplier.h"
#include "../Entity.h"
#include "../skilleffect/StaticObjectEffectScriptCommand.h"
#include "../ability/Partyable.h"
#include "../../state/CreatureState.h"
#include "../../../service/time/GameTimer.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/CooldownCallback.h"
#include "../../../controller/callback/EffectCallback.h"
#include "../../../controller/callback/StaticObjectEffectCallback.h"
#include "../../../controller/callback/AnchorCallback.h"
#include "../../../helper/SkillTableHelper.h"
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
* @class PartyMemberApplySkillEvent
*/
class PartyMemberApplySkillEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PartyMemberApplySkillEvent>
{
public:
    PartyMemberApplySkillEvent(go::Entity& to, go::Entity& from, const SkillEffectResult& skillResult) :
        to_(to),
        from_(from),
        skillResult_(skillResult) {}

private:
    virtual void call(go::Entity& entity) {
        gc::EffectCallback* callback = entity.getController().queryEffectCallback();
        if (callback) {
            callback->effectApplied(to_, from_, skillResult_);
        }
    }

private:
    go::Entity& to_;
    go::Entity& from_;
    SkillEffectResult skillResult_;
};

inline void removeTimeoutEffect(ServerEffects& effects, ServerEffectMap& targetEffects)
{
    const GameTime now = GAME_TIMER->msec();
    ServerEffectMap::iterator pos = targetEffects.begin();
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

inline void removeTimeoutEffect(ServerEffects& effects, ServerEffects& targetEffects)
{
    const GameTime now = GAME_TIMER->msec();
    ServerEffects::iterator pos = targetEffects.begin();
    while (pos != targetEffects.end()) {
        const ServerEffectPtr effectPtr = (*pos);
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


inline void fillServerEffectActivateTime(ServerEffectInfo& serverEffectInfo, const datatable::SkillActivateTimeInfos& skillActivateTimeInfos)
{
    const GameTime currentTime = GAME_TIMER->msec();
    for (int i = 0; i < eanCount; ++i ) {
        const GameTime activeTime = skillActivateTimeInfos[i].activateMilSec_;
        const GameTime intervalTime = skillActivateTimeInfos[i].activateIntervalMilSec_;

        serverEffectInfo.effects_[i].activateTime_ = activeTime;
        serverEffectInfo.effects_[i].intervalTime_ = intervalTime;

        if (activeTime > 0) {
            serverEffectInfo.effects_[i].stopTime_ = activeTime + currentTime;
        }

        if (intervalTime > 0 && activeTime > intervalTime) {	
            serverEffectInfo.effects_[i].nextIntervalTime_ = intervalTime + currentTime;
            serverEffectInfo.effects_[i].callCount_ = (activeTime / intervalTime);
        }
    }
}


inline bool fillServerEffectInfo(ServerEffectInfo& serverEffectInfo, DataCode dataCode, bool isCaster)
{
    const CodeType ct = getCodeType(dataCode);
    EffectIndex effectIndex = 0;
    
    if (ctSkill == ct) {
        SkillTableType skillTableType = getSkillTableType(dataCode);
        if (isActiveSkillTable(skillTableType)) {
            const datatable::ActiveSkillTemplate* skillTemplate = getActiveSkillTemplate(dataCode);
            if (! skillTemplate) {                
                return false;
            }
            effectIndex = skillTemplate->effectIndex_;
            serverEffectInfo.effectLevel_ = skillTemplate->effectLevel_;
            fillServerEffectActivateTime(serverEffectInfo, isCaster ? 
                skillTemplate->casterSkillActivateTimeInfos_ : skillTemplate->casteeSkillActivateTimeInfos_);
        }        
        else {
            const datatable::PassiveSkillTemplate* skillTemplate = PLAYER_PASSIVE_SKILL_TABLE->getPlayerSkill(dataCode);
            if (! skillTemplate) {                
                return false;
            }
            effectIndex = skillTemplate->effectIndex_;
            fillServerEffectActivateTime(serverEffectInfo, isCaster ? 
                skillTemplate->casterSkillActivateTimeInfos_ : skillTemplate->casteeSkillActivateTimeInfos_);
        }
    }
    

    const datatable::SkillEffectTemplate* effectTemplate = SKILL_EFFECT_TABLE->getSkillEffectTemplate(effectIndex);
    if (! effectTemplate) {
        return false;
    }

    serverEffectInfo.dataCode_ = dataCode;
    serverEffectInfo.isCaster_ = isCaster;
    serverEffectInfo.category_ = effectTemplate->category_;

    for (int i = 0; i < eanCount; ++i) {
        if (isCaster) {
            serverEffectInfo.effects_[i].script_ = effectTemplate->casterEffects_[i].script_;
        }
        else {
            serverEffectInfo.effects_[i].script_ = effectTemplate->casteeEffects_[i].script_;
        }

        if (serverEffectInfo.effects_[i].script_ == estUnknown) {
            serverEffectInfo.effects_[i].isRemove_ = true;
        }
        else {
            serverEffectInfo.effects_[i].shouldExcute_ = true;
        }
    }
    
    return serverEffectInfo.isValid();
}


inline ServerEffectPtr createServerEffectPtr(const SkillEffectResult& effectResult, const GameObjectInfo& fromObjectInfo)
{    
    ServerEffectInfo serverEffectInfo; 
    serverEffectInfo.startTime_ = getTime();
    if (! fillServerEffectInfo(serverEffectInfo, effectResult.dataCode_, effectResult.isCasterEffect_)) {
        assert(false);
        return ServerEffectPtr();
    }
    for (int i = 0; i < eanCount; ++i) {
        if (effectResult.effects_[i].isImmune_) {
            serverEffectInfo.effects_[i] = ServerEffectInfo::Effect();
        }
        else {
            serverEffectInfo.effects_[i].value_ = effectResult.effects_[i].value_;        
            serverEffectInfo.effects_[i].position_ = effectResult.effects_[i].position_;
        }
    }
    serverEffectInfo.gameObjectInfo_ = fromObjectInfo;
    return std::make_shared<ServerEffectInfo>(serverEffectInfo);
}


inline ServerEffectPtr createServerEffectPtr(const RemainEffectInfo& remainEffectInfo, const GameObjectInfo& fromObjectInfo)
{
    ServerEffectInfo serverEffectInfo; 
    EffectIndex effectIndex = invalidEffectIndex;
    SkillTableType skillTableType = getSkillTableType(remainEffectInfo.dataCode_);
    if (isActiveSkillTable(skillTableType)) {
        const datatable::ActiveSkillTemplate* skillTemplate = getActiveSkillTemplate(remainEffectInfo.dataCode_);
        effectIndex = skillTemplate->effectIndex_;
        const GameTime now = GAME_TIMER->msec();
        for (int i = 0; i < eanCount; ++i) {
            if (getTime() > remainEffectInfo.effects_[i].expireActivateTime_) {
                serverEffectInfo.effects_[i].isRemove_ = true;
                continue;
            }
            if (serverEffectInfo.startTime_ == 0 && skillTemplate->casterSkillActivateTimeInfos_[i].activateIntervalMilSec_ != 0) {
                serverEffectInfo.startTime_ = remainEffectInfo.effects_[i].expireActivateTime_ - skillTemplate->casterSkillActivateTimeInfos_[i].activateIntervalMilSec_ / 1000;
            }
            const GameTime remainActiveTime = static_cast<GameTime>((remainEffectInfo.effects_[i].expireActivateTime_ - getTime()) * 1000);
            const GameTime interval = remainEffectInfo.isCaster_ ? skillTemplate->casterSkillActivateTimeInfos_[i].activateIntervalMilSec_ :
                skillTemplate->casteeSkillActivateTimeInfos_[i].activateIntervalMilSec_;
            serverEffectInfo.effects_[i].stopTime_ = remainActiveTime + now; 
            if (interval > 0) {
                if (remainActiveTime > interval) {                
                    serverEffectInfo.effects_[i].nextIntervalTime_ = interval + now;
                    serverEffectInfo.effects_[i].callCount_ = (remainActiveTime / interval);
                }
            }
            serverEffectInfo.effects_[i].shouldExcute_ = true;
        }
    }
    else {
        assert(false);
        return ServerEffectPtr();
    }

    const datatable::SkillEffectTemplate* effectTemplate = SKILL_EFFECT_TABLE->getSkillEffectTemplate(effectIndex);
    if (! effectTemplate) {
        return ServerEffectPtr();
    }

    serverEffectInfo.dataCode_ = remainEffectInfo.dataCode_;
    serverEffectInfo.isCaster_ = fromObjectInfo.isValid();
    serverEffectInfo.category_ = effectTemplate->category_;
    for (int i = 0; i < eanCount; ++i) {
        if (serverEffectInfo.isCaster_) {
            serverEffectInfo.effects_[i].script_ = effectTemplate->casterEffects_[i].script_;
        }
        else {
            serverEffectInfo.effects_[i].script_ = effectTemplate->casteeEffects_[i].script_;
        }
        serverEffectInfo.effects_[i].value_ = remainEffectInfo.effects_[i].effectValue_;    
    }
    serverEffectInfo.gameObjectInfo_ = fromObjectInfo;
    return std::make_shared<ServerEffectInfo>(serverEffectInfo);
}

bool fillRemainEffectInfo(RemainEffectInfo& remainEffectInfo, go::Entity& owner, const ServerEffectPtr effectInfo)
{
    GameTime gameTimeNow = GAME_TIMER->msec();
    GameTime saveTime = GAME_TIMER->msec() + (cooldownSaveTime * 1000);
    sec_t localNow = getTime();

    bool isFill = false;
    remainEffectInfo.dataCode_ = effectInfo->dataCode_;
    remainEffectInfo.isCaster_ = effectInfo->isCaster_;
    remainEffectInfo.useTargetStatusType_ = owner.isSame(effectInfo->gameObjectInfo_) ? utstCaster : utstCastee;
    for (int i = 0; i < eanCount; ++i) {
        if (effectInfo->effects_[i].stopTime_ > saveTime && effectInfo->effects_[i].stopTime_ != 0) {
            const sec_t leftSec = (effectInfo->effects_[i].stopTime_ - gameTimeNow) / 1000;
            remainEffectInfo.effects_[i].effectValue_ = effectInfo->effects_[i].value_;
            remainEffectInfo.effects_[i].expireActivateTime_ = localNow + leftSec;
            isFill = true;
        }  
        else {
            remainEffectInfo.effects_[i].effectValue_ = 0;
            remainEffectInfo.effects_[i].expireActivateTime_ = 0;
        }
    }
    return isFill;
}

} // namespace

EffectScriptApplier::EffectScriptApplier(Entity& owner) :
    owner_(owner),
    effectId_(0)
{
}


EffectScriptApplier::~EffectScriptApplier()
{

}


void EffectScriptApplier::initialize()
{
    initialzeEffectCommand();
    effectId_ = 0;
}


void EffectScriptApplier::destroy()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    for (EffectSciptCommandMap::value_type& value : commandMap_) {
        EffectScriptCommand* effectCommand = value.second;
        boost::checked_delete(effectCommand);
    }
    effectId_ = 0;
    commandMap_.clear();
    effects_.clear();
}


void EffectScriptApplier::clean()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    effectMap_.clear();
    effects_.clear();
    effectId_ = 0;
}


void EffectScriptApplier::saveDB()
{
    gc::CooldownCallback* callback = owner_.getController().queryCooldownCallback();
    if (! callback) {
        return;
    }

    RemainEffectInfos remainEffectInfos;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! owner_.isValid()) {
            return;
        }

        for (const ServerEffectMap::value_type& value : effectMap_) {
            const ServerEffectPtr effectInfo = value.second;
            RemainEffectInfo remainInfo;
            if (fillRemainEffectInfo(remainInfo, owner_, effectInfo)) {
                remainEffectInfos.push_back(remainInfo);
            }
        }

        for (const ServerEffectPtr effectInfo : effects_) {
            RemainEffectInfo remainInfo;
            if (fillRemainEffectInfo(remainInfo, owner_, effectInfo)) {
                remainEffectInfos.push_back(remainInfo);
            }
        }
    }
    
    if (! remainEffectInfos.empty()) {
        callback->remainEffectsAdded(remainEffectInfos);
    }
}


void EffectScriptApplier::revert()
{	    
    ServerEffectMap effectMap;
    ServerEffects effects;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        effectMap = effectMap_;
        effects = effects_;
        effectMap_.clear();
        effects_.clear();
    }

    for (ServerEffectMap::value_type& value : effectMap) {
        const ServerEffectPtr effectPtr = value.second;
        cancelEffect(effectPtr);
    }

    for (ServerEffectPtr effectPtr : effects) {
        cancelEffect(effectPtr);
    }
}


bool EffectScriptApplier::hasEffectScipts(EffectScriptType effectScriptType) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    for (const ServerEffectMap::value_type& value : effectMap_) {
        const ServerEffectPtr effectInfo = value.second;
        if (effectInfo->hasEffect(effectScriptType)) {
            return true;
        }
    }

    for (const ServerEffectPtr effectInfo : effects_) {
        if (effectInfo->hasEffect(effectScriptType)) {
            return true;
        }
    }
    return false;
}


bool EffectScriptApplier::hasEffectStackCategory(EffectStackCategory category) const 
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasEffectStackCategory_i(category);
}



bool EffectScriptApplier::isApplyEffect(EffectStackCategory category, EffectLevel skillLevel)
{
    if (hasEffectStackCategory(category)) {
        ServerEffectPtr hasEffectPtr = getServerEffectPtr(category);
        if (hasEffectPtr->effectLevel_ > skillLevel) {
            return false;
        }
    }
    return true;
}


void EffectScriptApplier::applySkill(Entity& from, const SkillEffectResult& skillEffectResult)
{
    if (! skillEffectResult.isValid()) {
        return;
    }

    ServerEffectPtr effectPtr = createServerEffectPtr(skillEffectResult, from.getGameObjectInfo());
    if (effectPtr.get() == nullptr) {
        return;
    }

    gc::EffectCallback* ownerCallback = owner_.getController().queryEffectCallback();
    if (ownerCallback) {
        ownerCallback->effectApplied(owner_, from, skillEffectResult);
    }

    go::Partyable* ownerPartyable = owner_.queryPartyable();
    PartyId partyId = invalidPartyId;

    if (ownerPartyable) {
        const PartyPtr ownerParty = ownerPartyable->getParty();
        if (ownerParty.get()) {
            partyId = ownerParty->getPartyId();
            auto event = std::make_shared<PartyMemberApplySkillEvent>(owner_, from, skillEffectResult);
            ownerParty->notifyNearEvent(event, owner_.getObjectId(), true);
        }
    }

    bool isTargetPartyNotify = false;
    go::Partyable* targetPartyable = from.queryPartyable();
    if (targetPartyable) {
        const bool isSamePartyMember = targetPartyable->isSameParty(partyId);
        const PartyPtr targetParty = targetPartyable->getParty();
        if (targetParty.get() && ! isSamePartyMember) {
            auto event = std::make_shared<PartyMemberApplySkillEvent>(owner_, from, skillEffectResult);
            targetParty->notifyNearEvent(event, from.getObjectId(), false);
            isTargetPartyNotify = true;
        }
    }
    
    gc::EffectCallback* fromCallback = from.getController().queryEffectCallback();
    if (fromCallback && ! isTargetPartyNotify && ! from.isSame(owner_.getGameObjectInfo())) {
        fromCallback->effectApplied(owner_, from, skillEffectResult);
    }

    if (! from.isSame(owner_.getGameObjectInfo())) {
        CreatureState* creatureState = owner_.queryCreatureState();
        if (creatureState && creatureState->isInvincible()) {
            if (isAttackOrDebuffSkillType(getSkillType(skillEffectResult.dataCode_))) {
                return;
            }
        }
    }
    
    addEffect(effectPtr);
}


void EffectScriptApplier::applyRemainEffect(const RemainEffectInfo& remainEffect)
{
    const sec_t applyEnoughSec = 20;

    if (! remainEffect.canExcute(getTime() + applyEnoughSec)) {
        return;
    }

    ServerEffectPtr effectPtr = createServerEffectPtr(remainEffect, remainEffect.useTargetStatusType_ == utstCaster ? 
        owner_.getGameObjectInfo() : GameObjectInfo());
    if (effectPtr.get() == nullptr) {
        return;
    }

    addEffect(effectPtr ,false);
}


void EffectScriptApplier::applyEffectScript(EffectScriptType scriptType, GameTime activateTime)
{
    ServerEffectInfo effectInfo;
    effectInfo.effects_[0].script_ = scriptType;
    effectInfo.effects_[0].activateTime_ = activateTime;
    effectInfo.effects_[0].stopTime_ = GAME_TIMER->msec() + activateTime;
    effectInfo.effects_[0].shouldExcute_ = true;
    auto effectPtr = std::make_shared<ServerEffectInfo>(effectInfo);
    addEffect(effectPtr);
}


void EffectScriptApplier::tick()
{
    if (! owner_.isValid()) {
        return;
    }

	{
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (effectMap_.empty() && effects_.empty()) {
			return;
		}
	}
	
    removeTimeoutEffects();
    executeEffects();
}


void EffectScriptApplier::cancelRemoveEffect(EffectStackCategory category)
{
    ServerEffectPtr effectInfo = getServerEffectPtr(category);
    if (effectInfo.get() != nullptr) {
        cancelEffect(effectInfo);
        removeEffect(category);
    }
}


void EffectScriptApplier::addEffect(ServerEffectPtr effectPtr, bool isNotify)
{
    if (! canExecuteEffect() && (! (effectPtr->hasEffect(estExecution) || effectPtr->hasEffect(estRevive)))) {
        return;
    }

    if (hasEffectStackCategory(effectPtr->category_)) {
        ServerEffectPtr hasEffectPtr = getServerEffectPtr(effectPtr->category_);
        if (hasEffectPtr->effectLevel_ <= effectPtr->effectLevel_) {
            cancelRemoveEffect(effectPtr->category_);
        }
        else {
            return;
        }
    }

    if (effectPtr->hasTimeEffect()) {
        addHasTimeEffect(effectPtr);
        const DebuffBuffEffectInfo debuffBuffEffectInfo(effectPtr->dataCode_, effectPtr->isCaster_,
            effectPtr->startTime_, effectPtr->effects_[eanEffect_1].value_, effectPtr->effects_[eanEffect_2].value_);        
        notifyEffectAdded(debuffBuffEffectInfo, isNotify);    
    }

    if (effectPtr->hasInstantEffect()) {
        excuteEffect(effectPtr);
    }
}


void EffectScriptApplier::cancelEffect(ServerEffectPtr effectPtr)
{
    for (int i = 0; i < eanCount; ++i) {
        if (effectPtr->effects_[i].isRemove_ || ! isValid(effectPtr->effects_[i].script_)) {
            continue;
        }
        EffectScriptCommand* command = getEffectInfoCommand(effectPtr->effects_[i].script_);
        if (command) {
            command->execute(effectPtr, toEffectApplyNum(i), true);
            effectPtr->effects_[i].isRemove_ = true;
        }
    }
    if (effectPtr->shouldRemove()) {
        notifyEffectRemoved(effectPtr->dataCode_, effectPtr->isCaster_);
    }
}


void EffectScriptApplier::removeEffect(EffectStackCategory category)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    effectMap_.erase(category);
}


EffectScriptCommand* EffectScriptApplier::getEffectInfoCommand(EffectScriptType type)
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


ServerEffectPtr EffectScriptApplier::getServerEffectPtr(EffectStackCategory category)
{
    ServerEffectMap::iterator pos = effectMap_.find(category);
    if (pos != effectMap_.end()) {
        return (*pos).second;
    }

    return ServerEffectPtr();
}


void EffectScriptApplier::removeTimeoutEffects()
{
    ServerEffects effects;
    effects.reserve(effectMap_.size() + effects_.size());

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        removeTimeoutEffect(effects, effectMap_);
        removeTimeoutEffect(effects, effects_);
    }

    
    for (ServerEffectPtr effectPtr : effects) {
        cancelEffect(effectPtr);
    }    
}


void EffectScriptApplier::executeEffects()
{
    ServerEffects effects;
    GameTime now = GAME_TIMER->msec();
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        for (ServerEffectMap::value_type& value : effectMap_) {
            ServerEffectPtr effectPtr = value.second;
            for (int i = 0; i < eanCount; ++i) {
                if (isExcuteEffect(effectPtr, now, toEffectApplyNum(i))) {
                    fillNextEffectTime(effects, effectPtr, toEffectApplyNum(i));
                }
            }
        }

        for (ServerEffectPtr effectPtr : effects_) {
            for (int i = 0; i < eanCount; ++i) {
                if (isExcuteEffect(effectPtr, now, toEffectApplyNum(i))) {
                    fillNextEffectTime(effects, effectPtr, toEffectApplyNum(i));
                }
            }
        }
    }    

    for (ServerEffectPtr effectPtr : effects) {
        excuteEffect(effectPtr);
        if (! canExecuteEffect()) {
            break;
        }
    }
}


void EffectScriptApplier::addHasTimeEffect(ServerEffectPtr effectPtr)
{
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (escNone != effectPtr->category_) {
            effectMap_.emplace(effectPtr->category_, effectPtr);
        }
        else {
            effects_.push_back(effectPtr);
        }
    }        

    //notifyEffectScriptAdded(*effectInfo);
}

}}} // namespace gideon { namespace zoneserver { namespace go {
