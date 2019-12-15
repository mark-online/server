#pragma once

#include "../MoveState.h"
#include "../CreatureState.h"
#include "../GliderState.h"
#include "../VehicleState.h"
#include "../TradeState.h"
#include "../ItemManageState.h"
#include "../SkillCasterState.h"
#include "../TreasureState.h"
#include "../HarvestState.h"
#include "../ChaoState.h"
#include "../CombatState.h"
#include "../FieldDuelState.h"
#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/CreatureStates.h>
#include <gideon/cs/shared/data/Time.h>

namespace gideon { namespace zoneserver { namespace go {
class Creature;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

/** 
 * @class PlayerState
 **/
class PlayerState :
	public MoveState,
	public CreatureState,
	public SkillCasterState,
	public ItemManageState,
	public TradeState,
	public HarvestState,
    public TreasureState,
    public GliderState,
    public VehicleState,
    public ChaoState,
    public CombatState,
    public FieldDuelState
{
public:
    enum StateCodeIndex {
        sciTransformation = 1,
        sciMutation = 2,
        sciCount_,
    };
    enum StateIdIndex {        
        siiTrade = 0,
        siiHavest = 1,
        siiTreasure = 2,
        siiItemCast = 3,
        siiFieldDuel = 4,
        siiCount_,
    };
    enum TimeIndex {
        tiGlider = 0,
        tiCount,
    };

    typedef std::array<DataCode, sciCount_> StateDataCodes;
    typedef std::array<ObjectId, siiCount_> StateIds;
    typedef std::array<GameTime, tiCount> StateTimes;

	PlayerState(go::Creature& owner, CreatureStateInfo& creatureStateInfo,
        CreatureStateInfo& characterStateInfo, EntityVehicleInfo& vehicleInfo,
		DataCode& gliderCode);
	~PlayerState();

    void finalize();
    
    CreatureStateType getCreatureStateType() const {
        return static_cast<CreatureStateType>(creatureStateInfo_.states_);
    }

    bool changeCombatState(bool isCombatState);

public:
	// = MoveState overriding
	virtual bool isRooted(const Position* position) const;
    virtual bool isRunning() const;
    virtual bool isControlAbsorbed() const;
    virtual bool isInLava() const;

    virtual HitPoint getFallDamage() const;

	virtual void moved(bool turn = false);
    virtual void run();
    virtual void walked();
    virtual void fallen();
    virtual void landed();
    virtual void environmentEntered(EnvironmentType type);
    virtual void environmentLeft();
    
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
    virtual bool spawnProtection(bool isActivate);
    virtual bool invincible(bool isActivate);
    virtual bool knockbacked(bool isActivate);
    virtual bool isKnockback() const;
    virtual bool castingBindRecalled(bool isActivate);
    virtual bool isCastingBindRecall() const;
    virtual bool isDisableMagic() const;
    virtual bool isMutated() const;
    virtual bool isSleeped() const;
    virtual bool canApplyThreat() const {return false;}
    virtual bool isCrowdControl() const;

	// = SkillCasterState overriding
    virtual bool canCastSkill(bool isMezCheck) const;
    virtual bool magicDisabled(bool isActivate);
    virtual bool isActivateConcentrationSkill() const;

	virtual void releaseNotSkillCastingByAttack();

    virtual bool canCasting() const;
    virtual bool isCastingAndNotMovingCast() const;
	virtual DataCode getCastCode() const;

	virtual void castStarted(DataCode dataCode, bool isMovingCast);
    virtual void castReleased(bool isCancelRelease);
    virtual void concentrationSkillActivated(SkillCode skillCode, bool isMovingCast);
    virtual void concentrationSkillReleased();

	// = ItemManageState overriding
	virtual bool canInventoryChangeState() const;
	virtual bool canEquipOrUnEquipState() const;
	virtual bool canItemUseableState() const;
    virtual bool isInventoryLock() const {
        return isInventoryLock_;
    }

    virtual void lockLnventory() {
        isInventoryLock_ = true;
    }
    virtual void unlockLnventory() {
        isInventoryLock_ = false;
    }

	// = TradeState overriding
    virtual bool canRequestTrade() const;
	virtual bool canStartTrade() const;
    virtual bool isTradeRequstState() const;
	virtual TradeId getTradeId() const {
		return TradeId(stateIds_[siiTrade]);
	}

    virtual void tradeRequest();
	virtual void tradeAccepted(TradeId tradeId);
	virtual void tradeCancelled(TradeId tradeId);
	virtual void tradeCompleted(TradeId tradeId);

	// = HarvestState overriding
	virtual bool canHarvest() const;
    virtual bool isHarvesting() const;

	// = TreasureState overriding
    virtual bool canTreasureOpen() const;
    virtual bool isTreasureOpening() const;

    // = GliderState overriding
	virtual bool isMonutGliding() const;
    virtual bool canReadyMonutGlider() const;
    virtual void mountGlider(GliderCode code);
    virtual void dismountGlider();
	virtual bool isGliderMounting() const {
		return hasState(cstGliderMount);
	}

    // = VehicleState overriding
    virtual bool canMountVehicle() const;
    virtual bool canDismountVehicle() const;
    virtual void mountVehicle(VehicleCode code, HarnessCode harnessCode);
    virtual void dismountVehicle();
	virtual bool isVehicleMounting() const {
		return hasState(cstVehicleMount);
	}

    // = ChaoState overriding
    virtual bool changeChaoState(bool isChaoState);
    virtual bool changeTempChaoState(bool isChaoState);

    // = CombatState overriding
    virtual bool isCombating() const;
    virtual bool isEvading() const;
    virtual bool isFleeing() const;

    // = FieldDuelState overriding
    virtual bool isFieldDueling() const;
    virtual bool isMyDueler(ObjectId fieldDuelId) const;
    virtual ObjectId getFieldDuelId() const;

    virtual bool startFieldDuel(ObjectId fieldDuelId);
    virtual bool stopFieldDuel();

private:
    bool canCastSkill() const;
    bool canHarvest_i() const;
    bool canTreasureOpen_i() const;

    void cancelTrade();
    void cancelCurrentStates();

private:
    void cancelMoveCasting();
    void cancelCasting();
    void notifyCancelCasting();

private:
    bool hasState(CreatureStateType state) const {
        return creatureStateInfo_.hasState(state);
    }
    void toggleState(CreatureStateType state) {
        creatureStateInfo_.toggleState(state);
        characterStateInfo_.toggleState(state);
    }    
	void setCastCode(DataCode dataCode) {
		creatureStateInfo_.setCastCode(dataCode);
		characterStateInfo_.setCastCode(dataCode);
	}

private:
	go::Creature& owner_;
	CreatureStateInfo& creatureStateInfo_;
    CreatureStateInfo& characterStateInfo_;
	EntityVehicleInfo& vehicleInfo_;
    StateDataCodes stateCodes_;
	DataCode& gliderCode_;
	StateIds stateIds_;
    StateTimes stateTimes_;
    bool isMovingCast_;
	sec_t gliderMountTime_;
    float32_t lastFallHeight_;
    EnvironmentType environmentType_; 
    bool isInventoryLock_;
};

}} // namespace gideon { namespace zoneserver {