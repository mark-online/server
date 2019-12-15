#include "ZoneServerPCH.h"
#include "PlayerState.h"
#include "../../gameobject/Creature.h"
#include "../../gameobject/Harvest.h"
#include "../../gameobject/Treasure.h"
#include "../../gameobject/ability/Chaoable.h"
#include "../../gameobject/ability/Castable.h"
#include "../../gameobject/ability/SkillCastable.h"
#include "../../gameobject/ability/ItemCastable.h"
#include "../../gameobject/ability/ArenaMatchable.h"
#include "../../gameobject/ability/Gliderable.h"
#include "../../gameobject/ability/Castnotificationable.h"
#include "../../gameobject/ability/Moveable.h"
#include "../../gameobject/ability/CastCheckable.h"
#include "../../../service/trade/TradeManager.h"
#include "../../../service/time/GameTimer.h"
#include "../../../service/arena/mode/Arena.h"
#include "../../../service/movement/MovementManager.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/GliderCallback.h"
#include "../../../controller/callback/CreatureEffectCallback.h"

namespace gideon { namespace zoneserver {

namespace {
        
CreatureStateInfo removeChaoAndWalkState(CreatureStateInfo characterStateInfo)
{
    CreatureStateInfo stateInfo = characterStateInfo;
    
    if (stateInfo.hasState(cstWalk)) {
        stateInfo.toggleState(cstWalk);
    }
    if (stateInfo.hasState(cstTempChao)) {
        stateInfo.toggleState(cstTempChao);
    }
    if (stateInfo.hasState(cstChao)) {
        stateInfo.toggleState(cstChao);
    }
    return stateInfo;
}

CreatureStateInfo removeVehicleMount(CreatureStateInfo characterStateInfo)
{
    CreatureStateInfo stateInfo = characterStateInfo;

    if (stateInfo.hasState(cstVehicleMount)) {
        stateInfo.toggleState(cstVehicleMount);
    }

    return stateInfo;
}

} // namespace {

PlayerState::PlayerState(go::Creature& owner, CreatureStateInfo& creatureStateInfo,
    CreatureStateInfo& characterStateInfo, EntityVehicleInfo& vehicleInfo,
	DataCode& gliderCode) :
	owner_(owner),
	creatureStateInfo_(creatureStateInfo),
    characterStateInfo_(characterStateInfo),
	vehicleInfo_(vehicleInfo),
	gliderCode_(gliderCode),
    isMovingCast_(true),
	gliderMountTime_(0),
    lastFallHeight_(0.0f),
    isInventoryLock_(false)
{
    stateIds_.fill(invalidObjectId);
}


PlayerState::~PlayerState()
{
}


void PlayerState::finalize()
{
    cancelCurrentStates();
}


bool PlayerState::changeCombatState(bool isCombatState)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if ((isCombatState && ! hasState(cstCombat)) || (! isCombatState && hasState(cstCombat))) {
        toggleState(cstCombat);
		return true;
    }
	return false;
}


bool PlayerState::canCastSkill() const
{
    if (hasState(cstDead) || hasState(cstStun) || hasState(cstSleep) ||
        hasState(cstTrade) || hasState(cstMutation) || hasState(cstParalyse) ||
        hasState(cstKnockBack) || hasState(cstCasting) ||
        hasState(cstGliderMount) || hasState(cstVehicleMount)) {
		return false;
    }

    return true;
}


bool PlayerState::canHarvest_i() const
{
    CreatureStateInfo state = removeChaoAndWalkState(creatureStateInfo_);
    state = removeVehicleMount(state);
    if (cstNormal == state.states_ || cstCombat == state.states_ || cstHide == state.states_) {
        return true;
    }
    return false;
}


bool PlayerState::canTreasureOpen_i() const
{
    CreatureStateInfo state = removeChaoAndWalkState(creatureStateInfo_);
    state = removeVehicleMount(state);
    if (cstNormal == state.states_ || cstCombat == state.states_ || cstHide == state.states_) {
        return true;
    }
    return false;
}


void PlayerState::cancelTrade()
{
    TRADE_MANAGER->cancelTrade(owner_.getObjectId(), TradeId(stateIds_[siiTrade]));
    stateIds_[siiTrade] = invalidObjectId;
}


void PlayerState::cancelCurrentStates() 
{
    float32_t oldSpeed = 0.0f;
    bool shouldCancelTrade = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (hasState(cstVehicleMount)) {
            oldSpeed = owner_.queryMoveable()->getSpeed();
            toggleState(cstVehicleMount);
        }

        if (hasState(cstGliderMount)) {
            oldSpeed = owner_.queryMoveable()->getSpeed();
            toggleState(cstGliderMount);
        }
    
        if (hasState(cstRequestTrade)) {
            toggleState(cstRequestTrade);
            shouldCancelTrade = true;
        }
        else if (hasState(cstTrade)) {
            toggleState(cstTrade);
            shouldCancelTrade = true;
        }
        lastFallHeight_ = 0.0f;
    }

