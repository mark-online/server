#pragma once

#include "Entity.h"
#include "ability/Knowable.h"
#include "ability/Moveable.h"
#include "ability/Liveable.h"
#include "ability/Skillable.h"
#include "ability/Effectable.h"
#include "ability/CastGameTimeable.h"
#include "ability/CreatureStatusable.h"
#include "ability/PassiveSkillCastable.h"
#include "ability/Tickable.h"
#include "status/CreatureStatus.h"
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/CreatureStates.h>
#include <gideon/cs/shared/data/FactionInfo.h>

namespace gideon { namespace zoneserver { 
class GlobalCoolDownTimer;
class CoolDownTimer;
class MovementManager;
}} // namespace gideon { namespace zoneserver { 

namespace gideon { namespace zoneserver { namespace gc {
class MoveController;
class SkillController;
class CreatureEffectController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

class EffectScriptApplier;
class CreatureStatus;
class AggroSendable;

/**
 * @class Creature
 *
 */
class ZoneServer_Export Creature : public Entity,
    public CreatureStatusable,
    public PassiveSkillCastable,
    protected Skillable,
    protected Effectable,
    protected Moveable,
    protected Liveable,
    protected CastGameTimeable,
    private Tickable
{
    typedef Entity Parent;

public:
    Creature(std::unique_ptr<gc::EntityController> controller);
    virtual ~Creature();

    bool initialize(ObjectType objectType, ObjectId objectId, FactionCode factionCode);

public:
    uint32_t downCastingDelay(uint32_t delayTime);
    void upCastingDelay(uint32_t delayTime);

public:
    
    float32_t getCurrentScale() const;
    uint32_t getCastingDelay() const;

public:
    virtual void frenze(int32_t size, bool isRelease) = 0;

    virtual CreatureLevel getCreatureLevel() const = 0;
    virtual const DebuffBuffEffectInfoSet getDebuffBuffEffectInfoSet() const;

protected:
    virtual void finalize();

    virtual void initState(CreatureStateInfo& stateInfo) = 0;

    virtual std::unique_ptr<EffectScriptApplier> createEffectScriptApplier();

protected:
    virtual void setCooldown(DataCode dataCode, GameTime coolTime,
        uint32_t index, GameTime globalCoolDownTime);
    virtual void cancelCooldown(DataCode dataCode, uint32_t index);
    virtual void cancelPreCooldown();
    virtual bool isGlobalCooldown(uint32_t index) const;
    virtual bool isLocalCooldown(DataCode dataCode) const;

    virtual float32_t getCurrentDefaultSpeed() const = 0;

protected:
    virtual bips_t getLevelBonus(const Entity& target) const;
    virtual bips_t getMissChance(const Entity& target) const;
    virtual bips_t getResistChance(AttributeRateType attributeType) const;

public:
    // = EntityAbility overriding
    virtual Knowable* queryKnowable() { return knowable_.get(); }
    virtual Moveable* queryMoveable() { return this; }
    virtual Liveable* queryLiveable() { return this; }    
    virtual Tickable* queryTickable() { return this; }
    virtual PassiveSkillCastable* queryPassiveSkillCastable() { return this; }    
    virtual const CreatureStatusable* queryCreatureStatusable() const { return this; }
    virtual Skillable* querySkillable() { return this; }
    virtual CastGameTimeable* queryCastGameTimeable() { return this; }
    virtual Factionable* queryFactionable() { return factionable_.get(); }
    virtual const Factionable* queryFactionable() const { return factionable_.get(); }
    virtual Castable* queryCastable() { return castable_.get(); }
    virtual CastNotificationable* queryCastNotificationable() {
        return castNotificationable_.get();
    }
    virtual AggroSendable* queryAggroSendable() {
        return aggroSendable_.get();
    }
    
public:
    // = Positionable overriding
    virtual void setWorldMap(WorldMap& worldMap);
    virtual WorldPosition getWorldPosition() const;

protected:
    // = CreatureStatusable overriding
    virtual const CreatureStatusInfo& getCurrentCreatureStatusInfo() const {
        return creatureStatus_->getCurrentCreatureStatusInfo();
    }
    virtual const CreatureStatusInfo& getCreatureStatusInfo() const {
        return creatureStatus_->getCreatureStatusInfo();
    }

    virtual CreatureStatusInfo& getCurrentCreatureStatusInfo() {
        return creatureStatus_->getCurrentCreatureStatusInfo();
    }

    virtual CreatureStatusInfo& getCreatureStatusInfo() {
        return creatureStatus_->getCreatureStatusInfo();
    }

protected:
    CoolDownTimer& getCoolDownTimer() {
        return *coolTime_.get();
    }
    
    void initCastingDelay() {
        castingDelay_ = defaultSkillDelayPercent;
    }

    // 쿨타임 걸려있는걸 전부 초기화 한다.
    void resetCooldowns();

public:
    // = Moveable overriding
    virtual MovementManager& getMovementManager() {
        return *movementManager_;
    }

    virtual void setDestination(const ObjectPosition& destin);
    virtual void setMoving(bool isMoving);

    virtual ObjectPosition getDestination() const;
    virtual bool isMoving() const;

    virtual void setSpeedRate(permil_t totalSpeedRate);
    virtual void resetSpeedRate();
    virtual void updateSpeed();

    virtual float32_t getSpeed() const;

    virtual float32_t getMaxMoveDistance() const {
        return 0.0f;
    }

    virtual gc::MoveController& getMoveController() {
        return *moveController_;
    }

    virtual const gc::MoveController& getMoveController() const {
        return *moveController_;
    }

public:
    // = Skillable overriding	
    virtual gc::SkillController& getSkillController() {
        return *skillController_;
    }

    virtual const gc::SkillController& getSkillController() const {
        return *skillController_;
    }

    virtual msec_t getCalcCastingTime(msec_t castingTime) const;


public:
    // = Effectable overriding
    virtual gc::CreatureEffectController& getEffectController() {
        return *effectController_;
    }

    virtual const gc::CreatureEffectController& getEffectController() const {
        return *effectController_;
    }

public:
    // = Liveable overriding
    virtual CreatureStatus& getCreatureStatus() {
        assert(creatureStatus_.get() != nullptr);
        return *creatureStatus_;
    }

    virtual const CreatureStatus& getCreatureStatus() const {
        assert(creatureStatus_.get() != nullptr);
        return *creatureStatus_;
    }

protected:
    void resetSpeedRate_i() {
        speedRate_ = 0;
    }

private:
    bool shouldRecoverCp() const;

private:
    std::unique_ptr<Knowable> knowable_;
    std::unique_ptr<Factionable> factionable_;

    std::unique_ptr<gc::MoveController> moveController_;
    std::unique_ptr<gc::SkillController> skillController_;
    std::unique_ptr<gc::CreatureEffectController> effectController_;

    std::unique_ptr<CreatureStatus> creatureStatus_;

    std::unique_ptr<GlobalCoolDownTimer> globalCoolTime_;
    std::unique_ptr<CoolDownTimer> coolTime_;
    std::unique_ptr<Castable> castable_;
    std::unique_ptr<CastNotificationable> castNotificationable_;
    std::unique_ptr<AggroSendable> aggroSendable_;

    std::unique_ptr<MovementManager> movementManager_;

    uint32_t castingDelay_; // 시전 감속 증가 스킬
    permil_t speedRate_;

    mutable WorldPosition lastWorldPosition_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
