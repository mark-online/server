#include "ZoneServerPCH.h"
#include "AbstractAnchorController.h"
#include "callback/BuildCallback.h"
#include "callback/AnchorAbillityCallback.h"
#include "../world/WorldMap.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/AbstractAnchor.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/EntityEvent.h"
#include "../service/anchor/AnchorService.h"
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class AnchorTargetFireEvent
 */
class AnchorTargetFireEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<AnchorTargetFireEvent>
{
public:
    AnchorTargetFireEvent(const GameObjectInfo& anchorInfo, uint8_t index,
        const GameObjectInfo& targetInfo) :
        anchorInfo_(anchorInfo),
        index_(index),
		targetInfo_(targetInfo) {}

private:
    virtual void call(go::Entity& entity) {
		AnchorAbillityCallback* callback =
			entity.getController().queryAnchorAbillityCallback();
		if (callback) {
			callback->anchorTargetFired(anchorInfo_, index_, targetInfo_);
		}
    }

private:
    const GameObjectInfo anchorInfo_;
    const uint8_t index_;
	const GameObjectInfo targetInfo_;
};

/**
 * @class AnchorPositionFireEvent
 */
class AnchorPositionFireEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<AnchorPositionFireEvent>
{
public:
    AnchorPositionFireEvent(const GameObjectInfo& anchorInfo, uint8_t index,
        const Position& position) :
        anchorInfo_(anchorInfo),
        index_(index),
		position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        AnchorAbillityCallback* callback =
            entity.getController().queryAnchorAbillityCallback();
		if (callback) {
			callback->anchorPositionFired(anchorInfo_, index_, position_);
		}
    }

private:
    const GameObjectInfo anchorInfo_;
    const uint8_t index_;
	const Position position_;
};

} // namespace


// = EntityController overriding

void AbstractAnchorController::spawned(WorldMap& worldMap)
{
    StaticObjectController::spawned(worldMap);
}


void AbstractAnchorController::despawned(WorldMap& /*worldMap*/)
{
    //StaticObjectController::despawned(worldMap);
}


// = InterestAreaCallback overriding

void AbstractAnchorController::entitiesAppeared(const go::EntityMap& entities)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::AbstractAnchor>().queryKnowable()->know(entities);
}


void AbstractAnchorController::entityAppeared(go::Entity& entity,
    const UnionEntityInfo& entityInfo)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    entityInfo;
    getOwnerAs<go::AbstractAnchor>().queryKnowable()->know(entity);
}


void AbstractAnchorController::entitiesDisappeared(const go::EntityMap& entities)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::AbstractAnchor>().queryKnowable()->forget(entities);
}


void AbstractAnchorController::entityDisappeared(go::Entity& entity)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::AbstractAnchor>().queryKnowable()->forget(entity);
}


void AbstractAnchorController::entityDisappeared(const GameObjectInfo& info)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::AbstractAnchor>().queryKnowable()->forget(info);
}


}}} // namespace gideon { namespace zoneserver { namespace gc {