    if (oldSpeed > 0) {
        gc::CreatureEffectCallback* callback = 
            owner_.getController().queryCreatureEffectCallback();
        if (callback) {
            callback->changeMoveSpeed(oldSpeed);
        }
    }

    cancelCasting();
    
    if (shouldCancelTrade) {
        cancelTrade();
    }
}

// = MoveState overriding

bool PlayerState::isRooted(const Position* position) const 
{
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (hasState(cstDead) || hasState(cstStun) || hasState(cstSleep) || 
            hasState(cstParalyse) || hasState(cstShackle) || hasState(cstKnockBack)) {
            return true;
        }
    }

    if (position != nullptr) {
        Arena* arena = owner_.queryArenaMatchable()->getArena();
        if (arena) {
            if (! arena->canMoving(owner_.getObjectId(), *position)) {
                return true;
            }
        }
    }

	return false;
}


bool PlayerState::isRunning() const 
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return ! hasState(cstWalk);
}


bool PlayerState::isControlAbsorbed() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstMutation);
}


bool PlayerState::isInLava() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return environmentType_ == evtLava;
}


HitPoint PlayerState::getFallDamage() const
{
    const float32_t minDamageHeight = 14.0f; 
    const float32_t currentHeight = owner_.getPosition().z_; 
    const float32_t diffHeight = lastFallHeight_ > currentHeight ? lastFallHeight_ - currentHeight : 0.0f;
    if (minDamageHeight < diffHeight && ! isGliderMounting()) {
        const float32_t param = 10.33f; 
        const float32_t deadHeight = 30.0f;
        const float32_t value = diffHeight / param;
        permil_t pct = static_cast<uint32_t>(std::pow(diffHeight, value));
        if (deadHeight < diffHeight|| 1000 < pct) {
            pct = 1000;
        }
        const HitPoint maxHp = owner_.getCreatureStatus().getHitPoints().maxHp_;
        return toHitPoint(static_cast<uint32_t>(maxHp * (pct / 1000.0f)));
    }
    return hpMin;
}


void PlayerState::moved(bool turn)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (turn) {
        return;
    }

    cancelMoveCasting();
}


void PlayerState::run()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstWalk)) {
        toggleState(cstWalk);
    }
}


void PlayerState::walked()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! hasState(cstWalk)) {
        toggleState(cstWalk);
    }
}


void PlayerState::fallen()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    lastFallHeight_ = owner_.getPosition().z_;
}


void PlayerState::landed()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    lastFallHeight_ = 0.0f;
}


void PlayerState::environmentEntered(EnvironmentType type)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());
    if (isValid(type)) {
        environmentType_ = type;
    }
}


void PlayerState::environmentLeft()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());
    environmentType_ = evtGround;
}


// = CreatureState overriding

bool PlayerState::isDied() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstDead);
}


void PlayerState::died()
{  
    cancelCurrentStates();

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        creatureStateInfo_.reset();
        characterStateInfo_.reset();
        toggleState(cstDead);
        gliderCode_ = invalidDataCode;
        lastFallHeight_ = 0.0f;
        if (owner_.queryChaoable()->isChao()) {
            toggleState(cstChao);			
        }
        if (owner_.queryChaoable()->isTempChao()) {
            toggleState(cstTempChao);			
		}
    }
}


