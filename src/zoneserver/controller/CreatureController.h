#pragma once

#include "EntityController.h"
#include "callback/InterestAreaCallback.h"
#include "callback/EffectCallback.h"
#include "callback/LifeStatusCallback.h"
#include "../model/gameobject/Creature.h"
#include <gideon/cs/shared/data/GraveStoneInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CreatureController
 * Controller for Creature
 */
class ZoneServer_Export CreatureController : public EntityController,
    protected InterestAreaCallback,
    protected LifeStatusCallback
{
private:
    // = EntityControllerAbility overriding
    virtual InterestAreaCallback* queryAppearanceCallback() { return this; }
    virtual EffectCallback* queryEffectCallback();
    virtual CreatureEffectCallback* queryCreatureEffectCallback();
    virtual MovementCallback* queryMovementCallback();
	virtual LifeStatusCallback* queryLifeStatusCallback() { return this; }

protected:
    // = LifeStatusCallback overriding
    /// 공격 받아 사망하였다
    virtual void died(go::Entity* from);

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
