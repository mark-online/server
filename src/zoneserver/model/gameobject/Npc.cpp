#include "ZoneServerPCH.h"
#include "Npc.h"
#include "skilleffect/CreatureEffectScriptApplier.h"
#include "status/NpcStatus.h"
#include "ability/impl/SkillCastableAbility.h"
#include "ability/Chaoable.h"
#include "ability/AggroSendable.h"
#include "ability/Factionable.h"
#include "ability/Buildable.h"
#include "ability/Castable.h"
#include "skilleffect/PassiveSkillManager.h"
#include "../state/impl/NpcState.h"
#include "../../world/WorldMap.h"
#include "../../world/World.h"
#include "../../world/region/MapRegion.h"
#include "../../controller/NpcController.h"
#include "../../controller/npc/NpcSkillController.h"
#include "../../controller/npc/NpcEffectController.h"
#include "../../controller/npc/NpcMoveController.h"
#include "../../controller/callback/DialogCallback.h"
#include "../../controller/callback/EntityStatusCallback.h"
#include "../../controller/callback/WorldEventCallback.h"
#include "../../controller/callback/EntityEvents.h"
#include "../../service/spawn/SpawnService.h"
#include "../../service/item/GraveStoneService.h"
#include "../../service/skill/NpcSkillList.h"
#include "../../service/skill/helper/NpcEffectHelper.h"
#include "../../service/movement/MovementManager.h"
#include "../../service/formation/NpcFormation.h"
#include "../../service/distance/DistanceChecker.h"
#include "../../ai/BrainFactory.h"
#include "../../ai/Brain.h"
#include "../../ai/state/npc/NpcBrainStateDef.h"
#include "../../ai/event/npc/NpcBrainEventDef.h"
#include "../../ai/aggro/AggroList.h"
#include "../../helper/Utils.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/NpcActiveSkillTable.h>
#include <gideon/cs/datatable/NpcTalkingTable.h>
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/shared/data/EquipmentInfo.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class WorldEventNpcMovedEvent
 */
class WorldEventEntityMovedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<WorldEventEntityMovedEvent>
{
public:
    WorldEventEntityMovedEvent(const GameObjectInfo& gameObjectInfo, NpcCode npcCode, const Position& position) :
        gameObjectInfo_(gameObjectInfo),
        npcCode_(npcCode),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* worldEventCallback =
            entity.getController().queryWorldEventCallback();
        if (worldEventCallback != nullptr) {
            worldEventCallback->worldEventEntityMoved(gameObjectInfo_, npcCode_, position_);
        }
    }

private:
    GameObjectInfo gameObjectInfo_;
    DataCode npcCode_;
    Position position_;
};


/**
 * @class WorldEventEntityDespawndEvent
 */
class WorldEventEntityDespawndEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<WorldEventEntityDespawndEvent>
{
public:
    WorldEventEntityDespawndEvent(const GameObjectInfo& gameObjectInfo) :
        gameObjectInfo_(gameObjectInfo) {}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* worldEventCallback =
            entity.getController().queryWorldEventCallback();
        if (worldEventCallback != nullptr) {
            worldEventCallback->worldEventEntityDespawned(gameObjectInfo_);
        }
    }

private:
    GameObjectInfo gameObjectInfo_;
};


/**
 * @class TargetSelectedEvent
 */
class TargetSelectedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<TargetSelectedEvent>
{
public:
    TargetSelectedEvent(const GameObjectInfo& entityInfo, const GameObjectInfo& targetInfo, bool isAgressive) :
        entityInfo_(entityInfo),
        targetInfo_(targetInfo),
        isAgressive_(isAgressive) {}

private:
    virtual void call(go::Entity& entity) {
        gc::EntityStatusCallback* entityStatusCallback =
            entity.getController().queryEntityStatusCallback();
        if (entityStatusCallback != nullptr) {
            entityStatusCallback->entityTargetSelected(entityInfo_, targetInfo_, isAgressive_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const GameObjectInfo targetInfo_;
    const bool isAgressive_;
};


/**
 * @class DialogOpenedEvent
 */
class DialogOpenedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<DialogOpenedEvent>
{
public:
    DialogOpenedEvent(const GameObjectInfo& npc, const GameObjectInfo& requester) :
        npc_(npc),
        requester_(requester) {}

private:
    virtual void call(go::Entity& entity) {
        gc::DialogCallback* callback = entity.getController().queryDialogCallback();
        if (callback != nullptr) {
            callback->dialogOpened(npc_, requester_);
        }
    }

private:
    const GameObjectInfo& npc_;
    const GameObjectInfo& requester_;
};


/**
 * @class DialogClosedEvent
 */
class DialogClosedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<DialogClosedEvent>
{
public:
    DialogClosedEvent(const GameObjectInfo& npc, const GameObjectInfo& requester) :
        npc_(npc),
        requester_(requester) {}

private:
    virtual void call(go::Entity& entity) {
        gc::DialogCallback* callback = entity.getController().queryDialogCallback();
        if (callback != nullptr) {
            callback->dialogClosed(npc_, requester_);
        }
    }

private:
    const GameObjectInfo& npc_;
    const GameObjectInfo& requester_;
};


inline EquipType getEquipType(EquipCode equipCode)
{
    const gdt::equip_t* equipTemplate = EQUIP_TABLE->getEquip(equipCode);
    if (! equipTemplate != NULL) {
        return EquipType::etUnknown;
    }
    return toEquipType(equipTemplate->equip_type());
}

} // namespace

// = Npc

Npc::Npc(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller)),
    npcTemplate_(nullptr),
    npcTalkingList_(nullptr),
    maxMoveDistance_(0.0f),
    isMeleeWeaponEquipped_(false),
    isShieldEquipped_(false),
    isInRandomDungion_(false)
{
}


Npc::~Npc()
{
}


bool Npc::initialize(ObjectId npcId, const datatable::NpcTemplate& npcTemplate)
{
    assert(isValidObjectId(npcId));

    const NpcCode npcCode = npcTemplate.getInfo().npc_code();
    const ObjectType objectType = toObjectType(npcCode);
    const FactionCode factionCode = npcTemplate.getFactionCode();

    {
        std::lock_guard<LockType> lock(getLock());

        if (! Parent::initialize(objectType, npcId, factionCode)) {
            return false;
        }

        assert(! npcInfo_.isValid());

        npcTemplate_ = &npcTemplate;

        npcInfo_.objectType_ = objectType;
        npcInfo_.objectId_ = npcId;
        npcInfo_.level_ = npcTemplate_->getLevel();
        npcInfo_.npcCode_ = npcTemplate_->getNpcCode();

        CreatureStatusInfo& gameStatus = getCurrentCreatureStatusInfo();
        CreatureStatusInfo& creatureStatusInfo = getCreatureStatusInfo();
        gameStatus = npcTemplate_->getCurrentCreatureStatusInfo();
        creatureStatusInfo = npcTemplate_->getCurrentCreatureStatusInfo();
        npcInfo_.currentPoints_ = gameStatus.points_;
        shouldReleaseDownSpeedScript_ = false;
        restoreSpeedAndScale_i();
        MoreNpcInfo& moreNpcInfo = getUnionEntityInfo_i().asNpcInfo();
        static_cast<NpcInfo&>(moreNpcInfo) = npcInfo_;		

        skillList_= std::make_unique<NpcSkillList>(*this, *npcTemplate_);

        skillCastableAbility_= std::make_unique<SkillCastableAbility>(*this);
        //passiveActiveEffectInfoApplier_= std::make_unique<PassiveActiveEffectInfoApplier>(*this, getCurrentCreatureStatusInfo(), getCreatureStatusInfo());

        for (const SkillCode skillCode : skillList_->getLearnedSkills()) {            
            SkillTableType skillTableType = getSkillTableType(skillCode);
            if (sttActiveNpc == skillTableType) {
                const datatable::NpcActiveSkillTemplate* skillTemplate = NPC_ACTIVE_SKILL_TABLE->getNpcSkill(skillCode);
                if (! skillTemplate) {
                    return false;
                }
                skillCastableAbility_->learn(skillCode);
            }
            else {
                // not passive 
                assert(false);
            }
        }

        npcTalkingList_ = NPC_TALKING_TABLE->getNpcTalkingList(npcInfo_.npcCode_);

        brain_.reset(ai::BrainFactory::createBrain(*this).release());
        brain_->initialize();

        mercenaryId_ = invalidObjectId;

        maxMoveDistance_ = npcTemplate_->getMaxMoveDistance();

        isMarching_ = false;
    }

    isMeleeWeaponEquipped_ = gideon::isMeleeWeaponEquipped(getEquipType(npcTemplate_->getInfo().npc_righthand()));
    isShieldEquipped_ = (getEquipType(npcTemplate_->getInfo().npc_lefthand()) == EquipType::etShield);

    return true;
}


void Npc::finalize()
{
    {
        std::lock_guard<LockType> lock(getLock());

        npcInfo_.reset();
        skillCastableAbility_->cancelAll();
    }

    resetForFinalize();

    Parent::finalize();
}


void Npc::frenze(int32_t size, bool isRelease)
{
    std::lock_guard<LockType> lock(getLock());

    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    float32_t& creatureSize = entityInfo.asCreatureScale();

    if (isRelease) {
        creatureSize = npcTemplate_->getScale() * 1000.0f;
    }
    else {
        creatureSize *= (size / 1000.0f);        
    }
}


void Npc::initState(CreatureStateInfo& stateInfo)
{
    npcState_= std::make_unique<NpcState>(*this, stateInfo);
}


ObjectPosition Npc::getNextSpawnPosition() const
{
    if (hasPath()) {
        return getHomePosition();
    }
    return Parent::getNextSpawnPosition();
}


float32_t Npc::getCurrentDefaultSpeed() const
{
    if (npcState_->isKnockback()) {
        return GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"knockback_speed") / 100.0f;
    }

