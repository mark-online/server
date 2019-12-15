#include "ZoneServerPCH.h"
#include "AnchorAllocator.h"
#include "../Anchor.h"
#include "../../../controller/AnchorController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* AnchorAllocator::malloc()
{
    return new go::Anchor(
        std::make_unique<gc::AnchorController>());
}


void AnchorAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::Anchor*>(entity));
}

}}} // namespace gideon { namespace zoneserver { namespace go {

