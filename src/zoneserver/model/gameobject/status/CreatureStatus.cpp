#include "ZoneServerPCH.h"
#include "CreatureStatus.h"
#include "../Creature.h"
#include "../Entity.h"
#include "../EntityEvent.h"
#include "../../state/CastState.h"
#include "../../state/CreatureState.h"
#include "../../state/HarvestState.h"
#include "../../state/TreasureState.h"
#include "../skilleffect/CreatureEffectScriptApplier.h"
#include "../ability/Partyable.h"
#include "../ability/PassiveSkillCastable.h"
#include "../ability/Castable.h"
#include "../../../service/party/Party.h"
#include "../../../controller/callback/StatsCallback.h"
#include "../../../controller/callback/CreatureEffectCallback.h"
#include "../../../controller/EntityController.h"
#include "../../../service/time/GameTimer.h"
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class PointsRestoredEvent
 */
class PointsRestoredEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PointsRestoredEvent>
{
public:
    PointsRestoredEvent(const GameObjectInfo& entityInfo) :
        entityInfo_(entityInfo) {}

private:
    virtual void call(go::Entity& entity) {
        gc::StatsCallback* callback = entity.getController().queryStatsCallback();
        if (callback) {
            callback->pointsRestored(entityInfo_);
        } 
    }

private:
    const GameObjectInfo entityInfo_;
};



/**
 * @class PointChangedEvent
 */
class PointChangedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PointChangedEvent>
{
public:
    PointChangedEvent(const GameObjectInfo& entityInfo, PointType pointType,
        uint32_t currentPoint) :
        entityInfo_(entityInfo),
        pointType_(pointType),
        currentPoint_(currentPoint) {}

private:
    virtual void call(go::Entity& entity) {
        gc::StatsCallback* callback = entity.getController().queryStatsCallback();
        if (callback) {
            callback->pointChanged(entityInfo_, pointType_, currentPoint_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const PointType pointType_;
    const uint32_t currentPoint_;
};


/**
 * @class MaxPointChangedEvent
 */
class MaxPointChangedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<MaxPointChangedEvent>
{
public:
    MaxPointChangedEvent(const GameObjectInfo& entityInfo, PointType pointType,
        uint32_t currentPoint, uint32_t maxPoint) :
        entityInfo_(entityInfo),
        pointType_(pointType),
        currentPoint_(currentPoint),
        maxPoint_(maxPoint) {}

private:
    virtual void call(go::Entity& entity) {
        gc::StatsCallback* callback = entity.getController().queryStatsCallback();
        if (callback) {
            callback->maxPointChanged(entityInfo_, pointType_, currentPoint_, maxPoint_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const PointType pointType_;
    const uint32_t currentPoint_;
    const uint32_t maxPoint_;
};


/**
 * @class CreateShieldEvent
 */
class CreateShieldEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CreateShieldEvent>
{
public:
    CreateShieldEvent(const GameObjectInfo& entityInfo, PointType pointType,
        uint32_t createPoint) :
        entityInfo_(entityInfo),
        pointType_(pointType),
        createPoint_(createPoint)
    {
    }

    virtual void call(go::Entity& entity) {
        gc::StatsCallback* callback = entity.getController().queryStatsCallback();
        if (callback) {
            callback->shieldCreated(entityInfo_, pointType_, createPoint_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const PointType pointType_;
    uint32_t createPoint_;
};


/**
 * @class DestoryShieldEvent
 */
class DestoryShieldEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<DestoryShieldEvent>
{
public:
    DestoryShieldEvent(const GameObjectInfo& entityInfo, PointType pointType) :
        entityInfo_(entityInfo),
        pointType_(pointType)
    {
    }
    virtual void call(go::Entity& entity) {
        gc::StatsCallback* callback = entity.getController().queryStatsCallback();
        if (callback) {
            callback->shieldDestroyed(entityInfo_, pointType_);
        }
    }
private:
    const GameObjectInfo entityInfo_;
    const PointType pointType_;
};


/**
 * @class ShieldPointChangedEvent
 */
class ShieldPointChangedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ShieldPointChangedEvent>
{
public:
    ShieldPointChangedEvent(const GameObjectInfo& entityInfo, PointType pointType,
        uint32_t currentPoint) :
        entityInfo_(entityInfo),
        pointType_(pointType),
        currentPoint_(currentPoint) {}

private:
    virtual void call(go::Entity& entity) {
        gc::StatsCallback* callback = entity.getController().queryStatsCallback();
        if (callback) {
            callback->shieldPointChanged(entityInfo_, pointType_, currentPoint_);
        }   
    }

private:
    const GameObjectInfo entityInfo_;
    const PointType pointType_;
    const uint32_t currentPoint_;
};


} // namespace


CreatureStatus::CreatureStatus(go::Creature& owner,
    UnionEntityInfo& unionCreatureInfo) :
    owner_(owner),
    unionCreatureInfo_(unionCreatureInfo),
    lastDeadTime_(0)
{
}


FullCreatureStatusInfo CreatureStatus::getFullCreatureStatusInfo()
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! unionCreatureInfo_.isValid()) {
        return FullCreatureStatusInfo(getCurrentPoints(), currentGameStatus_);
    }
    return FullCreatureStatusInfo(getCurrentPoints(), currentGameStatus_);
}


LifeStats CreatureStatus::getCurrentLifeStats() const
{
    LifeStats lifeStats;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return lifeStats;
        }
        lifeStats.currentPoints_ = getCurrentPoints();
        lifeStats.maxPoints_ = getMaxPoints();
    }
    return lifeStats;
}


HitPoints CreatureStatus::getHitPoints() const
{
    HitPoints points;    
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return points;
        }
        points.hp_ = getCurrentPoints().hp_;
        points.maxHp_ = getMaxPoints().hp_;
    }
    return points;
}


ManaPoints CreatureStatus::getManaPoints() const
{
    ManaPoints points;    
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return points;
        }
        points.mp_ = getCurrentPoints().mp_;
        points.maxMp_ = getMaxPoints().mp_;
    }
    return points;
}