    if (npcState_->shouldRun()) {
        return npcTemplate_->getRunSpeed();
    }
    
    if (isMarching_) {
        return (npcTemplate_->getWalkSpeed() + npcTemplate_->getRunSpeed()) / 2.0f;
    }

    return npcTemplate_->getWalkSpeed();
}


float32_t Npc::getModelingRadiusSize() const
{
    return npcTemplate_->getModelingRadius();
}


bips_t Npc::getMissChance(const Entity& target) const
{
    std::lock_guard<LockType> lock(getLock());

    const bips_t missChanceByLevel = Parent::getMissChance(target);

    // FYI: NPC는 테이블에 명시된 천분율을 그대로 사용한다
    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    const bips_t hitChance = statusInfo.attackStatus_[atsiHitRate] * 10;
    return clampBips(missChanceByLevel - hitChance);
}


bips_t Npc::getDodgeChance() const
{
    std::lock_guard<LockType> lock(getLock());

    // FYI: NPC는 테이블에 명시된 천분율을 그대로 사용한다
    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return statusInfo.defenceStatus_[dsiDodgeRate] * 10;
}


bips_t Npc::getParryChance() const
{
    if (! queryWeaponUseable()->isMeleeWeaponEquipped()) {
        return 0;
    }

    std::lock_guard<LockType> lock(getLock());

    // FYI: NPC는 테이블에 명시된 천분율을 그대로 사용한다
    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return statusInfo.defenceStatus_[dsiParryRate] * 10;
}


bips_t Npc::getBlockChance() const
{
    if (! queryWeaponUseable()->isShieldEquipped()) {
        return 0;
    }

    std::lock_guard<LockType> lock(getLock());

    // FYI: NPC는 테이블에 명시된 천분율을 그대로 사용한다
    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return statusInfo.defenceStatus_[dsiBlockRate] * 10;
}


bips_t Npc::getPhysicalCriticalChance() const
{
    std::lock_guard<LockType> lock(getLock());

    // FYI: NPC는 테이블에 명시된 천분율을 그대로 사용한다
    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return statusInfo.attackStatus_[atsiPhysicalCriticalRate] * 10;
}


bips_t Npc::getMagicCriticalChance() const
{
    std::lock_guard<LockType> lock(getLock());

    // FYI: NPC는 테이블에 명시된 천분율을 그대로 사용한다
    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return statusInfo.attackStatus_[atsiMagicCriticalRate] * 10;
}


ErrorCode Npc::spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition)
{
    resetForRespawn();

    restoreSpeedAndScale_i();

    initCastingDelay();
    getCreatureStatus().restorePoints(false);


    // FYI: 무적화를 Parent::spawn() 먼저 호출하지 않으면 애니 연출이 이상해짐 모어정보가 없어서 그런듯
    const sec_t sawnProtectionInterval = npcTemplate_->getSpawnIntervalSec();
    if (sawnProtectionInterval > 0) {
        getEffectScriptApplier().applyEffectScript(estSpawnProtection, GameTime(sawnProtectionInterval * 1000));
        MoreNpcInfo& moreNpcInfo = getUnionEntityInfo_i().asNpcInfo();
        moreNpcInfo.startSpawnTime_ = getTime();
    }

    const ErrorCode errorCode = Parent::spawn(worldMap, spawnPosition);
    if (isSucceeded(errorCode)) {
        if (sawnProtectionInterval > 0) {
            npcState_->resetStateExceptSpawnProtection();
        }
        else {
            npcState_->resetState();
        }
        isInRandomDungion_ = worldMap.isDungeon();
    }
    return ecOk;
}


