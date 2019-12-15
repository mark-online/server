#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CombatStateCallback
 */
class CombatStateCallback
{
public:
    virtual void changeCombatState(bool isCombat) = 0;

    virtual void combatStateChanged(const GameObjectInfo& creatureInfo, bool isCombat) = 0;
};


}}} //namespace gideon { namespace zoneserver { namespace gc {