void PlayerState::revived()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    creatureStateInfo_.reset();	
    characterStateInfo_.reset();

    if (owner_.queryChaoable()->isChao()) {
        toggleState(cstChao);			
    }
    if (owner_.queryChaoable()->isTempChao()) {
        toggleState(cstTempChao);			
	}
}


bool PlayerState::stunned(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
        owner_.setMoving(false);
    }
    
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstStun)) || (! isActivate && hasState(cstStun))) {
			toggleState(cstStun);
            return true;
        }
    }
	return false;
}


bool PlayerState::slept(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
        owner_.setMoving(false);
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstSleep)) || (! isActivate && hasState(cstSleep))) {
            toggleState(cstSleep);
            return true;
        }

    }
    return false;
}


bool PlayerState::mutated(NpcCode /*npcCode*/, sec_t duration, bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
    }

    bool updated = false;
    {
        // TODO: 코드를 사용해서 동기화를 할것인가 결정
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (isActivate && ! hasState(cstMutation)) { 
            toggleState(cstMutation);
            //setCastCode[sciMutation] = npcCode;
            updated = true;
        }
        else if (! isActivate && hasState(cstMutation)) {
            toggleState(cstMutation);
            //stateDataCodes_[sciMutation] = invalidNpcCode;
            updated = true;
        }
    }

    if (updated) {
        go::Moveable* moveable = owner_.queryMoveable();
        if (moveable != nullptr) {
            if (isActivate) {
                moveable->getMovementManager().setConfusedMovement(GameTime(duration * 1000));
            }
            else {
                moveable->getMovementManager().unsetConfusedMovement();
            }
        }
    }

    return updated;
}


bool PlayerState::transformed(NpcCode /*npcCode*/, bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (isActivate && ! hasState(cstTransformation)) {
            toggleState(cstTransformation);
            //stateDataCodes_[sciTransformation] = npcCode;
            return true;
        }
        else if (! isActivate && hasState(cstTransformation)) {
            toggleState(cstTransformation);
            //stateDataCodes_[sciTransformation] = invalidNpcCode;
            return true;
        }
    }

    return false;
}


bool PlayerState::panicked(sec_t duration, bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
    }

    bool updated = false;
    {
        // TODO: 코드를 사용해서 동기화를 할것인가 결정
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (isActivate && ! hasState(cstPanic)) { 
            toggleState(cstPanic);
            //setCastCode[sciMutation] = npcCode;
            updated = true;
        }
        else if (! isActivate && hasState(cstPanic)) {
            toggleState(cstPanic);
            //stateDataCodes_[sciMutation] = invalidNpcCode;
            updated = true;
        }
    }

    if (updated) {
        go::Moveable* moveable = owner_.queryMoveable();
        if (moveable != nullptr) {
            if (isActivate) {
                moveable->getMovementManager().setConfusedMovement(GameTime(duration * 1000));
            }
            else {
                moveable->getMovementManager().unsetConfusedMovement();
            }
        }
    }

    return updated;
}


bool PlayerState::hidden(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstHide)) || (! isActivate && hasState(cstHide))) {
            toggleState(cstHide);
            return true;
        }
    }
    return false;
}


bool PlayerState::isHidden() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstHide);
}


bool PlayerState::frenzied(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstFrenzie)) || (! isActivate && hasState(cstFrenzie))) {
            toggleState(cstFrenzie);
            return true;
        }
    }
    return false;
}


bool PlayerState::paralysed(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
        owner_.setMoving(false);
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstParalyse)) || (! isActivate && hasState(cstParalyse))) {
            toggleState(cstParalyse);
            return true;
        }
    }
    return false;
}


bool PlayerState::shackled(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        owner_.setMoving(false);
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstShackle)) || (! isActivate && hasState(cstShackle))) {
            toggleState(cstShackle);
            return true;
        }
    }
    return false;
}


bool PlayerState::isSpawnProtection() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstSpawnProtection);
}


bool PlayerState::isInvincible() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstInvincible);
}


bool PlayerState::spawnProtection(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstSpawnProtection)) || (! isActivate && hasState(cstSpawnProtection))) {
            toggleState(cstSpawnProtection);
            return true;
        }
    }
    return false;
}


bool PlayerState::invincible(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstInvincible)) || (! isActivate && hasState(cstInvincible))) {
            toggleState(cstInvincible);
            return true;
        }
    }
    return false;
}


