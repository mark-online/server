#include "ZoneServerPCH.h"
#include "PlayerStatus.h"
#include "../Creature.h"
#include "../EntityEvent.h"
#include "../ability/Partyable.h"
#include "../../../service/party/Party.h"
#include "../../../controller/callback/StatsCallback.h"
#include "../../../controller/callback/StatsCallback.h"
#include "../../../controller/EntityController.h"
#include <gideon/cs/shared/data/PlayerStatusInfo.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/CharacterStatusTable.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace go {


namespace {
        
/**
 * @class CreatureStatusChangeEvent
 */
class CreatureStatusChangeEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CreatureStatusChangeEvent>
{
public:
    CreatureStatusChangeEvent(const GameObjectInfo& entityInfo, 
        const LifeStats& lifeStats) :
        entityInfo_(entityInfo),
        creatureStatus_(lifeStats) {}

private:
    virtual void call(go::Entity& entity) {
        gc::StatsCallback* callback = entity.getController().queryStatsCallback();
        if (callback) {
            callback->creatureLifeStatsChanged(entityInfo_, creatureStatus_);
        }   
    }

private:
    const GameObjectInfo entityInfo_;
    const LifeStats creatureStatus_;
};

}

// = CreatureStatus overriding

void PlayerStatus::finalize()
{
    CreatureStatus::finalize();
}


void PlayerStatus::setZeroAllCurrentStatus()
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        CreatureStatusInfo& creatureStatusInfo = getCreatureStatusInfo();
        CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
        CreatureStatusInfo& effectApplyStatusInfo = getEffectApplyStatusInfo();

        currentStatusInfo.baseStatus_.fill(0);
        currentStatusInfo.attackPowers_.fill(0);
        currentStatusInfo.attackStatus_.fill(0);
        currentStatusInfo.defenceStatus_.fill(0);
        currentStatusInfo.attributes_.fill(0);
        currentStatusInfo.attributeResists_.fill(0);

        currentStatusInfo.points_.hp_ = toHitPoint(creatureStatusInfo.points_.hp_ + effectApplyStatusInfo.points_.hp_);
        currentStatusInfo.points_.mp_ = toManaPoint(creatureStatusInfo.points_.mp_ + effectApplyStatusInfo.points_.mp_);
        characterInfo_.currentPoints_.clampPoints(currentStatusInfo.points_);
        getCurrentPoints() = characterInfo_.currentPoints_;
    }

    // TODO: CreatureStatusChangeEvent 호출 코드 중복 제거
    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    gc::StatsCallback* statCallback = getOwner().getController().queryStatsCallback();
    if (statCallback != nullptr) {
        statCallback->fullCreatureStatusChanged();
    }
}


void PlayerStatus::restoreCurrentStatus()
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        updateAllStatus();
        getCurrentPoints() = characterInfo_.currentPoints_;
    }

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    gc::StatsCallback* statCallback = getOwner().getController().queryStatsCallback();
    if (statCallback != nullptr) {
        statCallback->fullCreatureStatusChanged();
    }
}


