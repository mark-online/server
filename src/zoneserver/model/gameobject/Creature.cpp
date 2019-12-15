#include "ZoneServerPCH.h"
#include "Creature.h"
#include "EntityEvent.h"
#include "ability/impl/KnowableAbility.h"
#include "ability/impl/FactionableAbility.h"
#include "ability/impl/AggroSendableImpl.h"
#include "ability/impl/CastableImpl.h"
#include "ability/impl/CastNotificationableImpl.h"
#include "skilleffect/CreatureEffectScriptApplier.h"
#include "../time/CoolDownTimer.h"
#include "../time/GlobalCoolDownTimer.h"
#include "../../world/WorldMap.h"
#include "../../controller/EntityController.h"
#include "../../controller/MoveController.h"
#include "../../controller/SkillController.h"
#include "../../controller/CreatureEffectController.h"
#include "../../service/time/GameTimer.h"
#include "../../service/movement/MovementManager.h"
#include <gideon/3d/3d.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver { namespace go {

#pragma warning (disable: 4355)

Creature::Creature(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller)),
    knowable_(std::make_unique<KnowableAbility>(*this)),
    globalCoolTime_(std::make_unique<GlobalCoolDownTimer>()),
    castingDelay_(defaultSkillDelayPercent)
{
    coolTime_= std::make_unique<CoolDownTimer>(*this);
    castable_= std::make_unique<CastableImpl>(*this);
    castNotificationable_= std::make_unique<CastNotificationableImpl>(*this);
}


Creature::~Creature()
{
}


bool Creature::initialize(ObjectType objectType, ObjectId objectId, FactionCode factionCode)
{
    std::lock_guard<LockType> lock(getLock());

    if (! Parent::initialize(objectType, objectId)) {
        return false;
    }

    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    CreatureInfo& creatureInfo = entityInfo.asCreatureInfo();
    initState(creatureInfo.stateInfo_);
    castingDelay_ = defaultSkillDelayPercent;
    if (! moveController_) {
        moveController_ = createMoveController();
    }
    moveController_->initialize();

    if (! factionable_) {
        factionable_= std::make_unique<FactionableAbility>(*this);
    }
    factionable_->setFaction(factionCode);

    if (! aggroSendable_) {
        aggroSendable_= std::make_unique<AggroSendableImpl>(*this);
    }

    if (! effectController_) {
        effectController_ = createEffectController();
    }
    effectController_->initialize();

    if (! skillController_) {
        skillController_ = createSkillController();
    }

    if (! creatureStatus_) {
        creatureStatus_ = createCreatureStatus();
    }

    if (creatureStatus_) { // FYI: 테스트 코드 때문에
        creatureStatus_->initialize();
    }

    if (! movementManager_.get()) {
        movementManager_= std::make_unique<MovementManager>(*this);
    }
    movementManager_->initialize();

    speedRate_ = 0;
    return true;
}


void Creature::finalize()
{
    {
        std::lock_guard<LockType> lock(getLock());

        knowable_->forgetAll();

        if (moveController_.get() != nullptr) {
            moveController_->finalize();
        }

        if (creatureStatus_.get() != nullptr) {
            creatureStatus_->finalize();
        }

        lastWorldPosition_.reset();
        castingDelay_ = defaultSkillDelayPercent;

        if (coolTime_.get() != nullptr) {
            coolTime_->finalize();
        }

        if (movementManager_.get() != nullptr) {
            movementManager_->finalize();
        }
    }

    Parent::finalize();
}


std::unique_ptr<EffectScriptApplier> Creature::createEffectScriptApplier()
{
    return std::make_unique<CreatureEffectScriptApplier>(*this);
}


const DebuffBuffEffectInfoSet Creature::getDebuffBuffEffectInfoSet() const
{
    std::lock_guard<LockType> lock(getLock());

    return static_cast<const CreatureEffectScriptApplier&>(getEffectScriptApplier()).getDebuffBuffEffectInfoSet();
}


float32_t Creature::getCurrentScale() const
{
    std::lock_guard<LockType> lock(getLock());

    const UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    return entityInfo.asCreatureScale();
}


uint32_t Creature::getCastingDelay() const
{
    return castingDelay_;
}


uint32_t Creature::downCastingDelay(uint32_t delayTime)
{
    uint32_t appyDelayTime = delayTime;

    std::lock_guard<LockType> lock(getLock());

    if (castingDelay_ <  delayTime) {
        appyDelayTime = castingDelay_;
        castingDelay_ = 0;
    }
    else {
        castingDelay_ -= delayTime;
    }
    return appyDelayTime;
}


void Creature::upCastingDelay(uint32_t delayTime)
{
    std::lock_guard<LockType> lock(getLock());

    castingDelay_ += delayTime;
}


void Creature::setCooldown(DataCode dataCode, GameTime coolTime,
    uint32_t index, GameTime globalCoolDownTime)
{
    globalCoolTime_->setNextGlobalCooldownTime(index, globalCoolDownTime);
    coolTime_->startCooldown(dataCode, coolTime);
}


void Creature::cancelCooldown(DataCode dataCode, uint32_t index)
{
    globalCoolTime_->cancelCooldown(index);
    coolTime_->cancelCooldown(dataCode);    
}


void Creature::cancelPreCooldown()
{
    globalCoolTime_->cancelPreCooldown();
    coolTime_->cancelPreCooldown();
}


bool Creature::isGlobalCooldown(uint32_t index) const
{
    const GameTime currentTime = GAME_TIMER->msec();

    const msec_t nextGlobalCooldownTime = 
        globalCoolTime_->getNextGlobalCooldownTime(index);
    if (nextGlobalCooldownTime == 0) {
        return false;
    }
    return currentTime < nextGlobalCooldownTime;
}


