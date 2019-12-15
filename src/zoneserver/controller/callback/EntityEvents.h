#pragma once

#include "../../model/gameobject/EntityEvent.h"
#include <gideon/cs/shared/data/EntityStatusInfo.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class TargetChangedEvent
 */
class TargetChangedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<TargetChangedEvent>
{
public:
    TargetChangedEvent(const GameObjectInfo& entityInfo, const EntityStatusInfo& targetStatusInfo) :
        entityInfo_(entityInfo),
        targetStatusInfo_(targetStatusInfo) {}

private:
    virtual void call(go::Entity& entity);

private:
    const GameObjectInfo entityInfo_;
    const EntityStatusInfo targetStatusInfo_;
};

}} // namespace gideon { namespace zoneserver {
