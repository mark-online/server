#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockDefaultEquipTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const MockDefaultEquipTable::Equips& MockDefaultEquipTable::getEquips(EquipPart part) const
{
    assert(isValid(part));
    const DefaultEquipMap::const_iterator pos = equipMap_.find(part);
    if (pos != equipMap_.end()) {
        return (*pos).second;
    }
    static const Equips nullEquips;
    return nullEquips;
}


void MockDefaultEquipTable::fillEquips()
{
    equipMap_[epRightHand].push_back(defaultOneHandSwordEquipCode);
    equipMap_[epLeftHand].push_back(defaultShieldEquipCode);
    equipMap_[epHelmet].push_back(defaultHelmetEquipCode);
    equipMap_[epTwoHands].push_back(lanceEquipCode);
}

}} // namespace gideon { namespace servertest {