void PlayerStatus::setCurrentBaseStatus(BaseStatusIndex index, int32_t value)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        CreatureStatusInfo& creatureStatusInfo = getCreatureStatusInfo();
        CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
        CreatureStatusInfo& effectApplyStatusInfo = getEffectApplyStatusInfo();

        currentStatusInfo.baseStatus_[index] = value;
        const int32_t strength = currentStatusInfo.baseStatus_[bsiStrength];
        const int32_t dexterity = currentStatusInfo.baseStatus_[bsiDexterity];
        const int32_t intellect = currentStatusInfo.baseStatus_[bsiIntellect];
        const int32_t energy = currentStatusInfo.baseStatus_[bsiEnergy];

        const HitPoint statusHp = toHitPoint(getHitPoint(characterInfo_.characterClass_, strength, dexterity, energy) +
            creatureStatusInfo.points_.hp_ + effectApplyStatusInfo.points_.hp_);
        const ManaPoint statusMp = toManaPoint(getManaPoint(characterInfo_.characterClass_, intellect, energy) +
            creatureStatusInfo.points_.mp_ + effectApplyStatusInfo.points_.mp_);

        const int32_t statusMinAttack = creatureStatusInfo.attackPowers_[apiMinAttack] + effectApplyStatusInfo.attackPowers_[apiMinAttack];
        const int32_t statusMaxAttack = creatureStatusInfo.attackPowers_[apiMaxAttack] + effectApplyStatusInfo.attackPowers_[apiMaxAttack];

        const int32_t statusMagicPower = getMagicPower(characterInfo_.characterClass_, intellect) +
            creatureStatusInfo.attackPowers_[apiMagic] + effectApplyStatusInfo.attackPowers_[apiMagic];
        const int32_t statusPhysicalPower = getPhysicalPower(characterInfo_.characterClass_, strength, dexterity) +
            creatureStatusInfo.attackPowers_[apiPhysical] + effectApplyStatusInfo.attackPowers_[apiPhysical];

        const int16_t statusHitRate =
            creatureStatusInfo.attackStatus_[atsiHitRate] + effectApplyStatusInfo.attackStatus_[atsiHitRate];
        const int16_t statusPhysicalCriticalRate = getPhysicalCriticalRate(currentStatusInfo.baseStatus_) +
            creatureStatusInfo.attackStatus_[atsiPhysicalCriticalRate] + effectApplyStatusInfo.attackStatus_[atsiPhysicalCriticalRate];
        const int16_t statusMagicCriticalRate = getMagicCriticalRate(currentStatusInfo.baseStatus_) +
            creatureStatusInfo.attackStatus_[atsiMagicCriticalRate] + effectApplyStatusInfo.attackStatus_[atsiMagicCriticalRate];

        const int16_t statusParryRate = getParryRate(currentStatusInfo.baseStatus_) +
            creatureStatusInfo.defenceStatus_[dsiParryRate] + effectApplyStatusInfo.defenceStatus_[dsiParryRate];
        const int16_t statusBlockRate = getBlockRate(currentStatusInfo.baseStatus_) +
            creatureStatusInfo.defenceStatus_[dsiBlockRate] + effectApplyStatusInfo.defenceStatus_[dsiBlockRate];
        const int16_t statusDodgeRate = getDodgeRate(currentStatusInfo.baseStatus_) +
            creatureStatusInfo.defenceStatus_[dsiDodgeRate] + effectApplyStatusInfo.defenceStatus_[dsiDodgeRate];
        const int16_t statusDefencevalue = getDefencevalue(characterInfo_.characterClass_, strength, energy) +
            creatureStatusInfo.defenceStatus_[dsiDefence] + effectApplyStatusInfo.defenceStatus_[dsiDefence];

        currentStatusInfo.points_.hp_ = statusHp;
        currentStatusInfo.points_.mp_ = statusMp;

        currentStatusInfo.attackPowers_[apiMinAttack] = statusMinAttack;
        currentStatusInfo.attackPowers_[apiMaxAttack] = statusMaxAttack;
        currentStatusInfo.attackPowers_[apiPhysical] = statusPhysicalPower;
        currentStatusInfo.attackPowers_[apiMagic] = statusMagicPower;

        currentStatusInfo.attackStatus_[atsiHitRate] = statusHitRate;
        currentStatusInfo.attackStatus_[atsiPhysicalCriticalRate] = statusPhysicalCriticalRate;
        currentStatusInfo.attackStatus_[atsiMagicCriticalRate] = statusMagicCriticalRate;

        currentStatusInfo.defenceStatus_[dsiParryRate] = statusParryRate;
        currentStatusInfo.defenceStatus_[dsiBlockRate] = statusBlockRate;
        currentStatusInfo.defenceStatus_[dsiDodgeRate] = statusDodgeRate;
        currentStatusInfo.defenceStatus_[dsiDefence] = statusDefencevalue;

        characterInfo_.currentPoints_.clampPoints(currentStatusInfo.points_);
        getCurrentPoints() = characterInfo_.currentPoints_;
    }

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    gc::StatsCallback* statCallback = getOwner().getController().queryStatsCallback();
    if (statCallback != nullptr) {
        statCallback->fullCreatureStatusChanged();
    }
}
 

