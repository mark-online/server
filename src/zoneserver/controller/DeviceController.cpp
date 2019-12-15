#include "ZoneServerPCH.h"
#include "DeviceController.h"
#include "../world/WorldMap.h"
#include "../service/spawn/SpawnService.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/Device.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/EntityEvent.h"
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver { namespace gc {

void DeviceController::spawned(WorldMap& worldMap)
{
    Parent::spawned(worldMap);
}


void DeviceController::despawned(WorldMap& worldMap)
{
    Parent::despawned(worldMap);
}


// = InterestAreaCallback overriding

void DeviceController::entitiesAppeared(const go::EntityMap& entities)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Device>().queryKnowable()->know(entities);
}


void DeviceController::entityAppeared(go::Entity& entity,
    const UnionEntityInfo& entityInfo)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    entityInfo;
    getOwnerAs<go::Device>().queryKnowable()->know(entity);
}


void DeviceController::entitiesDisappeared(const go::EntityMap& entities)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Device>().queryKnowable()->forget(entities);
}


void DeviceController::entityDisappeared(go::Entity& entity)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Device>().queryKnowable()->forget(entity);
}


void DeviceController::entityDisappeared(const GameObjectInfo& info)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Device>().queryKnowable()->forget(info);
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