bool CreatureStatus::reduceHp(HitPoint& applyPoint, HitPoint hp, bool canDie)
{
    HitPoint point = hpMin;
    HitPoint shieldPoint = hpMin;
    bool shieldHpChanged = false;
    bool hpChanged = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return false;
        }

        HitPoint remainHp = hpMin;
        shieldHpChanged = shields_.reduceHp(applyPoint, remainHp, hp);
        if (shieldHpChanged) {
            shieldPoint = shields_.shlidHp_.hp_;
        }

        if (remainHp > hpMin) {
            hpChanged = reduceHp_i(applyPoint, remainHp, canDie);
            if (hpChanged) {
                point = getCurrentPoints().hp_;
            }
        }        
    }

    // 잠을 깨운다.
    owner_.queryCastState()->releaseNotSkillCastingByAttack();
    if (owner_.queryCreatureState()->isMutated()) {
        owner_.getEffectScriptApplier().cancelRemoveEffect(escMutation);        
    }

    if (owner_.queryCreatureState()->isSleeped()) {
        owner_.getEffectScriptApplier().cancelRemoveEffect(escMentalWeaken);
    }
    
    if (owner_.queryCreatureState()->isHidden()) {
        owner_.getEffectScriptApplier().cancelRemoveEffect(escHide);
    }

    TreasureState* treasureState = owner_.queryTreasureState();
    HarvestState* harvestState = owner_.queryHarvestState();

    if (treasureState && treasureState->isTreasureOpening()) {
        owner_.queryCastable()->cancelCasting();
    }
    else if (harvestState && harvestState->isHarvesting()) {
        owner_.queryCastable()->cancelCasting();
    }

    if (shieldHpChanged) {
        shieldPointChanged(ptHp, shieldPoint);
    }

    if (hpChanged) {
        pointChanged(ptHp, point);
    }

    return true;
}