void PlayerStatus::setCurrentAttackStatus(AttackStatusIndex index, int16_t value)    
{

    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

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
        CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
        currentStatusInfo.attackStatus_[index] = value;
        currentValue = currentStatusInfo.attackStatus_[index];        
    }

    gc::StatsCallback* callback = getOwner().getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void PlayerStatus::setCurrentAttackPowers(AttackPowerIndex index, int32_t value)    
{    
    EffectStatusType effectStatusType = esttNone;
    int32_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

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
        CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
        currentStatusInfo.attackPowers_[index] = value;
        currentValue = currentStatusInfo.attackPowers_[index];
    }

    gc::StatsCallback* callback = getOwner().getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void PlayerStatus::setCurrentDefenceStatus(DefenceStatusIndex index, int16_t  value)    
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

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

        CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
        currentStatusInfo.defenceStatus_[index] += value;
        currentValue = currentStatusInfo.defenceStatus_[index];
    }
    gc::StatsCallback* callback = getOwner().getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


void PlayerStatus::setCurrentAttribute(AttributeRateIndex index, int16_t value, bool isResist)
{
    EffectStatusType effectStatusType = esttNone;
    int16_t currentValue = 0;
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

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
            CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
            currentStatusInfo.attributeResists_[index] = value;
            currentValue = currentStatusInfo.attributeResists_[index];
        }
        else {
            CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
            currentStatusInfo.attributes_[index] = value;
            currentValue = currentStatusInfo.attributes_[index];
        }
    }

    gc::StatsCallback* callback = getOwner().getController().queryStatsCallback();
    if (callback) {
        callback->creatureStatusChanged(effectStatusType, currentValue);
    }
}


bool PlayerStatus::reduceHp_i(HitPoint& applyPoint, HitPoint hp, bool canDie)
{
    const bool hpChanged = CreatureStatus::reduceHp_i(applyPoint, hp, canDie);

    characterInfo_.currentPoints_.hp_ = getCurrentPoints().hp_;
    return hpChanged;
}


bool PlayerStatus::reduceMp_i(ManaPoint& applyPoint, ManaPoint mp)
{
    const bool mpChanged = CreatureStatus::reduceMp_i(applyPoint, mp);

    characterInfo_.currentPoints_.mp_ = getCurrentPoints().mp_;
    return mpChanged;
}



bool PlayerStatus::fillHp_i(HitPoint& applyPoint, HitPoint hp, bool force)
{
    const bool hpChanged = CreatureStatus::fillHp_i(applyPoint, hp, force);

    characterInfo_.currentPoints_.hp_ = getCurrentPoints().hp_;
    return hpChanged;
}


bool PlayerStatus::fillMp_i(ManaPoint& applyPoint,ManaPoint mp)
{
    const bool mpChanged = CreatureStatus::fillMp_i(applyPoint, mp);

    characterInfo_.currentPoints_.mp_ = getCurrentPoints().mp_;
    return mpChanged;
}


void PlayerStatus::setHp_i(HitPoint hp, bool canDie)
{
    CreatureStatus::setHp_i(hp, canDie);
    characterInfo_.currentPoints_.hp_ = getCurrentPoints().hp_;
}


void PlayerStatus::setMp_i(ManaPoint mp)
{
    CreatureStatus::setMp_i(mp);
    characterInfo_.currentPoints_.mp_ = getCurrentPoints().mp_;
}


void PlayerStatus::restorePoints_i()
{
    CreatureStatus::restorePoints_i();

    characterInfo_.currentPoints_ = getCurrentPoints();
}


void PlayerStatus::equipItemEquipped(EquipCode equipCode, const EquipItemInfo& equipItemInfo)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        EQUIP_TABLE->applyCreatureStatusInfoByEquip(getCreatureStatusInfo(), equipCode, equipItemInfo);
        updateAllStatus();
    }

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    Partyable* partyable = getOwner().queryPartyable();
    PartyPtr party = partyable->getParty();
    if (party.get() != nullptr) {
        party->notifyEvent(event, getOwner().getObjectId(), true);
    }
}


