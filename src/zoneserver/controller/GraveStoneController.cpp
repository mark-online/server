#include "ZoneServerPCH.h"
#include "GraveStoneController.h"
#include "../model/gameobject/GraveStone.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/EntityEvent.h"
#include "../service/skill/Skill.h"
#include "callback/StatsCallback.h"
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver { namespace gc {

void GraveStoneController::spawned(WorldMap& worldMap)
{
    StaticObjectController::spawned(worldMap);
}


void GraveStoneController::despawned(WorldMap& worldMap)
{
    StaticObjectController::despawned(worldMap);
}

}}} // namespace gideon { namespace zoneserver { namespace gc {