ErrorCode Npc::despawn()
{
    if (queryCastable()->isCasting()) {
        queryCastable()->cancelCasting();
    }

    if (! queryCreatureState()->isDied()) {
        queryCreatureState()->died();
        getEffectScriptApplier().revert();                        
    }

    if (skillCastableAbility_.get() != nullptr) {
        skillCastableAbility_->cancelAll();
    }

    resetWorldEventable();

    if (shouldNotifyMovementable()) {
        auto event = std::make_shared<WorldEventEntityDespawndEvent>(getGameObjectInfo());
        WORLD->broadcast(event);
    }

    if (formation_.get() != nullptr) {
        if (formation_->isLeader(*this)) {
            formation_->dismiss();
        }
        else {
            formation_->leave(*this);
        }
        formation_.reset();
    }

    const GameObjectInfo summonerInfo = querySummonable()->getSummoner();
    if (summonerInfo.isBuilding()) {
        go::Entity* summoner = queryKnowable()->getEntity(summonerInfo);
        if (summoner != nullptr) {
            go::Buildable* buildable = summoner->queryBuildable(); assert(buildable != nullptr);
            buildable->hiringNpcDespawned(mercenaryId_);
        }
    }

    brain_->deactivate();

    return Parent::despawn();
}


void Npc::setCombatState()
{
    if (npcState_->combat()) {
        getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());
        brain_->combatStarted();
    }
}


void Npc::setPeaceState()
{
    if (npcState_->peace()) {
        getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());
    }
}


void Npc::setEvadeState()
{
    if (npcState_->evading(true)) {
        getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());
    }
}


void Npc::unsetEvadeState()
{
    if (npcState_->evading(false)) {
        getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());
    }
}


void Npc::setFleeState()
{
    if (npcState_->fleeing(true)) {
        getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());
    }
}


void Npc::unsetFleeState()
{
    if (npcState_->fleeing(false)) {
        getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());
    }
}


void Npc::resetState()
{
    const float32_t oldSpeed = getSpeed();

    restoreSpeedAndScale();

    npcState_->resetState();

    getController().queryCreatureEffectCallback()->changeMoveSpeed(oldSpeed);

    getEffectScriptApplier().revert();
}


bool Npc::canBuy() const
{
    return npcTemplate_->canBuy();
}


bool Npc::canSell() const
{
    return npcTemplate_->canSell();
}


bool Npc::hasMail() const
{
    return npcTemplate_->hasMail();
}


bool Npc::hasBank() const
{
    return npcTemplate_->hasBank();
}


bool Npc::hasAuction() const
{
    return npcTemplate_->hasAuction();
}


bool Npc::isStaticNpc() const
{
    return gideon::isStaticNpc(npcTemplate_->getNpcType());
}


go::Entity* Npc::getTopScorePlayer() const
{
    const ai::AggroList* aggroList = brain_->getAggroList();
    if (aggroList) {
        return aggroList->getTopScorePlayer();
    }
    return nullptr;
}


ExpPoint Npc::getRewardExp(CreatureLevel rewarderLevel) const
{
    const CreatureLevel npcLevel = getCreatureLevel();
    int32_t diffLevel = npcLevel - rewarderLevel; 
    ExpPoint baseExpPoint = npcTemplate_->getExpPoint();
    if (isInRandomDungion_) {
        baseExpPoint = toExpPoint(baseExpPoint * npcTemplate_->getRandomDungeonExpPct() / 1000);
    }
    else if (getWorldEventMissionCallback() != nullptr) {
        baseExpPoint = toExpPoint(baseExpPoint * npcTemplate_->getWorldEventExpPct() / 1000);
    }

    const int32_t rewardExpDiffLevel = GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"reward_exp_diff_level");
    
    if (diffLevel < -rewardExpDiffLevel) {
        const float32_t lowPct = 
            static_cast<float32_t>(GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"low_diff_level_reward_exp_pct") / 1000.0f);
        return toExpPoint(static_cast<int>(baseExpPoint * lowPct));
    }
    else if (rewardExpDiffLevel < diffLevel) {
        const float32_t highPct = 
            static_cast<float32_t>(GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"high_diff_level_reward_exp_pct") / 1000.0f);
        return toExpPoint(static_cast<int>(baseExpPoint * highPct));
    }
    return baseExpPoint;
}


