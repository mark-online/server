#include "ZoneServerPCH.h"
#include "WorldMapSpawnMapRegion.h"
#include "../../service/anchor/AnchorService.h"

namespace gideon { namespace zoneserver {


bool WorldMapSpawnMapRegion::spawnBuildings()
{
    return isSucceeded(ANCHOR_SERVICE->spawnBuildings(getWorldMap(), *this, getPositionSpawnTemplates()));
}


}} // namespace gideon { namespace zoneserver {

