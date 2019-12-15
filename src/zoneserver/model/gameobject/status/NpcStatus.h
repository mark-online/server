#pragma once

#include "CreatureStatus.h"
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class NpcStatus
 *
 */
class ZoneServer_Export NpcStatus : public CreatureStatus
{
public:
    NpcStatus(go::Creature& owner, UnionEntityInfo& unionCreatureInfo,
        FullNpcInfo& npcInfo) :
        CreatureStatus(owner, unionCreatureInfo),
        npcInfo_(npcInfo) {}

private:
    // = CreatureStatus overriding
    virtual bool reduceHp_i(HitPoint& applyPoint, HitPoint hp, bool canDie);
    virtual bool reduceMp_i(ManaPoint& applyPoint, ManaPoint mp);
    virtual bool fillHp_i(HitPoint& applyPoint, HitPoint hp, bool force);
    virtual bool fillMp_i(ManaPoint& applyPoint, ManaPoint mp);

    virtual void restorePoints_i();
private:
    virtual void equipItemEquipped(EquipCode /*equipCode*/, const EquipItemInfo& /*equipItemInfo*/) {}
    virtual void equipItemUnequipped(EquipCode /*equipCode*/, const EquipItemInfo& /*equipItemInfo*/) {}
    virtual void equipItemChanged(EquipCode /*equipCode*/, const EquipItemInfo& /*equipItemInfo*/, 
        EquipCode /*updateEquipCode*/, const EquipItemInfo& /*updateEquipItemInfo*/) {}
    virtual void setCurrentPoint(const Points& points);
    virtual void accessoryEquipped(AccessoryCode /*accessoryCode*/, const AccessoryItemInfo& /*info*/) {}
    virtual void accessoryUnequipped(AccessoryCode /*accessoryCode*/, const AccessoryItemInfo& /*info*/) {}
    virtual void accessoryChanged(AccessoryCode /*asscessoryCode*/, const AccessoryItemInfo& /*asscessoryItemInfo*/, 
        AccessoryCode /*updateAccessoryCode*/, const AccessoryItemInfo& /*updateAccessoryItemInfo*/) {}

    virtual void upBaseStatus(BaseStatusIndex index, int32_t value);
    virtual void downBaseStatus(BaseStatusIndex index, int32_t value);
    virtual void updateAllStatus();
    virtual void levelUp(const CurrentLevel& /*orgLevel*/) {}
    virtual void changeCharacterClass(CharacterClass /*orgClass*/) {}
private:
    FullNpcInfo& npcInfo_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