bool Creature::isLocalCooldown(DataCode dataCode) const
{
    return coolTime_->isCooldown(dataCode);;
}


bips_t Creature::getLevelBonus(const Entity& target) const
{
    if (target.isCreature()) {
        const bips_t rate = 5; // 0.05%
        const go::Creature& targetCreature = static_cast<const go::Creature&>(target);
        return rate * (targetCreature.getCreatureLevel() - getCreatureLevel());
    }

    // TODO: anchor, building
    return 0;
}


bips_t Creature::getMissChance(const Entity& target) const
{
    if (target.isCreature()) {
        const bips_t maxMissChance = 60 * 100;
        const int32_t baseLevelDiff = 2;
        const go::Creature& targetCreature = static_cast<const go::Creature&>(target);
        const int32_t levelDiff = targetCreature.getCreatureLevel() - getCreatureLevel();

        bips_t baseChance = 0;
        bips_t levelDiffChance = 0;
        if (std::abs(levelDiff) <= baseLevelDiff) {
            baseChance = 5 * 100;
            levelDiffChance = 50; // 0.5 * 100;
        }
        else {
            baseChance = 2 * 100;
            levelDiffChance = 2 * 100;
        }

        const bips_t missChance = baseChance + (levelDiff * levelDiffChance);
        return clampBips(missChance, 0, maxMissChance);
    }

    // TODO: anchor, building
    return 0;
}


bips_t Creature::getResistChance(AttributeRateType attributeType) const
{
    if (! gideon::isValid(attributeType)) {
        return 0;
    }

    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return statusInfo.attributeResists_[getAttributeRateIndex(attributeType)] * 10;
}

// = Positionable overriding

void Creature::setWorldMap(WorldMap& worldMap)
{
    const MapCode mapCode = worldMap.getMapCode();
    const MapType mapType = getMapType(mapCode);

    std::lock_guard<LockType> lock(getLockPositionable());

    if ((mapType == mtGlobalWorld) || (mapType == mtArena)) {
        lastWorldPosition_.mapCode_ = mapCode;
        // FYI: 디버깅을 위해
        static_cast<ObjectPosition&>(lastWorldPosition_) = ObjectPosition();
    }
    else {
        static_cast<ObjectPosition&>(lastWorldPosition_) = getPosition();
    }

    Parent::setWorldMap(worldMap);
}


WorldPosition Creature::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* currentWorldMap = getCurrentWorldMap();
    if (currentWorldMap != nullptr) {
        const MapType mapType = currentWorldMap->getMapType();
        if ((mapType == mtGlobalWorld) || (mapType == mtArena)) {
            assert(lastWorldPosition_.mapCode_ == currentWorldMap->getMapCode());
            static_cast<ObjectPosition&>(lastWorldPosition_) = getPosition();
        }
    }
    return lastWorldPosition_;
}


void Creature::resetCooldowns()
{
    std::lock_guard<LockType> lock(getLock());

    globalCoolTime_->reset();
    coolTime_->reset();
}

// = Moveable overriding

void Creature::setDestination(const ObjectPosition& destin)
{
    std::lock_guard<LockType> lock(getLock());

    if (! isValid()) {
        return;
    }

    getUnionEntityInfo_i().asEntityMovementInfo().destination_ = destin;
}


void Creature::setMoving(bool isMoving)
{
    std::lock_guard<LockType> lock(getLock());

    if (! isValid()) {
        return;
    }

    getUnionEntityInfo_i().asEntityMovementInfo().isMoving_ = isMoving;
}


ObjectPosition Creature::getDestination() const
{
    std::lock_guard<LockType> lock(getLock());

    if (! isValid()) {
        return ObjectPosition();
    }

    return getUnionEntityInfo_i().asEntityMovementInfo().destination_;
}


void Creature::setSpeedRate(permil_t totalSpeedRate)
{
    std::lock_guard<LockType> lock(getLock());

    speedRate_ += totalSpeedRate;
}


void Creature::resetSpeedRate()
{
    std::lock_guard<LockType> lock(getLock());

    resetSpeedRate_i();
}


void Creature::updateSpeed()
{
    float32_t oldSpeed = 0.0f;
    float32_t newSpeed = 0.0f;
    {
        std::lock_guard<LockType> lock(getLock());

        newSpeed = getCurrentDefaultSpeed();
        newSpeed += (newSpeed * speedRate_) / 1000.0f;

        UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
        oldSpeed = entityInfo.asCreatureMoveSpeed();
        entityInfo.asCreatureMoveSpeed() = newSpeed;
    }

#ifndef NDEBUG
    SNE_LOG_DEBUG("Creature::updateSpeed(%d, %" PRIu64 ") - %f -> %f",
        getObjectType(), getObjectId(), oldSpeed, newSpeed);
#endif
}


bool Creature::isMoving() const
{
    std::lock_guard<LockType> lock(getLock());

    if (! isValid()) {
        return false;
    }

    return getUnionEntityInfo_i().asEntityMovementInfo().isMoving_;
}


float32_t Creature::getSpeed() const
{
    std::lock_guard<LockType> lock(getLock());

    const UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    return entityInfo.asCreatureMoveSpeed();
}


msec_t Creature::getCalcCastingTime(msec_t castingTime) const
{
    std::lock_guard<LockType> lock(getLock());

    if (castingDelay_ == 1000) {
        return castingTime;
    }

    return (castingTime * castingDelay_) / 1000;        
}

}}} // namespace gideon { namespace zoneserver { namespace go {