void CreatureStatus::reduceMp(ManaPoint& applyPoint, ManaPoint mp)
{
    ManaPoint point = mpMin;
    ManaPoint shieldPoint = mpMin;
    bool shieldMpChanged = false;
    bool mpChanged = false;

    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        ManaPoint remainMp = mpMin;
        shieldMpChanged = shields_.reduceMp(applyPoint, remainMp, mp);
        if (shieldMpChanged) {
            shieldPoint = shields_.shlidMp_.mp_;
        }

        if (remainMp > mpMin) {
            mpChanged = reduceMp_i(applyPoint, remainMp);
            if (mpChanged) {
                point = getCurrentPoints().mp_;
            }
        }  
    }

    if (shieldMpChanged) {
        shieldPointChanged(ptMp, shieldPoint);
    }

    if (mpChanged) {
        pointChanged(ptMp, point);
    }
}


bool CreatureStatus::reduceHp(HitPoint hp, bool canDie)
{
    HitPoint temp = hpMin;
    return reduceHp(temp, hp, canDie);
}


void CreatureStatus::reduceMp(ManaPoint mp)
{
    ManaPoint temp = mpMin;
    return reduceMp(temp, mp);
}


void CreatureStatus::fillHp(HitPoint& applyPoint, HitPoint hp, bool force)
{
    HitPoint point = hpMin;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }
        const bool hpChanged = fillHp_i(applyPoint, hp, force);
        if (! hpChanged) {
            return;
        }
        
        point = getCurrentPoints().hp_;
    }

    pointChanged(ptHp, point);
}


void CreatureStatus::fillMp(ManaPoint& applyPoint, ManaPoint mp)
{
    ManaPoint point = mpMin;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }
        const bool mpChanged = fillMp_i(applyPoint, mp);
        if (! mpChanged) {
            return;
        }

        point = getCurrentPoints().mp_;
    }

    pointChanged(ptMp, point);
}


void CreatureStatus::fillHp(HitPoint hp, bool force)
{
    HitPoint temp = hpMin;
    fillHp(temp, hp, force);
}


void CreatureStatus::fillMp(ManaPoint mp)
{
    ManaPoint temp = mpMin;
    fillMp(temp, mp);
}


void CreatureStatus::setHp(HitPoint hp, bool canDie)
{
	{
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (!unionCreatureInfo_.isValid()) {
			return;
		}
		setHp_i(hp, canDie);		
	}
	pointChanged(ptHp, uint32_t(hp));
}


void CreatureStatus::setMp(ManaPoint mp)
{
	{
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (!unionCreatureInfo_.isValid()) {
			return;
		}
        setMp_i(mp);

	}
	pointChanged(ptMp, uint32_t(mp));
}


void CreatureStatus::restorePoints(bool shouldNotify)
{
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }
        restorePoints_i();
    }

    if (shouldNotify) {
        pointsRestored();
    }
}


void CreatureStatus::createShield(PointType pointType, uint32_t value)
{
    bool result = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        result = shields_.createShield(pointType, value);
    }

    if (result) {
        shieldCreated(pointType, value);
    }
}


void CreatureStatus::destoryShield(PointType pointType)
{
    bool result = false;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        result = shields_.destroyShield(pointType);
    }
    if (result) {
        shieldDestroyed(pointType);
    }
}