bool PlayerState::knockbacked(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
        owner_.setMoving(false);
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstKnockBack)) || (! isActivate && hasState(cstKnockBack))) {
            toggleState(cstKnockBack);
            return true;
        }
    }
    return false;
}


bool PlayerState::isKnockback() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstKnockBack);
}


bool PlayerState::castingBindRecalled(bool isActivate)
{
    if (isDied()) {
        return false;
    }

    if (isActivate) {
        cancelCurrentStates();
        owner_.setMoving(false);
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstBindRecallCasting)) || (! isActivate && hasState(cstBindRecallCasting))) {
            toggleState(cstBindRecallCasting);
            return true;
        }
    }
    return false;
}


bool PlayerState::isCastingBindRecall() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstBindRecallCasting);
}


bool PlayerState::isDisableMagic() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstDisableMagic);
}


bool PlayerState::isMutated() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstMutation);
}


bool PlayerState::isSleeped() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstSleep);
}


bool PlayerState::isCrowdControl() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return creatureStateInfo_.isCrowdControl();
}

// = SkillCasterState overriding

bool PlayerState::canCastSkill(bool isMezCheck) const
{
    Arena* arena = owner_.queryArenaMatchable()->getArena();
    if (arena) {
        if (! arena->isPlaying()) {
            return false;
        }
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (hasState(cstSpawnProtection)) {
            return false;
        }

        if (isMezCheck) {            
            if (hasState(cstStun) || hasState(cstSleep) || hasState(cstParalyse) || 
                hasState(cstMutation) || hasState(cstShackle)) {
                return true;
            }
            go::CastCheckable* skillCastable = owner_.queryCastCheckable();
            if (skillCastable) {
                if (skillCastable->checkCastableEffectCategory(escSpeedDown) ||
                    skillCastable->checkCastableEffectCategory(escAttackImpossibe)) {
                    return true;
                }
            }
        }
        else {
            return canCastSkill();
        }
    }

    return false;
}


bool PlayerState::isActivateConcentrationSkill() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstConcentrationSkill);
}


DataCode PlayerState::getCastCode() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return characterStateInfo_.castCode_;
}

void PlayerState::releaseNotSkillCastingByAttack()
{
	if (hasState(cstCasting)) {
		const CodeType ct = getCodeType(creatureStateInfo_.castCode_);
		if (isHarvestType(ct) || isTreasureType(ct) || isGliderType(ct) || isVehicleType(ct) ||
            isCastStateType(ct)) {            
			owner_.queryCastable()->cancelCasting();
			notifyCancelCasting();
		}
	}
}


bool PlayerState::canCasting() const
{
    return canCastSkill();
}


bool PlayerState::isCastingAndNotMovingCast() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstCasting) && ! isMovingCast_;
}


bool PlayerState::magicDisabled(bool isActivate)
{
    if (hasState(cstDead)) {
        return false;
    }
    
    if (hasState(cstCasting)) {
        go::Castable* castable = owner_.queryCastable();
        if (castable) {
            castable->cancelCasting();
        }		
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if ((isActivate && ! hasState(cstDisableMagic)) || (! isActivate && hasState(cstDisableMagic))) {
            toggleState(cstDisableMagic);
            return true;
        }
    }
    return false;
}



void PlayerState::castStarted(DataCode dataCode, bool isMovingCast)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! canCastSkill()) {
        return;
    }	

    if (! hasState(cstCasting)) {
        toggleState(cstCasting);
        setCastCode(dataCode);
        isMovingCast_ = isMovingCast;
    }
}


void PlayerState::castReleased(bool isCancelRelease)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (isCancelRelease) {
        notifyCancelCasting();
    }
    if (hasState(cstCasting)) {
        toggleState(cstCasting);
		setCastCode(invalidDataCode);
    }

    if (hasState(cstConcentrationSkill)) {        
		toggleState(cstConcentrationSkill);
		setCastCode(invalidDataCode);
    }

    isMovingCast_ = true;
}


void PlayerState::concentrationSkillActivated(SkillCode skillCode, bool isMovingCast)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! hasState(cstConcentrationSkill)) {
        toggleState(cstConcentrationSkill);
		setCastCode(skillCode);
        isMovingCast_ = isMovingCast;
    }
}


