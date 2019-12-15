#pragma once

#include "DungeonController.h"
#include "callback/InterestAreaCallback.h"

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class RandomDungeonController
 * Controller for RandomDungeon
 */
class ZoneServer_Export RandomDungeonController : public DungeonController,
    protected InterestAreaCallback
{
    typedef DungeonController Parent;

private:
    // = EntityControllerAbility overriding
    virtual InterestAreaCallback* queryInterestAreaCallback() { return this; }

protected:
    // = InterestAreaCallback overriding
    virtual void entitiesAppeared(const go::EntityMap& entities);
    virtual void entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo);
    virtual void entitiesDisappeared(const go::EntityMap& entities);
    virtual void entityDisappeared(go::Entity& entity);
    virtual void entityDisappeared(const GameObjectInfo& info);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {