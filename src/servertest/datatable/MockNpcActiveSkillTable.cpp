#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockNpcActiveSkillTable.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/template/npc_active_skill_table.hxx>

namespace gideon { namespace servertest {

const datatable::NpcActiveSkillTemplate*
    MockNpcActiveSkillTable::getNpcSkill(SkillCode code) const
{
    const NpcSkillMap::const_iterator pos = skillMap_.find(code);
    if (pos != skillMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}

}} // namespace gideon { namespace servertest {
