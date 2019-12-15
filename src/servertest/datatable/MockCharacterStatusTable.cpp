#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockCharacterStatusTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const gdt::character_status_t* MockCharacterStatusTable::getCharacterStatus(CharacterClass cc, CreatureLevel cl, LevelStep ls) const
{
    cc, cl, ls;
    return nullptr;
}

}} //namespace gideon { namespace servertest {
