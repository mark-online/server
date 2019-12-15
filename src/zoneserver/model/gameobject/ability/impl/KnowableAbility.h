#pragma once

#include "../Knowable.h"
#include "../../Entity.h"

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class KnowableAbility
 * Entity를 알 수 있는 능력
 */
class KnowableAbility : public Knowable
{
    typedef std::mutex LockType;

public:
    KnowableAbility(Entity& owner) :
        owner_(owner) {}

private:
    // = Knowable overriding
    virtual void know(const EntityMap& entities);
    virtual void know(Entity& entity);

    virtual void forget(const EntityMap& entities);
    virtual void forget(Entity& entity);
    virtual void forget(const GameObjectInfo& info);
    virtual void forgetAll();

    virtual void broadcast(EntityEvent::Ref event, bool exceptSelf);

    virtual bool enumerate(EnumerateCommand& command);
    virtual bool enumeratePlayers(EnumerateCommand& command);

    virtual bool doesKnow(const Entity& entity) const {
        return doesKnow(entity.getGameObjectInfo());
    }

    virtual bool doesKnow(const GameObjectInfo& objectInfo) const;

    virtual EntityMap getKnownEntities() const;

    virtual GameObjects getKnownEntitiesInArea(const Position& center,
        float32_t radius, const QueryFilter* filter) const;

    virtual size_t getKnownPlayerCount() const;
    virtual bool isAnyPlayerIn2dDistance(float32_t distanceSq) const;

    virtual Entity* getEntity(const GameObjectInfo& id);
    virtual Entity* getNearestEntityBy(DataCode entityCode);
    virtual Entity* getNearestNpc();
    virtual Entity* getNearestMonster();

private:
    void know_i(Entity& entity);
    void forget_i(const GameObjectInfo& info);

private:
    go::Entity* getEntity_i(const GameObjectInfo& id) {
        const EntityMap::iterator pos = knownEntities_.find(id);
        if (pos != knownEntities_.end()) {
            return (*pos).second;
        }
        return nullptr;
    }

    bool isKnownEntity(const GameObjectInfo& entityInfo) const {
        return knownEntities_.find(entityInfo) != knownEntities_.end();
    }

    bool isKnownSkip(const GameObjectInfo& entityInfo) const;

private:
    Entity& owner_;

    EntityMap knownEntities_;
    EntityMap knownPlayers_;

    mutable LockType lockKnowable_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