void CreatureStatus::upAttackStatus(AttackStatusIndex index, int16_t value)
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }
        
        switch(index) {
        case atsiHitRate:
            effectStatusType = esttHitRate;
            break;
        case atsiPhysicalCriticalRate:
            effectStatusType = esttPhysicalCriticalRate;
            break;
        case atsiMagicCriticalRate:
            effectStatusType = esttMagicCriticalRate;
            break;
        default:
            assert(false);
            return;
        }

        effectApplyStatusInfo_.attackStatus_[index] += value;
        currentGameStatus_.attackStatus_[index] += value;

        currentValue = currentGameStatus_.attackStatus_[index];        
    }

    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void CreatureStatus::downAttackStatus(AttackStatusIndex index, int16_t value)
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        switch(index) {
        case atsiHitRate:
            effectStatusType = esttHitRate;
            break;
        case atsiPhysicalCriticalRate:
            effectStatusType = esttPhysicalCriticalRate;
            break;
        case atsiMagicCriticalRate:
            effectStatusType = esttMagicCriticalRate;
            break;
        default:
            assert(false);
            return;
        }

        effectApplyStatusInfo_.attackStatus_[index] -= value;
        currentGameStatus_.attackStatus_[index] -= value;
        currentValue = currentGameStatus_.attackStatus_[index];
    }

    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void CreatureStatus::upAttackPowers(AttackPowerIndex index, int32_t value)
{
    EffectStatusType effectStatusType = esttNone;
    int32_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        switch (index) {
        case apiPhysical:
            effectStatusType = esttPhysical;
            break;
        case apiMagic:
            effectStatusType = esttMagic;
            break;
        default:
            assert(false);
            return;
        }

        effectApplyStatusInfo_.attackPowers_[index] += value;
        currentGameStatus_.attackPowers_[index] += value;
        currentValue = currentGameStatus_.attackPowers_[index];
    }

    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void CreatureStatus::downAttackPowers(AttackPowerIndex index, int32_t value)
{
    EffectStatusType effectStatusType = esttNone;
    int32_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        switch (index) {
        case apiPhysical:
            effectStatusType = esttPhysical;
            break;
        case apiMagic:
            effectStatusType = esttMagic;
            break;
        default:
            assert(false);
            return;
        }

        effectApplyStatusInfo_.attackPowers_[index] -= value;
        currentGameStatus_.attackPowers_[index] -= value;
        currentValue = currentGameStatus_.attackPowers_[index];
    }

    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void CreatureStatus::upDefenceStatus(DefenceStatusIndex index, int16_t value)
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        switch (index) {
        case dsiBlockRate:
            effectStatusType = esttBlockRate;
            break;
        case dsiDodgeRate:
            effectStatusType = esttDodgeRate;
            break;
        case dsiDefence:
            effectStatusType = esttDefence;
            break;
        default:
            assert(false);
            return;
        }

        effectApplyStatusInfo_.defenceStatus_[index] += value;
        currentGameStatus_.defenceStatus_[index] += value;
        currentValue = currentGameStatus_.defenceStatus_[index];
    }
    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void CreatureStatus::downDefenceStatus(DefenceStatusIndex index, int16_t value)
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        switch (index) {
        case dsiBlockRate:
            effectStatusType = esttBlockRate;
            break;
        case dsiDodgeRate:
            effectStatusType = esttDodgeRate;
            break;
        case dsiDefence:
            effectStatusType = esttDefence;
            break;
        default:
            assert(false);
            return;
        }

        effectApplyStatusInfo_.defenceStatus_[index] -= value;
        currentGameStatus_.defenceStatus_[index] -= value;
        currentValue = currentGameStatus_.defenceStatus_[index];
    }
    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void CreatureStatus::upAttribute(AttributeRateIndex index, int16_t value, bool isResist)
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (!unionCreatureInfo_.isValid()) {
            return;
        }

        switch (index) {
        case abiLight:
            effectStatusType = isResist ? esttLightResist : esttLight;
            break;
        case abiDark:
            effectStatusType = isResist ? esttDarkResist : esttDark;
            break;
        case abiFire:
            effectStatusType = isResist ? esttFireResist : esttFire;
            break;
        case abiIce:
            effectStatusType = isResist ? esttIceResist : esttIce;
            break;
        default:
            assert(false);
            return;
        }

        if (isResist) {
            effectApplyStatusInfo_.attributeResists_[index] += value;
            currentGameStatus_.attributeResists_[index] += value;
            currentValue = currentGameStatus_.attributeResists_[index];
        }
        else {
            effectApplyStatusInfo_.attributes_[index] += value;
            currentGameStatus_.attributes_[index] += value;
            currentValue = currentGameStatus_.attributes_[index];
        }
    }

    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
     
}