void Npc::resetForFinalize()
{
    std::lock_guard<LockType> lock(getLock());

    if (brain_.get() != nullptr) {
        brain_->finalize();
    }

    if (skillCastableAbility_.get() != nullptr) {
        skillCastableAbility_->cancelAll();
    }

    targetInfo_.reset();
}


void Npc::resetForRespawn()
{
    std::lock_guard<LockType> lock(getLock());

    if (brain_.get() != nullptr) {
        brain_->deactivate();
    }

    targetInfo_.reset();
    npcState_->resetState();
}


void Npc::restoreSpeedAndScale()
{
    std::lock_guard<LockType> lock(getLock());

    restoreSpeedAndScale_i();
}


void Npc::restoreSpeedAndScale_i()
{
    UnionEntityInfo& info = getUnionEntityInfo_i();
    info.asCreatureMoveSpeed() = npcTemplate_->getWalkSpeed();
    info.asCreatureScale() = npcTemplate_->getScale() * 1000.0f;

    resetSpeedRate_i();
}


bool Npc::setTarget(GameObjectInfo& oldTargetInfo, bool& isAggressive,
    const go::Entity& target, const GameObjectInfo& targetInfo)
{
    if (getCreatureStatus().isDied()) {
        oldTargetInfo = targetInfo_;
        targetInfo_.reset();    
        return false;
    }

    const bool isTargetChanged = ! (targetInfo_ == targetInfo);    
    if (! isTargetChanged) {
        return false;
    }

    oldTargetInfo = targetInfo_;
    targetInfo_ = targetInfo;

    if (! queryFactionable()->isFriendlyTo(target)) {
        (void)npcState_->combat();
        isAggressive = true;
    }

    getUnionEntityInfo_i().asNpcInfo().isAggressive_ = isAggressive;
    return true;
}


bool Npc::isWanderable() const
{
    return npcTemplate_->getMaxWanderDistance() > 0.0f;
}


std::unique_ptr<EffectHelper> Npc::createEffectHelper()
{
    return std::make_unique<NpcEffectHelper>(*this);
}

// = EntityStateAbility overriding

MoveState* Npc::queryMoveState()
{
    return npcState_.get();
}


CreatureState* Npc::queryCreatureState()
{
    return npcState_.get();
}


SkillCasterState* Npc::querySkillCasterState()
{
    return npcState_.get();
}


CombatState* Npc::queryCombatState()
{
    return npcState_.get();
}


CastState* Npc::queryCastState()
{
    return npcState_.get();
}


ErrorCode Npc::canBankable(go::Entity& /*player*/) const
{
    if (! hasBank()) {
        return ecBankNotAbilltyBankObject;
    }
    return ecOk;
}

// = Positionable overriding

void Npc::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);

    npcInfo_.position_ = position;
}


void Npc::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);

    npcInfo_.position_.heading_ = heading;
}

// = Moveable overriding

std::unique_ptr<gc::MoveController> Npc::createMoveController()
{
    return std::make_unique<gc::NpcMoveController>(this);
}

// = Liveable overriding

std::unique_ptr<CreatureStatus> Npc::createCreatureStatus()
{
    return std::make_unique<NpcStatus>(*this, getUnionEntityInfo_i(), npcInfo_);
}

ErrorCode Npc::revive(bool /*skipTimeCheck*/)
{
    const ErrorCode errorCode = despawn();
    if (isFailed(errorCode)) {
        assert(false);
        return errorCode;
    }

    return ecOk;
}


bool Npc::reviveByEffect(HitPoint& /*refillPoint*/, permil_t /*perRefillHp*/)
{
    const ErrorCode errorCode = despawn();
    if (isFailed(errorCode)) {
        assert(false);
        return false;
    }

    return true;
}

// = Thinkable overriding

