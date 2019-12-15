#include "ZoneServerPCH.h"
#include "ObeliskAllocator.h"
#include "../Obelisk.h"
#include "../../../controller/ObeliskController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* ObeliskAllocator::malloc()
{
    return new go::Obelisk(
        std::make_unique<gc::ObeliskController>());
}


void ObeliskAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::Obelisk*>(entity));
}

}}} // namespace gideon { namespace zoneserver { namespace go {
