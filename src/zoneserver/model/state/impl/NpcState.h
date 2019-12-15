#pragma once

#include "../MoveState.h"
#include "../CreatureState.h"
#include "../SkillCasterState.h"
#include "../CombatState.h"
#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/CreatureStates.h>

namespace gideon { namespace zoneserver { namespace go {
class Npc;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

/** 
 * @class NpcState
 **/
class NpcState :
	public MoveState,
	public CreatureState,
	public SkillCasterState,
    public CombatState
{
public:
	NpcState(go::Npc& owner, CreatureStateInfo& creatureStateInfo);
	~NpcState();

public:
    void resetState();
    void resetStateExceptSpawnProtection();

    bool combat();
    bool peace();
    bool evading(bool isOn);
    bool fleeing(bool isOn);

public:
	// = MoveState overriding
	virtual bool isRooted(const Position* position) const;
    virtual bool isRunning() const;
    virtual bool isControlAbsorbed() const;
    virtual bool isInLava() const {
        return false;
    }

    virtual HitPoint getFallDamage() const {
        return hpMin;
    }

    virtual void moved(bool turn = false);
    virtual void run();
    virtual void walked();
    virtual void fallen() {}
    virtual void landed() {}
    virtual void environmentEntered(EnvironmentType /*type*/) {}
    virtual void environmentLeft() {}
public:
	// = CreatureState overriding
    virtual bool isDied() const;
	virtual void died();
	virtual void revived();
	virtual bool stunned(bool isActivate);
    virtual bool slept(bool isActivate);
    virtual bool mutated(NpcCode npcCode, sec_t duration, bool isActivate);
    virtual bool transformed(NpcCode npcCode, bool isActivate);
    virtual bool panicked(sec_t duration, bool isActivate);
    virtual bool hidden(bool isActivate);
    virtual bool isHidden() const;
    virtual bool frenzied(bool isActivate);
    virtual bool paralysed(bool isActivate);
    virtual bool shackled(bool isActivate);
    virtual bool isSpawnProtection() const;
    virtual bool isInvincible() const;
    virtual bool invincible(bool isActivate);
    virtual bool spawnProtection(bool isActivate);
    virtual bool knockbacked(bool isActivate);
    virtual bool isKnockback() const;
    virtual bool castingBindRecalled(bool /*isActivate*/) {return false;}
    virtual bool isCastingBindRecall() const {return false;}
    virtual bool isDisableMagic() const; 
    virtual bool isMutated() const;
    virtual bool isSleeped() const;
    virtual bool canApplyThreat() const;
    virtual bool isCrowdControl() const;

public:
    // = SkillCasterState overriding	
    virtual bool canCasting() const;
    virtual bool isCastingAndNotMovingCast() const;
	virtual DataCode getCastCode() const { return creatureStateInfo_.castCode_; }
	virtual void releaseNotSkillCastingByAttack() {}
	virtual void castStarted(DataCode dataCode, bool isMovingCast);
	virtual void castReleased(bool isCancelRelease);

    virtual bool canCastSkill(bool isMezCheck) const;
    virtual bool magicDisabled(bool isActivate);

    virtual void concentrationSkillActivated(SkillCode skillCode, bool isMovingCast);
	virtual void concentrationSkillReleased();
    virtual bool isActivateConcentrationSkill() const;

public:
    // = CombatState overriding
    virtual bool isCombating() const;
    virtual bool isEvading() const;
    virtual bool isFleeing() const;

private:
    void toggleState(CreatureStateType state) {
        creatureStateInfo_.toggleState(state);
    }

    void setState(CreatureStateType state) {
        creatureStateInfo_.setState(state);
    }

    void unsetState(CreatureStateType state) {
        creatureStateInfo_.unsetState(state);
    }

private:
    bool hasState(CreatureStateType state) const {
        return creatureStateInfo_.hasState(state);
    }

    bool canCastSkill() const;

    void notifyCancelCasting();
	void cancelCasting();

	void setCastCode(DataCode dataCode) {
		creatureStateInfo_.setCastCode(dataCode);
	}

private:
	go::Npc& owner_;
	CreatureStateInfo& creatureStateInfo_;
    bool isMovingCast_;
};

}} // namespace gideon { namespace zoneserver {