bool Npc::hasWalkRoutes() const
{
    if (npcTemplate_->getWalkSpeed() <= 0.01f) {
        return false;
    }

    if (hasPath()) {
        return true;
    }
    return isWanderable();
}

// = Tickable overriding

void Npc::tick(GameTime diff)
{
    brain_->analyze(diff);

    const bool isDied = queryCreatureState()->isDied();
    if (isDied) {
        getEffectScriptApplier().revert();
    }
    else {
        getEffectScriptApplier().tick();

        getMovementManager().updateMovement(diff);
    }
}

// = SkillCastable overriding

ErrorCode Npc::castTo(const GameObjectInfo& targetInfo, SkillCode skillCode)
{
    return skillCastableAbility_->castTo(targetInfo, skillCode);
}


ErrorCode Npc::castAt(const Position& targetPosition, SkillCode skillCode)
{
    return skillCastableAbility_->castAt(targetPosition, skillCode);
}


void Npc::cancel(SkillCode skillCode)
{
    skillCastableAbility_->cancel(skillCode);
}


void Npc::cancelAll()
{
    skillCastableAbility_->cancelAll();
}


void Npc::consumePoints(const Points& points)
{
    // NPC는 포인트 소모를 하지 않는다
    points;
}


void Npc::consumeMaterialItem(const BaseItemInfo& itemInfo)
{
    itemInfo;
}


void Npc::setNextGlobalCooldownTime(uint32_t index, msec_t nextCooldownTime)
{
    globalGoolDownTimer_.setNextGlobalCooldownTime(index, nextCooldownTime);
}


ErrorCode Npc::checkSkillCasting(SkillCode skillCode,
    const GameObjectInfo& targetInfo) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetInfo);
}


ErrorCode Npc::checkSkillCasting(SkillCode skillCode,
    const Position& targetPosition) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetPosition);
}


bool Npc::checkCastableNeedSkill(SkillCode /*needSkillCode*/) const
{
    return true;
}


bool Npc::checkCastableUsableState(SkillUseableState /*needUsableState*/) const
{
    return true;
}


bool Npc::checkCastableEquip(EquipPart /*checekEquipPart*/, SkillCastableEquipType /*checkAllowedType*/) const
{
    return true;
}


bool Npc::checkCastableNeedItem(const BaseItemInfo& /*needItem*/) const
{
    return true;
}


bool Npc::checkCastableEffectCategory(EffectStackCategory category) const
{
    return getEffectScriptApplier().hasEffectStackCategory(category);
}


ErrorCode Npc::checkCastablePoints(PointType pt, bool isPercent,
    bool isCheckUp, uint32_t checkValue) const
{
    std::lock_guard<LockType> lock(getLock());

    if (gideon::isValid(pt)) {
        const CreatureStatusInfo& gameStatus = getCurrentCreatureStatusInfo();

        uint32_t currentValue = 0;         
        if (pt == ptHp) {
            currentValue = isPercent ? 
                getPercentValue(npcInfo_.currentPoints_.hp_, gameStatus.points_.hp_) : 
                npcInfo_.currentPoints_.hp_;
        }

        if (isCheckUp) {
            if (currentValue <= checkValue) {
                return ecSkillNotEnoughStats;
            }
        }
        else {
            if (currentValue >= checkValue) {
                return ecSkillOverStats;
            }
        }
    }
    return ecOk;
}


void Npc::notifyChangeCondition(PassiveCheckCondition condition)
{
    if (passiveSkillManager_.get()) {
        passiveSkillManager_->notifyChangeCondition(condition);
    }
}


msec_t Npc::getNextGlobalCooldownTime(uint32_t index) const
{
    return globalGoolDownTimer_.getNextGlobalCooldownTime(index);
}


float32_t Npc::getLongestSkillDistance() const
{
    return skillCastableAbility_->getLongestSkillDistance();
}


bool Npc::isUsing(SkillCode skillCode) const
{
    return skillCastableAbility_->isUsing(skillCode);
}


bool Npc::canCast(SkillCode skillCode) const
{
    return skillCastableAbility_->canCast(skillCode);
}

// = Skillable overriding

std::unique_ptr<gc::SkillController> Npc::createSkillController()
{
    return std::make_unique<gc::NpcSkillController>(this);
}


std::unique_ptr<gc::CreatureEffectController> Npc::createEffectController()
{
    return std::make_unique<gc::NpcEffectController>(this);
}

// = Dialogable overriding

