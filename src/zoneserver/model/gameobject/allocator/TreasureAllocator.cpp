#include "ZoneServerPCH.h"
#include "TreasureAllocator.h"
#include "../Treasure.h"
#include "../../../controller/TreasureController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* TreasureAllocator::malloc()
{
    return new go::Treasure(
        std::make_unique<gc::TreasureController>());
}


void TreasureAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::Treasure*>(entity));
}

}}} // namespace gideon { namespace zoneserver { namespace go {
