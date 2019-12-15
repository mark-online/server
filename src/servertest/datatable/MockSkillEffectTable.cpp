#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockSkillEffectTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

MockSkillEffectTable::MockSkillEffectTable()
{
    //fillSkillEffects();
}

const datatable::SkillEffectTemplate*
MockSkillEffectTable::getSkillEffectTemplate(EffectIndex effectIndex) const
{
    const SkillEffectMap::const_iterator pos = effectMap_.find(effectIndex);
    if (pos != effectMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}

}} //namespace gideon { namespace servertest {
