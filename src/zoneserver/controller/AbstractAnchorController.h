#pragma once

#include "StaticObjectController.h"
#include "callback/BuildingStateCallback.h"
#include "callback/InterestAreaCallback.h"


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class AbstractAnchorController
 * Controller for AbstractAnchor
 */
class ZoneServer_Export AbstractAnchorController : public StaticObjectController,
    public InterestAreaCallback
{
protected:
    // = EntityControllerAbility overriding
    virtual InterestAreaCallback* queryAppearanceCallback() { return this;}

protected:
	// = EntityController overriding
	virtual void spawned(WorldMap& worldMap);
	virtual void despawned(WorldMap& worldMap);

protected:
    // = InterestAreaCallback overriding
    virtual void entitiesAppeared(const go::EntityMap& entities);
    virtual void entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo);
    virtual void entitiesDisappeared(const go::EntityMap& entities);
    virtual void entityDisappeared(go::Entity& entity);
    virtual void entityDisappeared(const GameObjectInfo& info);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {