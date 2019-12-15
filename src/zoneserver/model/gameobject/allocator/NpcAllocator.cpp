#include "ZoneServerPCH.h"
#include "NpcAllocator.h"
#include "../Npc.h"
#include "../../../controller/NpcController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* NpcAllocator::malloc()
{
    return new go::Npc(
        std::make_unique<gc::NpcController>());
}


void NpcAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::Npc*>(entity));
}

}}} // namespace gideon { namespace zoneserver { namespace go {
