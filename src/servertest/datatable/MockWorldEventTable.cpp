#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockWorldEventTable.h>

namespace gideon { namespace servertest {

const gdt::world_event_t* MockWorldEventTable::getWorldEventTemplate(WorldEventCode code) const
{
    code;
    return nullptr;
}

}} // namespace gideon { namespace servertest {
