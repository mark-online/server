#include "ZoneServerPCH.h"
#include "EntityController.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/ability/Thinkable.h"
#include "../ai/Brain.h"

namespace gideon { namespace zoneserver { namespace gc {

void EntityController::spawned(WorldMap& /*worldMap*/)
{
    go::Thinkable* thinkable = getOwner().queryThinkable();
    if (thinkable != nullptr) {
        thinkable->getBrain().spawned();
    }
}


void EntityController::despawned(WorldMap& /*worldMap*/)
{
    getOwner().setPathForward(true);

    go::Thinkable* thinkable = getOwner().queryThinkable();
    if (thinkable != nullptr) {
        thinkable->getBrain().despawned();
    }
}

}}} // namespace gideon { namespace zoneserver { namespace gc {