void PlayerStatus::equipItemUnequipped(EquipCode equipCode, const EquipItemInfo& equipItemInfo)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        EQUIP_TABLE->applyCreatureStatusInfoByUnequip(getCreatureStatusInfo(), equipCode, equipItemInfo);
        updateAllStatus();
    }
    getCurrentPoints() = characterInfo_.currentPoints_;

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    Partyable* partyable = getOwner().queryPartyable();
    PartyPtr party = partyable->getParty();
    if (party.get() != nullptr) {
        party->notifyEvent(event, getOwner().getObjectId(), true);
    }
}


void PlayerStatus::equipItemChanged(EquipCode equipCode, const EquipItemInfo& equipItemInfo, 
    EquipCode updateEquipCode, const EquipItemInfo& updateEquipItemInfo)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        EQUIP_TABLE->applyCreatureStatusInfoByUnequip(getCreatureStatusInfo(), equipCode, equipItemInfo);
        EQUIP_TABLE->applyCreatureStatusInfoByEquip(getCreatureStatusInfo(), updateEquipCode, updateEquipItemInfo);
        updateAllStatus();
    }
    getCurrentPoints() = characterInfo_.currentPoints_;

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);
}


void PlayerStatus::accessoryEquipped(AccessoryCode accessoryCode, const AccessoryItemInfo& accessoryItemInfo)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        ACCESSORY_TABLE->applyCreatureStatusInfoByEquip(getCreatureStatusInfo(), accessoryCode, accessoryItemInfo);
        updateAllStatus();
    }
    getCurrentPoints() = characterInfo_.currentPoints_;

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_,  getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    Partyable* partyable = getOwner().queryPartyable();
    PartyPtr party = partyable->getParty();
    if (party.get() != nullptr) {
        party->notifyEvent(event, getOwner().getObjectId(), true);
    }
}


void PlayerStatus::accessoryUnequipped(AccessoryCode accessoryCode, const AccessoryItemInfo& accessoryItemInfo)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        ACCESSORY_TABLE->applyCreatureStatusInfoByUnequip(getCreatureStatusInfo(), accessoryCode, accessoryItemInfo);
        updateAllStatus();
    }
    getCurrentPoints() = characterInfo_.currentPoints_;

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    Partyable* partyable = getOwner().queryPartyable();
    PartyPtr party = partyable->getParty();
    if (party.get() != nullptr) {
        party->notifyEvent(event, getOwner().getObjectId(), true);
    }
}

void PlayerStatus::accessoryChanged(AccessoryCode asscessoryCode, const AccessoryItemInfo& asscessoryItemInfo, 
    AccessoryCode updateAccessoryCode, const AccessoryItemInfo& updateAccessoryItemInfo)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        ACCESSORY_TABLE->applyCreatureStatusInfoByUnequip(getCreatureStatusInfo(), asscessoryCode, asscessoryItemInfo);
        ACCESSORY_TABLE->applyCreatureStatusInfoByEquip(getCreatureStatusInfo(), updateAccessoryCode, updateAccessoryItemInfo);
        updateAllStatus();
    }
    getCurrentPoints() = characterInfo_.currentPoints_;
    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);
}


void PlayerStatus::upBaseStatus(BaseStatusIndex index, int32_t value)
{
    std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

    getCreatureStatusInfo().baseStatus_[index] += value;
    updateAllStatus();
}


void PlayerStatus::downBaseStatus(BaseStatusIndex index, int32_t value)
{
    std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

    getCreatureStatusInfo().baseStatus_[index] -= value;
    updateAllStatus();
}


//void PlayerStatus::pointChanged(PointType pointType, uint32_t currentPoint)
//{
//    CreatureStatus::pointChanged(pointType, currentPoint);
//}
//
//
//void PlayerStatus::pointsRestored()
//{
//    CreatureStatus::pointsRestored();
//}


void PlayerStatus::setCurrentPoint(const Points& points)
{
    characterInfo_.currentPoints_ = points;
}