void Npc::openDialog(const Entity& requester)
{
    if (! npcTemplate_->canDialog()) {
        return;
    }

    if (! queryKnowable()->doesKnow(requester)) {
        return;
    }

    if (! DISTANCE_CHECKER->checkNpcDistance(requester.getPosition(), getPosition())) {
        return;
    }

    if (npcState_->isCombating()) {
        return; // 전투 중에는 대화 금지
    }

    ai::Stateable* stateable = brain_->queryStateable();
    if (stateable != nullptr) {
        stateable->asyncHandleEvent(ai::eventDialogRequested);
    }

    auto event = std::make_shared<DialogOpenedEvent>(
        getGameObjectInfo(), requester.getGameObjectInfo());
    queryKnowable()->broadcast(event);
}


void Npc::closeDialog(const GameObjectInfo& requester)
{
    ai::Stateable* stateable = brain_->queryStateable();
    {
        std::lock_guard<LockType> lock(getLock());

        if (stateable != nullptr) {
            if (! stateable->isInState(ai::stateDialog)) {
                return;
            }
        }
    }

    if (stateable != nullptr) {
        stateable->asyncHandleEvent(ai::eventActive);
    }

    auto event = std::make_shared<DialogClosedEvent>(
        getGameObjectInfo(), requester);
    queryKnowable()->broadcast(event);
}

// = Formable overriding

void Npc::formUp()
{
    assert(! formation_.get());
    formation_= std::make_unique<NpcFormation>(*this);

    if (! formation_->formUp(formation_)) {
        formation_.reset();
    }
}


void Npc::setFormation(NpcFormationRefPtr formation)
{
    assert(formation.get() != nullptr);

    formation_ = formation;

    if (! formation_->isLeader(*this)) {
        assert(formation->getLeader().isNpcOrMonster());
        go::Npc& leader = static_cast<go::Npc&>(formation->getLeader());
        go::Factionable* leaderFactionable = leader.queryFactionable();
        if (leaderFactionable != nullptr) {
            go::Factionable* factionable = queryFactionable();
            if (factionable != nullptr) {
                factionable->setFaction(leaderFactionable->getFactionCode());
            }
        }

        const float32_t leaderMaxMoveDistance = leader.getNpcTemplate().getMaxMoveDistance();
        if (leaderMaxMoveDistance > maxMoveDistance_) {
            maxMoveDistance_ = leaderMaxMoveDistance;
        }
    }
}


bool Npc::hasForm() const
{
    return NpcFormation::hasForm(*this);
}

// = Marchable overriding

void Npc::marchStarted()
{
    isMarching_ = true;
    getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());

    NpcFormationRefPtr formation = queryFormable()->getFormation();
    if ((formation.get() != nullptr) && (formation->isLeader(*this))) {
        formation->marchStarted();
    }
}


void Npc::marchStopped()
{
    isMarching_ = false;
    getController().queryCreatureEffectCallback()->changeMoveSpeed(getSpeed());

    NpcFormationRefPtr formation = queryFormable()->getFormation();
    if ((formation.get() != nullptr) && (formation->isLeader(*this))) {
        formation->marchStopped();
    }
}


bool Npc::shouldMarch() const
{
    if (! isInvader()) {
        return false;
    }

    // TODO: 보스인 경우는 행군을 하지 않아야 하는가?

    return ! isLastPathNode(getCurrentPathIndex());
}

// = WorldEventable overriding

void Npc::notifyMovemoent(const Position& position, bool isStop)
{
    updateNotifyMovementCount(isStop);
    if (shouldNotifyMovement()) {
        auto event = std::make_shared<WorldEventEntityMovedEvent>(getGameObjectInfo(),
            getEntityCode(), position);
        WORLD->broadcast(event);
    }
}

// = Invadable overriding

bool Npc::isInvader() const
{
    return gideon::isInvader(getNpcType(npcInfo_.npcCode_));
}

// = Summonable overriding

void Npc::setSummoner(go::Entity& summoner, SpawnType spawnType)
{
    Parent::setSummoner(summoner, spawnType);

    if (summoner.isNpcOrMonster()) {
        if (shouldInheritFaction(spawnType)) {
            const float32_t summonerMaxMoveDistance =
                static_cast<go::Npc&>(summoner).getNpcTemplate().getMaxMoveDistance();
            if (summonerMaxMoveDistance > maxMoveDistance_) {
                maxMoveDistance_ = summonerMaxMoveDistance;
            }
        }
    }
}


