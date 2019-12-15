#include "ZoneServerPCH.h"
#include "EntityEvents.h"
#include "EntityStatusCallback.h"
#include "../EntityController.h"
#include "../../model/gameobject/Entity.h"

namespace gideon { namespace zoneserver {

void TargetChangedEvent::call(go::Entity& entity)
{
    gc::EntityStatusCallback* entityStatusCallback =
        entity.getController().queryEntityStatusCallback();
    if (entityStatusCallback != nullptr) {
        entityStatusCallback->entityTargetChanged(entityInfo_, targetStatusInfo_);
    }
}

}} // namespace gideon { namespace zoneserver {
