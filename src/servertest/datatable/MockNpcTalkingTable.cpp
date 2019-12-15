#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockNpcTalkingTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const datatable::NpcTalkingTemplate*
MockNpcTalkingTable::getNpcTalkingTemplate(NpcTalkingCode code) const
{
    code;
    //const NpcTalkingListMap::const_iterator pos = talkingListMap_.find(code);
    //if (pos != talkingListMap_.end()) {
    //    return (*pos).second;
    //}
    return nullptr;
}

}} // namespace gideon { namespace servertest {