void CreatureStatus::upAllAttribute(int16_t value, bool isResist)
{
    AttributeRates rates;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }
        for (int i = 0; i < abiCount; ++i) {
            if (isResist) {
                effectApplyStatusInfo_.attributeResists_[i] += value;
                currentGameStatus_.attributeResists_[i] += value;
                rates = currentGameStatus_.attributeResists_;
            }
            else {
                effectApplyStatusInfo_.attributes_[i] += value;
                currentGameStatus_.attributes_[i] += value;
                rates = currentGameStatus_.attributes_; 
            }
        }
    }
    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->allAttributeChanged(currentGameStatus_.attributes_, isResist);
    }
}


void CreatureStatus::downAttribute(AttributeRateIndex index, int16_t value, bool isResist)
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        switch (index) {
        case abiLight:
            effectStatusType = isResist ? esttLightResist : esttLight;
            break;
        case abiDark:
            effectStatusType = isResist ? esttDarkResist : esttDark;
            break;
        case abiFire:
            effectStatusType = isResist ? esttFireResist : esttFire;
            break;
        case abiIce:
            effectStatusType = isResist ? esttIceResist :esttIce;
            break;
        default:
            assert(false);
            return;
        }

        if (isResist) {
            effectApplyStatusInfo_.attributeResists_[index] -= value;
            currentGameStatus_.attributeResists_[index] -= value;
            currentValue = currentGameStatus_.attributeResists_[index]; 
        }
        else {
            effectApplyStatusInfo_.attributes_[index] -= value;
            currentGameStatus_.attributes_[index] -= value;
            currentValue = currentGameStatus_.attributes_[index];
        }
    }
    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void CreatureStatus::downAllAttribute(int16_t value, bool isResist)
{
    AttributeRates rates;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }
        for (int i = 0; i < abiCount; ++i) {
            if (isResist) {
                effectApplyStatusInfo_.attributeResists_[i] -= value;
                currentGameStatus_.attributeResists_[i] -= value;
                rates = currentGameStatus_.attributeResists_;
            }
            else {
                effectApplyStatusInfo_.attributes_[i] -= value;
                currentGameStatus_.attributes_[i] -= value;
                rates = currentGameStatus_.attributes_;
            }
        }
    }
    gc::StatsCallback* callback = owner_.getController().queryStatsCallback();
    if (callback) {
        callback->allAttributeChanged(rates, isResist);
    }
}



void CreatureStatus::upHpCapacity(HitPoint hp)
{
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }

        effectApplyStatusInfo_.points_.hp_ = toHitPoint(effectApplyStatusInfo_.points_.hp_ + hp);        
        currentGameStatus_.points_.hp_ = toHitPoint(currentGameStatus_.points_.hp_ + hp);
    }

    maxPointChanged(ptHp, getCurrentPoints().hp_, currentGameStatus_.points_.hp_);
}


void CreatureStatus::upMpCapacity(ManaPoint mp)
{
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return;
        }
        effectApplyStatusInfo_.points_.mp_ = toManaPoint(effectApplyStatusInfo_.points_.mp_ + mp);        
        currentGameStatus_.points_.mp_ = toManaPoint(currentGameStatus_.points_.mp_ + mp);
    }

    maxPointChanged(ptMp, getCurrentPoints().mp_, currentGameStatus_.points_.mp_);
}


HitPoint CreatureStatus::downHpCapacity(HitPoint hp)
{
    HitPoint applyPoint = hp;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return hpMin;
        }

        if (currentGameStatus_.points_.hp_ < hp) {
            applyPoint = currentGameStatus_.points_.hp_; 
        }
        effectApplyStatusInfo_.points_.hp_ = toHitPoint(effectApplyStatusInfo_.points_.hp_ - applyPoint);        
        currentGameStatus_.points_.hp_ = toHitPoint(currentGameStatus_.points_.hp_ - applyPoint);

        getCurrentPoints().clampPoints(currentGameStatus_.points_);
        setCurrentPoint(getCurrentPoints());
    }

    maxPointChanged(ptHp, getCurrentPoints().hp_, currentGameStatus_.points_.hp_);

    return applyPoint;
}


