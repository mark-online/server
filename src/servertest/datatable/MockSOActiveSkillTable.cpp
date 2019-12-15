#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockSOActiveSkillTable.h>
//#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/template/so_active_skill_table.hxx>

namespace gideon { namespace servertest {

const datatable::SOActiveSkillTemplate*
    MockSOActiveSkillTable::getSOSkill(SkillCode code) const
{
    const SOSkillMap::const_iterator pos = skillMap_.find(code);
    if (pos != skillMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}

}} // namespace gideon { namespace servertest {