void PlayerState::concentrationSkillReleased()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstConcentrationSkill)) {
        toggleState(cstConcentrationSkill);
        setCastCode(invalidDataCode);
        isMovingCast_ = true;
    }
}

// = ItemManageState overriding

bool PlayerState::canInventoryChangeState() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead) || hasState(cstTrade)) {
        return false;
    }

    if (isInventoryLock()) {
        return false;
    }
	
	return true;
}


bool PlayerState::canEquipOrUnEquipState() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead) || hasState(cstTrade) || hasState(cstCombat) || 
		hasState(cstCasting) || hasState(cstConcentrationSkill)) {
        return false;
    }

    if (isInventoryLock()) {
        return false;
    }

	return true;
}


bool PlayerState::canItemUseableState() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead) || hasState(cstTrade) || hasState(cstVehicleMount)) {
        return true;
    }

    if (isInventoryLock()) {
        return false;
    }

	return true;
}

// = TradeState overriding

bool PlayerState::canRequestTrade() const
{
    Arena* arena = owner_.queryArenaMatchable()->getArena();
    if (arena) {
        return false;
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (isInventoryLock()) {
            return false;
        }
        CreatureStateInfo state = removeChaoAndWalkState(creatureStateInfo_);
        state = removeVehicleMount(state);
        if (cstNormal == state.states_ || cstCombat == state.states_) {
            return true;
        }
    }
    return false;
}


bool PlayerState::canStartTrade() const
{
    Arena* arena = owner_.queryArenaMatchable()->getArena();
    if (arena) {
        return false;
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (isInventoryLock()) {
            return false;
        }
        CreatureStateInfo state = removeChaoAndWalkState(creatureStateInfo_);
        if (cstNormal == state.states_ || cstCombat == state.states_) {
            return true;
        }
    }

	return false;
}


bool PlayerState::isTradeRequstState() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstRequestTrade);
}


void PlayerState::tradeRequest()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return;
    }
    
    if (! hasState(cstRequestTrade)) {
        toggleState(cstRequestTrade);
    }
}


void PlayerState::tradeAccepted(TradeId tradeId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return;
    }

    if (hasState(cstRequestTrade)) {
        toggleState(cstRequestTrade);
    }

    if (! hasState(cstTrade)) {
        toggleState(cstTrade);
        stateIds_[siiTrade] = tradeId;
    }
}


void PlayerState::tradeCancelled(TradeId tradeId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstRequestTrade)) {
        toggleState(cstRequestTrade);
    }
    else if (hasState(cstTrade)) {
        if (stateIds_[siiTrade] != tradeId) {
            assert(false);
        }
        toggleState(cstTrade);
        stateIds_[siiTrade] = invalidObjectId;
    }	
}


void PlayerState::tradeCompleted(TradeId tradeId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstTrade)) {
        if (stateIds_[siiTrade] != tradeId) {
            assert(false);
        }
        toggleState(cstTrade);
        stateIds_[siiTrade] = invalidObjectId;
    }
}

// = HarvestState overriding

bool PlayerState::canHarvest() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (isInventoryLock()) {
        return false;
    }
	return canHarvest_i();
}


bool PlayerState::isHarvesting() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return getCodeType(getCastCode()) == ctHarvest;
}

// = TreasureState overriding

bool PlayerState::canTreasureOpen() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (isInventoryLock()) {
        return false;
    }
    return canTreasureOpen_i();
}


bool PlayerState::isTreasureOpening() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return getCodeType(getCastCode()) == ctTreasure;
}


// = GliderState overriding
bool PlayerState::canReadyMonutGlider() const
{
    Arena* arena = owner_.queryArenaMatchable()->getArena();
    if (arena) {
        if (! arena->canMountVehicle()) {
            return false;
        }
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        CreatureStateInfo state = removeChaoAndWalkState(creatureStateInfo_);

        if (cstNormal == state.states_) {
            return true;
        }
    }

    return false;
}


bool PlayerState::isMonutGliding() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstGliderMount);
}