void Npc::resetSummoner()
{
    maxMoveDistance_ = npcTemplate_->getMaxMoveDistance();
}

// = Craftable overriding

bool Npc::hasCraftFunction(CraftType craftType) const
{
    return npcTemplate_->hasCraftFunction(craftType);
}


// = TargetSelectable overriding

const go::Entity* Npc::selectTarget(const GameObjectInfo& targetInfo)
{
    if (! targetInfo.isValid()) {
        assert(false);
        return nullptr;
    }

    go::Entity* target = queryKnowable()->getEntity(targetInfo);
    if (! target) {
        return nullptr;
    }

    GameObjectInfo oldTargetInfo;
    bool isAggressive = false;
    bool isSuccessSetTarget = false;
    {
        std::lock_guard<LockType> lock(getLock());

        isSuccessSetTarget = setTarget(oldTargetInfo, isAggressive, *target, targetInfo);
    }

    if (oldTargetInfo.isValid()) {
        go::Entity* oldTarget = queryKnowable()->getEntity(oldTargetInfo);
        if (oldTarget != nullptr) {
            AggroSendable* oldAggroSendable = oldTarget->queryAggroSendable();
            if ((oldAggroSendable != nullptr) && (! queryFactionable()->isFriendlyTo(*oldTarget))) {
                oldAggroSendable->removeAggroTarget(getGameObjectInfo());
            }
            oldTarget->unregisterObserver(getGameObjectInfo()); 
        }
    }

    if (! isSuccessSetTarget) {
        return nullptr;
    }

    Entity* targetOfTarget = nullptr;
    if (target != nullptr) {
        setHeading(getHeading(target->getPosition(), getPosition()));
        if (isAggressive) {
            AggroSendable* newAggroSendable = target->queryAggroSendable();
            if (newAggroSendable != nullptr) {
                newAggroSendable->setAggroTarget(getGameObjectInfo(), *this);
            }
        }
        target->registerObserver(getGameObjectInfo());

        go::TargetSelectable* targetSelectable = target->queryTargetSelectable();
        if (targetSelectable != nullptr) {
            targetOfTarget = targetSelectable->getSelectedTarget();
        }
    }

    auto targetChangedEvent = std::make_shared<TargetChangedEvent>(
        getGameObjectInfo(), WORLD->getEntityStatusInfo(target));
    notifyToObervers(targetChangedEvent, true);

    auto targetSelectedEvent = std::make_shared<TargetSelectedEvent>(
        getGameObjectInfo(), targetInfo, isAggressive);
    queryKnowable()->broadcast(targetSelectedEvent);
    return targetOfTarget;
}


void Npc::unselectTarget()
{
    GameObjectInfo oldTargetInfo;
    {
        std::lock_guard<LockType> lock(getLock());

        oldTargetInfo = targetInfo_;
        targetInfo_.reset();
    }

    if (! oldTargetInfo.isValid()) {
        return;
    }

    go::Entity* oldTarget = queryKnowable()->getEntity(oldTargetInfo);
    if (oldTarget != nullptr) {
        AggroSendable* oldAggroSendable = oldTarget->queryAggroSendable();
        if ((oldAggroSendable != nullptr) && (! queryFactionable()->isFriendlyTo(*oldTarget))) {
            oldAggroSendable->removeAggroTarget(getGameObjectInfo());
        }
        oldTarget->unregisterObserver(getGameObjectInfo()); 
    }

    EntityEvent::Ref targetChangedEvent(
        new TargetChangedEvent(getGameObjectInfo(), EntityStatusInfo()));
    notifyToObervers(targetChangedEvent, true);

    go::EntityEvent::Ref targetSelectedEvent(
        new TargetSelectedEvent(getGameObjectInfo(), GameObjectInfo::null(), false));
    queryKnowable()->broadcast(targetSelectedEvent);
}


go::Entity* Npc::getSelectedTarget()
{
    const GameObjectInfo& targetInfo = getSelectedTargetInfo();
    if (! targetInfo.isValid()) {
        return nullptr;
    }
    return queryKnowable()->getEntity(targetInfo);
}


const GameObjectInfo& Npc::getSelectedTargetInfo() const
{
    std::lock_guard<LockType> lock(getLock());

    return targetInfo_;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
