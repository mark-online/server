#pragma once

#include "../../../zoneserver_export.h"
#include <gideon/cs/shared/data/UnionEntityInfo.h>
#include <gideon/cs/shared/data/Time.h>

namespace gideon { namespace zoneserver { namespace go {


class Creature;

/**
 * @class CreatureStatus
 * - Non Virtual Interface idiom
 */
class ZoneServer_Export CreatureStatus : public boost::noncopyable
{
public:
    CreatureStatus(go::Creature& owner, UnionEntityInfo& unionCreatureInfo);

    virtual ~CreatureStatus() {}

public:
    go::Creature& getOwner() {
        return owner_;
    }

    const go::Creature& getOwner() const {
        return owner_;
    }

    GameTime getLastDeadTime() const {
        return lastDeadTime_;
    }

    FullCreatureStatusInfo getFullCreatureStatusInfo();

    CreatureStatusInfo& getCurrentCreatureStatusInfo() {
        return currentGameStatus_;
    }

    const CreatureStatusInfo& getCurrentCreatureStatusInfo() const {
        return currentGameStatus_;
    }

    CreatureStatusInfo& getCreatureStatusInfo() {
        return creatureStatusInfo_;
    }

    const CreatureStatusInfo& getCreatureStatusInfo() const {
        return creatureStatusInfo_;
    }

    LifeStats getCurrentLifeStats() const;
    HitPoints getHitPoints() const;
    ManaPoints getManaPoints() const;

    uint8_t getHitPointRatio() const {
        const HitPoints hitPoints = getHitPoints();
        return uint8_t((hitPoints.hp_ * 100) / hitPoints.maxHp_);
    }

    ManaPoint getManaPointPer() const {
        return toManaPoint(currentGameStatus_.points_.mp_ / 100);
    }

public:
    /// @return hp changed?
    bool reduceHp(HitPoint& applyPoint, HitPoint hp, bool canDie);
    void reduceMp(ManaPoint& applyPoint, ManaPoint mp);
    bool reduceHp(HitPoint hp, bool canDie);
    void reduceMp(ManaPoint mp);

    void fillHp(HitPoint& applyPoint, HitPoint hp, bool force = false);
    void fillMp(ManaPoint& applyPoint, ManaPoint mp);
    void fillHp(HitPoint hp, bool force = false);
    void fillMp(ManaPoint mp);

	void setHp(HitPoint hp, bool canDie);
	void setMp(ManaPoint mp);

    void restorePoints(bool shouldNotify = true);

    void createShield(PointType pointType, uint32_t value);
    void destoryShield(PointType pointType);

    void upAttackStatus(AttackStatusIndex index, int16_t value);
    void downAttackStatus(AttackStatusIndex index, int16_t value);

    void upAttackPowers(AttackPowerIndex index, int32_t value);
    void downAttackPowers(AttackPowerIndex index, int32_t value);

    void upDefenceStatus(DefenceStatusIndex Index, int16_t  value);
    void downDefenceStatus(DefenceStatusIndex index, int16_t  value);

    void upAttribute(AttributeRateIndex index, int16_t value, bool isResist);
    void upAllAttribute(int16_t value, bool isResist);
    void downAttribute(AttributeRateIndex index, int16_t value, bool isResist);
    void downAllAttribute(int16_t value, bool isResist);
    
public:
    // effect status 스킬로 넘어온다
    void upHpCapacity(HitPoint hp);
    void upMpCapacity(ManaPoint mp);
    HitPoint downHpCapacity(HitPoint hp);
    ManaPoint downMpCapacity(ManaPoint mp);
    const CreatureShields& getCreatureShields() const {
        return shields_;
    }

public:
    Points getMaxPoints() const;
    
    bool isDied() const;
    bool isHpFullyRestored() const;
    bool isCpFullyRecovered() const;

public:
    virtual void initialize();
    virtual void finalize();    

protected:
    /// @return hp changed?
    virtual bool reduceHp_i(HitPoint& applyPoint, HitPoint hp, bool canDie);
    /// @return mp changed?
    virtual bool reduceMp_i(ManaPoint& applyPoint, ManaPoint mp);
    /// @return cp changed?

    /// @return hp changed?
    virtual bool fillHp_i(HitPoint& applyPoint, HitPoint hp, bool force);
    /// @return mp changed?
    virtual bool fillMp_i(ManaPoint& applyPoint, ManaPoint mp);
    /// @return cp changed?


    virtual void setHp_i(HitPoint hp, bool canDie);
    virtual void setMp_i(ManaPoint mp);

    virtual void restorePoints_i();

public:
    virtual void equipItemEquipped(EquipCode equipCode, const EquipItemInfo& equipItemInfo) = 0;
    virtual void equipItemUnequipped(EquipCode equipCode, const EquipItemInfo& equipItemInfo) = 0;
    virtual void equipItemChanged(EquipCode equipCode, const EquipItemInfo& equipItemInfo, 
        EquipCode updateEquipCode, const EquipItemInfo& updateEquipItemInfo) = 0;
    virtual void setCurrentPoint(const Points& points) = 0;
    virtual void accessoryEquipped(AccessoryCode accessoryCode, const AccessoryItemInfo& accessoryItemInfo) = 0;
    virtual void accessoryUnequipped(AccessoryCode accessoryCode, const AccessoryItemInfo& accessoryItemInfo) = 0;
    virtual void accessoryChanged(AccessoryCode asscessoryCode, const AccessoryItemInfo& asscessoryItemInfo, 
        AccessoryCode updateAccessoryCode, const AccessoryItemInfo& updateAccessoryItemInfo) = 0;

    virtual void upBaseStatus(BaseStatusIndex index, int32_t value) = 0;
    virtual void downBaseStatus(BaseStatusIndex index, int32_t value) = 0;
    virtual void updateAllStatus() = 0;
    virtual void levelUp(const CurrentLevel& orgLevel) = 0;
    virtual void changeCharacterClass(CharacterClass orgClass) = 0;
protected:
    virtual void pointChanged(PointType pointType, uint32_t currentPoint);
    virtual void maxPointChanged(PointType pointType, uint32_t currentPoint,
        uint32_t maxPoint);
    virtual void pointsRestored();

private:
    void shieldPointChanged(PointType pointType, uint32_t currentPoint);

    void shieldCreated(PointType pointType, uint32_t value);
    void shieldDestroyed(PointType pointType);


protected:
    const Points& getCurrentPoints() const {
        return unionCreatureInfo_.asCreatureInfo().currentPoints_;
    }

    Points& getCurrentPoints() {
        return unionCreatureInfo_.asCreatureInfo().currentPoints_;
    }
    
    const CreatureStatusInfo& getEffectApplyStatusInfo() const {
        return effectApplyStatusInfo_;
    }

    CreatureStatusInfo& getEffectApplyStatusInfo() {
        return effectApplyStatusInfo_;
    }

private:
    go::Creature& owner_;
    CreatureStatusInfo currentGameStatus_; // 장비 + 캐릭터 + 효과
    CreatureStatusInfo creatureStatusInfo_; // 장비 + 케릭터
    CreatureStatusInfo effectApplyStatusInfo_; // 효과
    UnionEntityInfo& unionCreatureInfo_;
    GameTime lastDeadTime_;
    CreatureShields shields_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
