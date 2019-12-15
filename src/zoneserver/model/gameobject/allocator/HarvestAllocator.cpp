#include "ZoneServerPCH.h"
#include "HarvestAllocator.h"
#include "../Harvest.h"
#include "../../../controller/HarvestController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* HarvestAllocator::malloc()
{
    return new go::Harvest(
        std::make_unique<gc::HarvestController>());
}


void HarvestAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::Harvest*>(entity));
}

}}} // namespace gideon { namespace zoneserver { namespace go {
