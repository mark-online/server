#include "ZoneServerPCH.h"
#include "NpcState.h"
#include "../../gameobject/Npc.h"
#include "../../gameobject/ability/Castable.h"
#include "../../gameobject/ability/Castnotificationable.h"
#include "../../../service/movement/MovementManager.h"
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver {

NpcState::NpcState(go::Npc& owner, CreatureStateInfo& creatureStateInfo) :
	owner_(owner),
	creatureStateInfo_(creatureStateInfo),
    isMovingCast_(true)
{
}


NpcState::~NpcState()
{

}


void NpcState::resetState()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    creatureStateInfo_.reset();
}


void NpcState::resetStateExceptSpawnProtection()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    creatureStateInfo_.reset();
    spawnProtection(true);
}


bool NpcState::combat()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstCombat)) {
        return false;
    }

    setState(cstCombat);
    return true;
}


bool NpcState::peace()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! hasState(cstCombat)) {
        return false;
    }

    unsetState(cstCombat);
    return true;
}


bool NpcState::evading(bool isOn)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (isOn) {
        if (hasState(cstEvading)) {
            return false;
        }
        unsetState(cstCombat);
        setState(cstEvading);
    }
    else {
        if (! hasState(cstEvading)) {
            return false;
        }
        unsetState(cstEvading);
    }
    return true;
}


bool NpcState::fleeing(bool isOn)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (isOn) {
        if (hasState(cstFleeing)) {
            return false;
        }
        unsetState(cstCombat);
        setState(cstFleeing);
    }
    else {
        if (! hasState(cstFleeing)) {
            return false;
        }
        unsetState(cstFleeing);
    }
    return true;
}

// = MoveState overriding

bool NpcState::isRooted(const Position* /*position*/) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead) || hasState(cstStun) || hasState(cstSleep) || hasState(cstShackle) ||
        hasState(cstParalyse) || hasState(cstKnockBack)) {
        return true;
    }

    if ((hasState(cstCasting) || hasState(cstConcentrationSkill)) && (! isMovingCast_)) {
        return true;
    }

    if (owner_.getNpcTemplate().getWalkSpeed() > 0.0f) {
        if (owner_.getSpeed() <= 0.01f) {
            return true;
        }
    }

    return false;
}


bool NpcState::isRunning() const 
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return ! hasState(cstWalk);
}


bool NpcState::isControlAbsorbed() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstMutation);
}


void NpcState::run()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstWalk)) {
        toggleState(cstWalk);
    }
}


void NpcState::walked()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! hasState(cstWalk)) {
        toggleState(cstWalk);
    }
}


void NpcState::moved(bool /*turn*/)
{
    // FYI: 이동 시 시전 취소는 AttackNpcBrainDesire에서 처리한다
}

// = CreatureState overriding

bool NpcState::isDied() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstDead);
}


void NpcState::died()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    creatureStateInfo_.reset();
    toggleState(cstDead);
}


void NpcState::revived()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        creatureStateInfo_.states_ = cstNormal;
    }
}


bool NpcState::stunned(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if ((isActivate && ! hasState(cstStun)) || (! isActivate && hasState(cstStun))) {
        toggleState(cstStun);
    }

    return true;
}


bool NpcState::slept(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if ((isActivate && ! hasState(cstSleep)) || (! isActivate && hasState(cstSleep))) {
        toggleState(cstSleep);
    }

    return true;
}


bool NpcState::mutated(NpcCode npcCode, sec_t duration, bool isActivate)
{
    npcCode, duration;

    bool updated = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (hasState(cstDead)) {
            return false;
        }

        if ((isActivate && ! hasState(cstMutation)) || (! isActivate && hasState(cstMutation))) {
            toggleState(cstMutation);
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


bool NpcState::transformed(NpcCode npcCode, bool isActivate)
{
    npcCode;

    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if ((isActivate && ! hasState(cstTransformation)) || (! isActivate && hasState(cstTransformation))) {
        toggleState(cstTransformation);
        return true;
    }

    return false;
}


bool NpcState::panicked(sec_t duration, bool isActivate)
{
    bool updated = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (hasState(cstDead)) {
            return false;
        }

        if ((isActivate && ! hasState(cstPanic)) || (! isActivate && hasState(cstPanic))) {
            toggleState(cstPanic);
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


bool NpcState::hidden(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if ((isActivate && ! hasState(cstHide)) || (! isActivate && hasState(cstHide))) {
        toggleState(cstHide);
    }

    return true;
}


bool NpcState::isHidden() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstHide);
}


bool NpcState::frenzied(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if ((isActivate && ! hasState(cstFrenzie)) || (! isActivate && hasState(cstFrenzie))) {
        toggleState(cstFrenzie);
        return true;
    }

    return false;
}


bool NpcState::paralysed(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if ((isActivate && ! hasState(cstParalyse)) || (! isActivate && hasState(cstParalyse))) {
        toggleState(cstParalyse);
        return true;
    }

    return false;
}


bool NpcState::shackled(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if ((isActivate && ! hasState(cstShackle)) || (! isActivate && hasState(cstShackle))) {
        toggleState(cstShackle);
        return true;
    }

    return false;
}


bool NpcState::isSpawnProtection() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstSpawnProtection);
}


bool NpcState::isInvincible() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstInvincible);
}