void PlayerState::mountGlider(GliderCode code)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! hasState(cstGliderMount)) {
        toggleState(cstGliderMount);
		gliderCode_ = code; 
		gliderMountTime_ = getTime();
    }    
}


void PlayerState::dismountGlider()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstGliderMount)) {
        toggleState(cstGliderMount);
		gliderCode_ = invalidDataCode;
		const sec_t now = getTime();
		if (now <= gliderMountTime_) {
			return;
		}
		const uint32_t value = uint32_t(now - gliderMountTime_);
		owner_.queryGliderable()->decreaseGliderDurability(value);
    }
}

// = VehicleState overriding

bool PlayerState::canMountVehicle() const
{
    Arena* arena = owner_.queryArenaMatchable()->getArena();
    if (arena) {
        if (! arena->canMountVehicle()) {
            return false;
        }
    }

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        CreatureStateInfo state = removeChaoAndWalkState(creatureStateInfo_);
        if (cstNormal == state.states_) {
            return true;
        }
    }
    return false;
}


bool PlayerState::canDismountVehicle() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstVehicleMount)) {
        return true;
    }

    return false;
}


void PlayerState::mountVehicle(VehicleCode code, HarnessCode harnessCode)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! hasState(cstVehicleMount)) {
        toggleState(cstVehicleMount);
		vehicleInfo_.vehicleCode_ = code;
		vehicleInfo_.harnessCode_ = harnessCode;
    }
}


void PlayerState::dismountVehicle()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstVehicleMount)) {
        toggleState(cstVehicleMount);
		vehicleInfo_.reset();
    }
}

// = ChaoState overriding

bool PlayerState::changeChaoState(bool isChaoState)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if ((isChaoState && ! hasState(cstChao)) || (! isChaoState && hasState(cstChao))) {
        toggleState(cstChao);
        return true;
    }
    return false;
}


bool PlayerState::changeTempChaoState(bool isChaoState)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if ((isChaoState && ! hasState(cstTempChao)) || (! isChaoState && hasState(cstTempChao))) {
        toggleState(cstTempChao);
        return true;
    }
    return false;
}

// = CombatState overriding

bool PlayerState::isCombating() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstCombat);
}


bool PlayerState::isEvading() const
{
    return false;

    //std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    //return hasState(cstEvading);
}


bool PlayerState::isFleeing() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstCombat);
}


bool PlayerState::isFieldDueling() const 
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstFieldDuel);
}


bool PlayerState::isMyDueler(ObjectId fieldDuelId) const 
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return stateIds_[siiFieldDuel] == fieldDuelId;
}


ObjectId PlayerState::getFieldDuelId() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return stateIds_[siiFieldDuel];
}


bool PlayerState::startFieldDuel(ObjectId fieldDuelId)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! creatureStateInfo_.hasState(cstFieldDuel)) {
        toggleState(cstFieldDuel);
        stateIds_[siiFieldDuel] = fieldDuelId;
        return true;
    }
    return false;
}


bool PlayerState::stopFieldDuel()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (creatureStateInfo_.hasState(cstFieldDuel)) {
        toggleState(cstFieldDuel);
        stateIds_[siiFieldDuel] = invalidObjectId;
        return true;
    }
    return false;
}


void PlayerState::cancelMoveCasting()
{
    if (isMovingCast_) {
        return;
    }
    cancelCasting();
}


void PlayerState::cancelCasting()
{
    go::Castable* castable = owner_.queryCastable();
    if (! castable) {
        return;
    }
	
	if (castable->isCasting() || hasState(cstConcentrationSkill)) {
		castable->cancelCasting();
	}
}


void PlayerState::notifyCancelCasting()
{
    go::CastNotificationable* notificationable = owner_.queryCastNotificationable();
    if (! notificationable) {
        return;
    }

    if (hasState(cstCasting)) {
        notificationable->notifyCancelCasting(CancelCastResultInfo(characterStateInfo_.castCode_, owner_.getGameObjectInfo()));
    }

    if (hasState(cstConcentrationSkill)) {
        go::SkillCastable* skillCastable = owner_.querySkillCastable();
        if (skillCastable) {
            skillCastable->cancelConcentrationSkill(characterStateInfo_.castCode_);
        }
    }
}

}} // namespace gideon { namespace zoneserver {
