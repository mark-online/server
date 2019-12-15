#pragma once

#include "Creature.h"
#include "ability/Thinkable.h"
#include "ability/SkillCastable.h"
#include "ability/Dialogable.h"
#include "ability/Formable.h"
#include "ability/CastCheckable.h"
#include "ability/Bankable.h"
#include "ability/BuildingGuardable.h"
#include "ability/WorldEventable.h"
#include "ability/Marchable.h"
#include "ability/Invadable.h"
#include "ability/Craftable.h"
#include "ability/TargetSelectable.h"
#include "ability/WeaponUseable.h"
#include "../time/GlobalCoolDownTimer.h"
#include <gideon/cs/shared/data/UnionEntityInfo.h>
#include <gideon/cs/shared/data/EntityPathInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace gideon { namespace datatable {
class NpcTemplate;
class NpcTalkingList;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

class NpcState;
class NpcSkillList;
class NpcFormation;

namespace gc {
class NpcController;
class NpcMoveController;
} // namespace gc {

namespace ai {
class NpcStateBrain;
} // namespace ai {

}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

class SkillCastableAbility;
class PassiveSkillManager;

/**
 * @class Npc
 * Non-Entity Character
 */
class ZoneServer_Export Npc : public Creature,
    private Thinkable,
    private SkillCastable,
    private Dialogable,
    private Formable,
	private CastCheckable,
    private Bankable,
    private BuildingGuardable,
    private WorldEventable,
    private Marchable,
    private Invadable,
    private Craftable,
    private TargetSelectable,
    private WeaponUseable
{
    typedef Creature Parent;

public:
    Npc(std::unique_ptr<gc::EntityController> controller);
    virtual ~Npc();

    bool initialize(ObjectId npcId, const datatable::NpcTemplate& npcTemplate);

protected:
    virtual void finalize();

    virtual ErrorCode spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition);
    virtual ErrorCode despawn();

protected:
    virtual void frenze(int32_t size, bool isRelease);

    virtual void initState(CreatureStateInfo& stateInfo);

protected:
    virtual ObjectPosition getNextSpawnPosition() const;

private:
    virtual DataCode getEntityCode() const {
        return npcInfo_.npcCode_;
    }

    virtual float32_t getCurrentDefaultSpeed() const;

    virtual CreatureLevel getCreatureLevel() const {
        return npcInfo_.level_;
    }
    virtual float32_t getModelingRadiusSize() const;

private:
    virtual bips_t getMissChance(const Entity& target) const;
    virtual bips_t getDodgeChance() const;
    virtual bips_t getParryChance() const;
    virtual bips_t getBlockChance() const;
    virtual bips_t getPhysicalCriticalChance() const;
    virtual bips_t getMagicCriticalChance() const;

public:
    void talkTo(go::Entity& entity);

public:
    void setCombatState();
    void setPeaceState();

    void setEvadeState();
    void unsetEvadeState();

    void setFleeState();
    void unsetFleeState();

    void resetState();

public:
    bool isValid() const {
        return Parent::isValid() && npcInfo_.isValid();
    }

    bool canBuy() const;
    bool canSell() const;
    bool hasMail() const;
    bool hasBank() const;
    bool hasAuction() const;
    bool isStaticNpc() const;

    bool shouldActiveBrain() const {
        return true; // hasPath();
    }

public:
	const datatable::NpcTemplate& getNpcTemplate() const {
		return *npcTemplate_;
	}

    const datatable::NpcTalkingList* getNpcTalkingList() const {
        return npcTalkingList_;
    }

    const NpcSkillList& getSkillList() const {
        return *skillList_;
    }

	go::Entity* getTopScorePlayer() const;

    gc::NpcMoveController& getNpcMoveController() {
        return reinterpret_cast<gc::NpcMoveController&>(getMoveController());
    }

    ExpPoint getRewardExp(CreatureLevel rewarderLevel) const;

private:
    void resetForFinalize();
    void resetForRespawn();

    void restoreSpeedAndScale();
    void restoreSpeedAndScale_i();

    bool setTarget(GameObjectInfo& oldTargetInfo, bool& isAggressive,
        const go::Entity& target, const GameObjectInfo& targetInfo);

private:
    bool isWanderable() const;

private:
    // = Entity overrding
    virtual std::unique_ptr<EffectHelper> createEffectHelper();

public:
    // = EntityAbility overriding
    virtual Thinkable* queryThinkable() { return this; }
    virtual SkillCastable* querySkillCastable() { return this; }
    virtual Dialogable* queryDialogable() { return this; }
    virtual Formable* queryFormable() { return this; }
	virtual CastCheckable* queryCastCheckable() { return this;}
    virtual Bankable* queryBankable() { return this; }
    virtual BuildingGuardable* queryBuildingGuardable() { return this; }
    virtual WorldEventable* queryWorldEventable() { return this; }
    virtual Marchable* queryMarchable() { return this; }
    virtual Invadable* queryInvadable() { return this; }
    virtual const Craftable* queryCraftable() const { return this; }
    virtual Craftable* queryCraftable() { return this; }
    virtual TargetSelectable* queryTargetSelectable() { return this; }
    virtual const WeaponUseable* queryWeaponUseable() const { return this; }

public: // = EntityStateAbility overriding
    virtual MoveState* queryMoveState();
    virtual CreatureState* queryCreatureState();
    virtual SkillCasterState* querySkillCasterState();
    virtual CombatState* queryCombatState();
    virtual CastState* queryCastState();

private:
    virtual ErrorCode canBankable(go::Entity& player) const;