bool NpcState::spawnProtection(bool isActivate)
{
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (hasState(cstDead)) {
            return false;
        }

        if ((isActivate && ! hasState(cstSpawnProtection)) || (! isActivate && hasState(cstSpawnProtection))) {
            toggleState(cstSpawnProtection);
            return true;
        }
    }
    return false;
}


bool NpcState::invincible(bool isActivate)
{
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (hasState(cstDead)) {
            return false;
        }

        if ((isActivate && ! hasState(cstInvincible)) || (! isActivate && hasState(cstInvincible))) {
            toggleState(cstInvincible);
            return true;
        }
    }
    return false;
}


bool NpcState::knockbacked(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }
    
    if ((isActivate && ! hasState(cstKnockBack)) || (! isActivate && hasState(cstKnockBack))) {
        toggleState(cstKnockBack);
        return true;
    }

    return false;
}


bool NpcState::isKnockback() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstKnockBack);
}


bool NpcState::isDisableMagic() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstDisableMagic);
}


bool NpcState::isMutated() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstMutation);
}


bool NpcState::isSleeped() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstSleep);
}


bool NpcState::canApplyThreat() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return ! (hasState(cstSleep) || hasState(cstStun));
}


bool NpcState::isCrowdControl() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return creatureStateInfo_.isCrowdControl();
}

// = SkillCasterState overriding

bool NpcState::canCastSkill(bool isMezCheck) const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (isMezCheck) {
        if (hasState(cstStun) || hasState(cstSleep) || hasState(cstParalyse) || hasState(cstMutation)) {
            return true;
        }
    }
    else {
        return canCastSkill();
    }

    return false;
}


bool NpcState::magicDisabled(bool isActivate)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead)) {
        return false;
    }

    if (hasState(cstCasting)) {
        go::SkillCastable* skillCastable = owner_.querySkillCastable();
        if (skillCastable) {
            skillCastable->cancel(creatureStateInfo_.castCode_);
        }
    }

    if ((isActivate && ! hasState(cstDisableMagic)) || (! isActivate && hasState(cstDisableMagic))) {
        toggleState(cstDisableMagic);
        return true;
    }

    return false;
}


bool NpcState::canCasting() const
{
    if (hasState(cstDead) || hasState(cstStun) || hasState(cstSleep) ||
        hasState(cstMutation) || hasState(cstParalyse) ||
        hasState(cstKnockBack) || hasState(cstCasting)) {
        return false;
    }

    return true;
}


bool NpcState::isCastingAndNotMovingCast() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstCasting) && ! isMovingCast_;
}


void NpcState::castStarted(DataCode dataCode, bool isMovingCast)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead) || hasState(cstStun)) {
        return;
    }

    if (! hasState(cstCasting)) {
        setState(cstCasting);
        creatureStateInfo_.setCastCode(dataCode);
        isMovingCast_ = isMovingCast;
    }
}


void NpcState::castReleased(bool isCancelRelease)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead) || hasState(cstStun)) {
        return;
    }

    if (isCancelRelease) {
        // TODO: 케스팅 취소를 알린다.
        notifyCancelCasting();
    }

    if (hasState(cstCasting)) {
        unsetState(cstCasting);
		creatureStateInfo_.setCastCode(invalidDataCode);
    }

	if (hasState(cstConcentrationSkill)) {        
		toggleState(cstConcentrationSkill);
		setCastCode(invalidDataCode);
	}

    isMovingCast_ = true;
}


bool NpcState::canCastSkill() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (hasState(cstDead) || hasState(cstStun) || hasState(cstSleep) ||
        hasState(cstMutation) || hasState(cstParalyse) || 
        hasState(cstKnockBack) || hasState(cstEvading)) {
        return false;
    }

    return true;
}


void NpcState::concentrationSkillActivated(SkillCode skillCode, bool isMovingCast)
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

	if (! hasState(cstConcentrationSkill)) {
		toggleState(cstConcentrationSkill);
		setCastCode(skillCode);
		isMovingCast_ = isMovingCast;
	}

}


void NpcState::concentrationSkillReleased(){

    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

	if (hasState(cstConcentrationSkill)) {
		toggleState(cstConcentrationSkill);
		setCastCode(invalidDataCode);
		isMovingCast_ = true;
	}
}


bool NpcState::isActivateConcentrationSkill() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstConcentrationSkill);
}

// = CombatState overriding

bool NpcState::isCombating() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstCombat);
}


bool NpcState::isEvading() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstEvading);
}


bool NpcState::isFleeing() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return hasState(cstFleeing);
}


void NpcState::notifyCancelCasting()
{
    go::CastNotificationable* notificationable = owner_.queryCastNotificationable();
    if (! notificationable) {
        return;
    }

    if (hasState(cstCasting)) {
        notificationable->notifyCancelCasting(CancelCastResultInfo(creatureStateInfo_.castCode_, owner_.getGameObjectInfo()));
    }

    if (hasState(cstConcentrationSkill)) {
        go::SkillCastable* skillCastable = owner_.querySkillCastable();
        if (skillCastable) {
            skillCastable->cancelConcentrationSkill(creatureStateInfo_.castCode_);
        }
    }
}

void NpcState::cancelCasting()
{
	go::Castable* castable = owner_.queryCastable();
	if (! castable) {
		return;
	}

	if (castable->isCasting() || hasState(cstConcentrationSkill)) {
		castable->cancelCasting();
	}
}

}} // namespace gideon { namespace zoneserver {
