#pragma once

#include <gideon/cs/shared/data/UnionEntityInfo.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
class EntityMap;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class InterestAreaCallback
 */
class InterestAreaCallback
{
public:
    virtual ~InterestAreaCallback() {}

    virtual void entitiesAppeared(const go::EntityMap& entities) = 0;
    virtual void entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo) = 0;

    virtual void entitiesDisappeared(const go::EntityMap& entities) = 0;
    virtual void entityDisappeared(go::Entity& entity) = 0;
    virtual void entityDisappeared(const GameObjectInfo& info) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