ManaPoint CreatureStatus::downMpCapacity(ManaPoint mp)
{
    ManaPoint applyPoint = mp;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! unionCreatureInfo_.isValid()) {
            return mpMin;
        }
        
        if (currentGameStatus_.points_.mp_ < mp) {
            applyPoint = currentGameStatus_.points_.mp_; 
        }

        effectApplyStatusInfo_.points_.mp_ = toManaPoint(effectApplyStatusInfo_.points_.mp_ - applyPoint);        
        currentGameStatus_.points_.mp_ = toManaPoint(currentGameStatus_.points_.mp_ - applyPoint);

        getCurrentPoints().clampPoints(currentGameStatus_.points_);
        setCurrentPoint(getCurrentPoints());
    }

    maxPointChanged(ptMp, getCurrentPoints().mp_, currentGameStatus_.points_.mp_);

    return applyPoint;
}


Points CreatureStatus::getMaxPoints() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return currentGameStatus_.points_;         
}


bool CreatureStatus::isDied() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! unionCreatureInfo_.isValid()) {
        return true;
    }
    return getCurrentPoints().isDied();
}


bool CreatureStatus::isHpFullyRestored() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    if (! unionCreatureInfo_.isValid()) {
        return true;
    }
    return getCurrentPoints().hp_ >= currentGameStatus_.points_.hp_;
}



// = virtual

void CreatureStatus::initialize()
{
    lastDeadTime_ = 0;
}


void CreatureStatus::finalize()
{
}


bool CreatureStatus::reduceHp_i(HitPoint& applyPoint, HitPoint hp, bool canDie)
{
    Points& currentPoints = getCurrentPoints();
    const HitPoint prevHp = currentPoints.hp_;

    currentPoints.reduceHp(applyPoint, hp, canDie);

    const bool isDied = currentPoints.isDied();
    if (isDied) {
        lastDeadTime_ = GAME_TIMER->msec();
    }

    return currentPoints.hp_ != prevHp;
}


bool CreatureStatus::reduceMp_i(ManaPoint& applyPoint,ManaPoint mp)
{
    Points& currentPoints = getCurrentPoints();
    const ManaPoint prevMp = currentPoints.mp_;

    currentPoints.reduceMp(applyPoint, mp);

    return currentPoints.mp_ != prevMp;
}


bool CreatureStatus::fillHp_i(HitPoint& applyPoint, HitPoint hp, bool force)
{
    if (isDied() && ! force) {
        return false;
    }

    Points& currentPoints = getCurrentPoints();
    const HitPoint prevHp = currentPoints.hp_;
    const HitPoint maxHp = getMaxPoints().hp_;
    currentPoints.fillHp(hp);
    if (maxHp < currentPoints.hp_) {
        HitPoint overPoint = toHitPoint(currentPoints.hp_ - maxHp);
        applyPoint = toHitPoint(applyPoint + overPoint);
        currentPoints.clampPoints(getMaxPoints());
    }
    else {
        applyPoint = toHitPoint(applyPoint + hp);
    }

    return currentPoints.hp_ != prevHp;
}


bool CreatureStatus::fillMp_i(ManaPoint& applyPoint, ManaPoint mp)
{
    Points& currentPoints = getCurrentPoints();
    const ManaPoint prevMp = currentPoints.mp_;
    const ManaPoint maxMp = getMaxPoints().mp_;
    currentPoints.fillMp(mp);
    if (maxMp < currentPoints.mp_) {
        ManaPoint overPoint = toManaPoint(currentPoints.mp_ - maxMp);
        applyPoint = toManaPoint(applyPoint + overPoint);
        currentPoints.clampPoints(getMaxPoints());
    }
    else {
        applyPoint = toManaPoint(applyPoint + mp);
    }

    return currentPoints.mp_ != prevMp;
}


