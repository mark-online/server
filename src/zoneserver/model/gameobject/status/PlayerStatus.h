#pragma once

#include "CreatureStatus.h"
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class PlayerStatus
 *
 */
class ZoneServer_Export PlayerStatus : public CreatureStatus
{
public:
    PlayerStatus(go::Creature& owner, UnionEntityInfo& unionCreatureInfo,
        FullCharacterInfo& characterInfo) :
        CreatureStatus(owner, unionCreatureInfo),
        characterInfo_(characterInfo) {}    

public:
    // = 치트 전용
    void setZeroAllCurrentStatus();
    void restoreCurrentStatus();
    void setCurrentBaseStatus(BaseStatusIndex index, int32_t value);
    void setCurrentAttackStatus(AttackStatusIndex index, int16_t value);    
    void setCurrentAttackPowers(AttackPowerIndex index, int32_t value);    
    void setCurrentDefenceStatus(DefenceStatusIndex Index, int16_t  value);    
    void setCurrentAttribute(AttributeRateIndex index, int16_t value, bool isResist);

private:
    // = CreatureStatus overriding

    virtual void finalize();    
    virtual bool reduceHp_i(HitPoint& applyPoint, HitPoint hp, bool canDie);
    virtual bool reduceMp_i(ManaPoint& applyPoint, ManaPoint mp);
    virtual bool fillHp_i(HitPoint& applyPoint, HitPoint hp, bool force);
    virtual bool fillMp_i(ManaPoint& applyPoint,ManaPoint mp);
    
    virtual void setHp_i(HitPoint hp, bool canDie);
    virtual void setMp_i(ManaPoint mp);
    
    virtual void restorePoints_i();

private:
    virtual void equipItemEquipped(EquipCode equipCode, const EquipItemInfo& equipItemInfo);
    virtual void equipItemUnequipped(EquipCode equipCode, const EquipItemInfo& equipItemInfo);
    virtual void equipItemChanged(EquipCode equipCode, const EquipItemInfo& equipItemInfo, 
        EquipCode updateEquipCode, const EquipItemInfo& updateEquipItemInfo);
    virtual void accessoryEquipped(AccessoryCode accessoryCode, const AccessoryItemInfo& accessoryItemInfo);
    virtual void accessoryUnequipped(AccessoryCode accessoryCode, const AccessoryItemInfo& accessoryItemInfo);
    virtual void accessoryChanged(AccessoryCode asscessoryCode, const AccessoryItemInfo& asscessoryItemInfo, 
        AccessoryCode updateAccessoryCode, const AccessoryItemInfo& updateAccessoryItemInfo);

    virtual void upBaseStatus(BaseStatusIndex index, int32_t value);
    virtual void downBaseStatus(BaseStatusIndex index, int32_t value);

    //virtual void pointChanged(PointType pointType, uint32_t currentPoint);
    //virtual void pointsRestored();

    virtual void setCurrentPoint(const Points& points);
    virtual void updateAllStatus();

    virtual void levelUp(const CurrentLevel& orgLevel);
    virtual void changeCharacterClass(CharacterClass orgClass);

private:
    FullCharacterInfo& characterInfo_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