void PlayerStatus::updateAllStatus()
{
    CreatureStatusInfo& creatureStatusInfo = getCreatureStatusInfo();
    CreatureStatusInfo& currentStatusInfo = getCurrentCreatureStatusInfo();
    CreatureStatusInfo& effectApplyStatusInfo = getEffectApplyStatusInfo();

    const int32_t strength = creatureStatusInfo.baseStatus_[bsiStrength] + effectApplyStatusInfo.baseStatus_[bsiStrength];
    const int32_t dexterity = creatureStatusInfo.baseStatus_[bsiDexterity] + effectApplyStatusInfo.baseStatus_[bsiDexterity];
    const int32_t intellect = creatureStatusInfo.baseStatus_[bsiIntellect] + effectApplyStatusInfo.baseStatus_[bsiIntellect];
    const int32_t energy = creatureStatusInfo.baseStatus_[bsiEnergy] + effectApplyStatusInfo.baseStatus_[bsiEnergy];
    currentStatusInfo.baseStatus_[bsiStrength] = strength;
    currentStatusInfo.baseStatus_[bsiDexterity] = dexterity;
    currentStatusInfo.baseStatus_[bsiIntellect] = intellect;
    currentStatusInfo.baseStatus_[bsiEnergy] = energy;

    const HitPoint statusHp = toHitPoint(getHitPoint(characterInfo_.characterClass_, strength, dexterity, energy) +
        creatureStatusInfo.points_.hp_ + effectApplyStatusInfo.points_.hp_);
    const ManaPoint statusMp = toManaPoint(getManaPoint(characterInfo_.characterClass_, intellect, energy) +
        creatureStatusInfo.points_.mp_ + effectApplyStatusInfo.points_.mp_);

    const int32_t statusMinAttack = creatureStatusInfo.attackPowers_[apiMinAttack] + effectApplyStatusInfo.attackPowers_[apiMinAttack];
    const int32_t statusMaxAttack = creatureStatusInfo.attackPowers_[apiMaxAttack] + effectApplyStatusInfo.attackPowers_[apiMaxAttack];

    const int32_t statusMagicPower = getMagicPower(characterInfo_.characterClass_, intellect) +
        creatureStatusInfo.attackPowers_[apiMagic] + effectApplyStatusInfo.attackPowers_[apiMagic];
    const int32_t statusPhysicalPower = getPhysicalPower(characterInfo_.characterClass_, strength, dexterity) +
        creatureStatusInfo.attackPowers_[apiPhysical] + effectApplyStatusInfo.attackPowers_[apiPhysical];

    const int16_t statusHitRate =
        creatureStatusInfo.attackStatus_[atsiHitRate] + effectApplyStatusInfo.attackStatus_[atsiHitRate];
    const int16_t statusPhysicalCriticalRate = getPhysicalCriticalRate(currentStatusInfo.baseStatus_) +
        creatureStatusInfo.attackStatus_[atsiPhysicalCriticalRate] + effectApplyStatusInfo.attackStatus_[atsiPhysicalCriticalRate];
    const int16_t statusMagicCriticalRate = getMagicCriticalRate(currentStatusInfo.baseStatus_) +
        creatureStatusInfo.attackStatus_[atsiMagicCriticalRate] + effectApplyStatusInfo.attackStatus_[atsiMagicCriticalRate];

    const int16_t statusParryRate = getParryRate(currentStatusInfo.baseStatus_) +
        creatureStatusInfo.defenceStatus_[dsiParryRate] + effectApplyStatusInfo.defenceStatus_[dsiParryRate];
    const int16_t statusBlockRate = getBlockRate(currentStatusInfo.baseStatus_) +
        creatureStatusInfo.defenceStatus_[dsiBlockRate] + effectApplyStatusInfo.defenceStatus_[dsiBlockRate];
    const int16_t statusDodgeRate = getDodgeRate(currentStatusInfo.baseStatus_) +
        creatureStatusInfo.defenceStatus_[dsiDodgeRate] + effectApplyStatusInfo.defenceStatus_[dsiDodgeRate];
    const int16_t statusDefencevalue = getDefencevalue(characterInfo_.characterClass_, strength, energy) +
        creatureStatusInfo.defenceStatus_[dsiDefence] + effectApplyStatusInfo.defenceStatus_[dsiDefence];

    currentStatusInfo.points_.hp_ = statusHp;
    currentStatusInfo.points_.mp_ = statusMp;

    currentStatusInfo.attackPowers_[apiMinAttack] = statusMinAttack;
    currentStatusInfo.attackPowers_[apiMaxAttack] = statusMaxAttack;
    currentStatusInfo.attackPowers_[apiPhysical] = statusPhysicalPower;
    currentStatusInfo.attackPowers_[apiMagic] = statusMagicPower;
    
    currentStatusInfo.attackStatus_[atsiHitRate] = statusHitRate;
    currentStatusInfo.attackStatus_[atsiPhysicalCriticalRate] = statusPhysicalCriticalRate;
    currentStatusInfo.attackStatus_[atsiMagicCriticalRate] = statusMagicCriticalRate;

    currentStatusInfo.defenceStatus_[dsiParryRate] = statusParryRate;
    currentStatusInfo.defenceStatus_[dsiBlockRate] = statusBlockRate;
    currentStatusInfo.defenceStatus_[dsiDodgeRate] = statusDodgeRate;
    currentStatusInfo.defenceStatus_[dsiDefence] = statusDefencevalue;

    currentStatusInfo.attributes_[abiLight] = creatureStatusInfo.attributes_[abiLight] + effectApplyStatusInfo.attributes_[abiLight];
    currentStatusInfo.attributes_[abiDark] = creatureStatusInfo.attributes_[abiDark] + effectApplyStatusInfo.attributes_[abiDark];
    currentStatusInfo.attributes_[abiFire] = creatureStatusInfo.attributes_[abiFire] + effectApplyStatusInfo.attributes_[abiFire];
    currentStatusInfo.attributes_[abiIce] = creatureStatusInfo.attributes_[abiIce] + effectApplyStatusInfo.attributes_[abiIce];

    currentStatusInfo.attributeResists_[abiLight] = creatureStatusInfo.attributeResists_[abiLight] + effectApplyStatusInfo.attributeResists_[abiLight];
    currentStatusInfo.attributeResists_[abiDark] = creatureStatusInfo.attributeResists_[abiDark] + effectApplyStatusInfo.attributeResists_[abiDark];
    currentStatusInfo.attributeResists_[abiFire] = creatureStatusInfo.attributeResists_[abiFire] + effectApplyStatusInfo.attributeResists_[abiFire];
    currentStatusInfo.attributeResists_[abiIce] = creatureStatusInfo.attributeResists_[abiIce] + effectApplyStatusInfo.attributeResists_[abiIce];

    characterInfo_.currentPoints_.clampPoints(currentStatusInfo.points_);
}