private: // = Positionable overriding
    virtual void setPosition(const ObjectPosition& position);
    virtual void setHeading(Heading heading);

private: // = Moveable overriding
    virtual std::unique_ptr<gc::MoveController> createMoveController();
    virtual void reserveReleaseDownSpeed() {
        shouldReleaseDownSpeedScript_ = true;
    }
    virtual void reserveReleaseUpSpeed() {
        shouldReleaseUpSpeedScript_ = true;
    }
    virtual float32_t getMaxMoveDistance() const {
        return maxMoveDistance_;
    }

private: // = Liveable overriding
    virtual std::unique_ptr<CreatureStatus> createCreatureStatus();
	virtual ErrorCode revive(bool skipTimeCheck = false);
    virtual bool reviveByEffect(HitPoint& refillPoint, permil_t perRefillHp);

private: // = Thinkable overriding
    virtual bool hasWalkRoutes() const;
    virtual ai::Brain& getBrain() {
        return *brain_;
    }

private: // = Tickable overriding
    virtual void tick(GameTime diff);

private: // = SkillCastable overriding
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, SkillCode skillCode);
    virtual ErrorCode castAt(const Position& targetPosition, SkillCode skillCode);
    virtual void cancel(SkillCode skillCode);
    virtual void cancelConcentrationSkill(SkillCode /*skillCode*/) {}
    virtual void cancelAll();
    virtual void consumePoints(const Points& points);
	virtual void consumeMaterialItem(const BaseItemInfo& itemInfo);
    virtual void setNextGlobalCooldownTime(uint32_t index, msec_t nextCooldownTime);
    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const GameObjectInfo& targetInfo) const;
    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const Position& targetPosition) const;

    virtual bool checkCastableNeedSkill(SkillCode needSkillCode) const;
    virtual bool checkCastableUsableState(SkillUseableState needUsableState) const;
    virtual bool checkCastableEquip(EquipPart checekEquipPart, SkillCastableEquipType checkAllowedType) const;
	virtual bool checkCastableNeedItem(const BaseItemInfo& needItem) const;
	virtual bool checkCastableEffectCategory(EffectStackCategory category) const;
    virtual ErrorCode checkCastablePoints(PointType pt, bool isPercent,
        bool isCheckUp, uint32_t checkValue) const;

	virtual void notifyChangeCondition(PassiveCheckCondition condition);
    virtual PassiveSkillManager& getPassiveSkillManager() {
        assert(false);
        return *passiveSkillManager_.get();
    }

    virtual msec_t getNextGlobalCooldownTime(uint32_t index) const;
    virtual float32_t getLongestSkillDistance() const;
    virtual bool isUsing(SkillCode skillCode) const;
    virtual bool canCast(SkillCode skillCode) const;

private: // = Skillable overriding
	virtual std::unique_ptr<gc::SkillController> createSkillController();

private: // = Effectable overriding
    virtual std::unique_ptr<gc::CreatureEffectController> createEffectController();

private: // = Dialogable overriding
    virtual void openDialog(const Entity& requester);
    virtual void closeDialog(const GameObjectInfo& requester);

private: // = Formable overriding
    virtual void formUp();
    virtual void setFormation(NpcFormationRefPtr formation);
    virtual bool hasForm() const;
    virtual NpcFormationRefPtr getFormation() {
        return formation_;
    }

private: // = BuildingGuardable overriding
    virtual void setMercenaryId(ObjectId mercenaryId) {
        mercenaryId_ = mercenaryId;
    }
    virtual ObjectId getMercenaryId() const {
        return mercenaryId_;
    }

private: // = Marchable overriding
    virtual void marchStarted();
    virtual void marchStopped();
    virtual bool shouldMarch() const;
    virtual bool isMarching() const {
        return isMarching_;
    }

private: // = WorldEventable overriding
    virtual void notifyMovemoent(const Position& position, bool isStop);

private: // = Invadable overriding
    virtual bool isInvader() const;

private: // = Summonable overriding
    virtual void setSummoner(go::Entity& summoner, SpawnType spawnType);
    virtual void resetSummoner();

private: // = Craftable overriding
    virtual bool hasCraftFunction(CraftType craftType) const;

private: // = TargetSelectable overriding
    virtual const go::Entity* selectTarget(const GameObjectInfo& targetInfo);
    virtual void unselectTarget();
    virtual go::Entity* getSelectedTarget();
    virtual const GameObjectInfo& getSelectedTargetInfo() const;

private: // = WeaponUseable overriding
    virtual bool isMeleeWeaponEquipped() const {
        return isMeleeWeaponEquipped_;
    }
    virtual bool isShieldEquipped() const {
        return isShieldEquipped_;
    }

private:
    const datatable::NpcTemplate* npcTemplate_;
    const datatable::NpcTalkingList* npcTalkingList_;

    FullNpcInfo npcInfo_;
    GameObjectInfo targetInfo_;
    GlobalCoolDownTimer globalGoolDownTimer_;

    std::unique_ptr<SkillCastableAbility> skillCastableAbility_;
    std::unique_ptr<PassiveSkillManager> passiveSkillManager_;
    std::unique_ptr<NpcSkillList> skillList_;
    std::unique_ptr<ai::Brain> brain_;
	std::unique_ptr<NpcState> npcState_;
    NpcFormationRefPtr formation_;
    ObjectId mercenaryId_;

    bool isMarching_;
    uint32_t notifyMovementCount_;

    bool shouldReleaseDownSpeedScript_;
    bool shouldReleaseUpSpeedScript_;

    float32_t maxMoveDistance_;

    bool isMeleeWeaponEquipped_;
    bool isShieldEquipped_;
    bool isInRandomDungion_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
