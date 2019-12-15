#include "ZoneServerPCH.h"
#include "NormalMapRegion.h"

namespace gideon { namespace zoneserver {

void NormalMapRegion::entitySpawned(go::Entity& /*entity*/)
{
    assert(false && "DON'T CALL ME!");
}


void NormalMapRegion::entityDespawned(go::Entity& /*entity*/)
{
    assert(false && "DON'T CALL ME!");
}


go::Entity* NormalMapRegion::getFirstEntity(ObjectType /*objectType*/, DataCode /*dataCode*/)
{
    assert(false && "DON'T CALL ME!");
    return nullptr;
}


size_t NormalMapRegion::getEntityCount(ObjectType /*objectType*/) const
{
    assert(false && "DON'T CALL ME!");
    return 0;
}

}} // namespace gideon { namespace zoneserver {