void PlayerStatus::levelUp(const CurrentLevel& orgLevel)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        CreatureStatusInfo& creatureStatus = getCreatureStatusInfo();
        CHARACTER_STATUS_TABLE->applyCreatureStatusInfo(characterInfo_.characterClass_, 
            creatureStatus.baseStatus_, orgLevel.level_, orgLevel.levelStep_, 
            characterInfo_.currentLevel_.level_, characterInfo_.currentLevel_.levelStep_);
        updateAllStatus();
    }
    
    getCurrentPoints() = characterInfo_.currentPoints_;

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_,  getCurrentCreatureStatusInfo().points_));

    Partyable* partyable = getOwner().queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyEvent(event, getOwner().getObjectId(), true);
        }
    }
    getOwner().notifyToOberversOfObservers(event, true);

    getOwner().getController().queryStatsCallback()->fullCreatureStatusChanged();
}


void PlayerStatus::changeCharacterClass(CharacterClass orgClass)
{
    {
        std::lock_guard<go::Entity::LockType> lock(getOwner().getLock());

        CreatureStatusInfo& creatureStatus = getCreatureStatusInfo();
        CHARACTER_STATUS_TABLE->applyCreatureStatusInfo(orgClass, characterInfo_.characterClass_, 
            creatureStatus.baseStatus_, characterInfo_.currentLevel_.level_, characterInfo_.currentLevel_.levelStep_);
        updateAllStatus();
    }

    getCurrentPoints() = characterInfo_.currentPoints_;

    auto event = std::make_shared<CreatureStatusChangeEvent>(
        getOwner().getGameObjectInfo(),
        LifeStats(characterInfo_.currentPoints_, getCurrentCreatureStatusInfo().points_));
    getOwner().notifyToOberversOfObservers(event, true);

    getOwner().getController().queryStatsCallback()->fullCreatureStatusChanged();
}

}}} // namespace gideon { namespace zoneserver { namespace go {