void CreatureStatus::setHp_i(HitPoint hp, bool canDie)
{
    Points& currentPoints = getCurrentPoints();
    currentPoints.setHp(hp, canDie);
    if (getMaxPoints().hp_ < hp) {
        currentPoints.clampPoints(getMaxPoints());
    }
    const bool isDied = currentPoints.isDied();
    if (isDied) {
        lastDeadTime_ = GAME_TIMER->msec();
    }
}


void CreatureStatus::setMp_i(ManaPoint mp)
{
    Points& currentPoints = getCurrentPoints();
    currentPoints.setMp(mp);
    if (getMaxPoints().mp_ < mp) {
        currentPoints.clampPoints(getMaxPoints());
    }
}


void CreatureStatus::restorePoints_i()
{
    CreatureInfo& creatureInfo = unionCreatureInfo_.asCreatureInfo();
    creatureInfo.refillPoints(getMaxPoints());
}


void CreatureStatus::pointChanged(PointType pointType, uint32_t currentPoint)
{
    auto event = std::make_shared<PointChangedEvent>(
        owner_.getGameObjectInfo(), pointType, currentPoint);
    if (pointType == ptHp) {
        owner_.notifyToOberversOfObservers(event);
    }
    else {
        owner_.notifyToObervers(event);
        // TODO: MP가 대상에 표시가 된다면 notifyToOberversOfObservers() 호출로 변경해야 한다
    }

    Partyable* partyable = owner_.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyEvent(event, owner_.getObjectId(), true);
        }
    }
    go::PassiveSkillCastable* passiveCastable = owner_.queryPassiveSkillCastable();
    if (passiveCastable) {
        passiveCastable->notifyChangeCondition(pccPoints);
    }
}


void CreatureStatus::maxPointChanged(PointType pointType, uint32_t currentPoint,
    uint32_t maxPoint)
{
    auto event = std::make_shared<MaxPointChangedEvent>(
        owner_.getGameObjectInfo(), pointType, currentPoint, maxPoint);
    owner_.notifyToOberversOfObservers(event);

    Partyable* partyable = owner_.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyEvent(event, owner_.getObjectId(), true);
        }
    }
    go::PassiveSkillCastable* passiveCastable = owner_.queryPassiveSkillCastable();
    if (passiveCastable) {
        passiveCastable->notifyChangeCondition(pccPoints);
    }
}


void CreatureStatus::pointsRestored()
{
    auto event = std::make_shared<PointsRestoredEvent>(owner_.getGameObjectInfo());
    owner_.notifyToOberversOfObservers(event);

    Partyable* partyable = owner_.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyEvent(event, owner_.getObjectId(), true);
        }
    }
    go::PassiveSkillCastable* passiveCastable = owner_.queryPassiveSkillCastable();
    if (passiveCastable) {
        passiveCastable->notifyChangeCondition(pccPoints);
    }
}


void CreatureStatus::shieldPointChanged(PointType pointType, uint32_t currentPoint)
{
    auto event = std::make_shared<ShieldPointChangedEvent>(
        owner_.getGameObjectInfo(), pointType, currentPoint);
    owner_.notifyToOberversOfObservers(event);

    Partyable* partyable = owner_.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyEvent(event, owner_.getObjectId(), true);
        }
    }
}


void CreatureStatus::shieldCreated(PointType pointType, uint32_t value)
{
    auto event = std::make_shared<CreateShieldEvent>(
        owner_.getGameObjectInfo(), pointType, value);
    owner_.notifyToOberversOfObservers(event);

    Partyable* partyable = owner_.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyEvent(event, owner_.getObjectId(), true);
        }
    }
}


void CreatureStatus::shieldDestroyed(PointType pointType)
{
    auto event = std::make_shared<DestoryShieldEvent>(
        owner_.getGameObjectInfo(), pointType);
    owner_.notifyToOberversOfObservers(event);

    Partyable* partyable = owner_.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyEvent(event, owner_.getObjectId(), true);
        }
    }
}

}}} // namespace gideon { namespace zoneserver { namespace go {
