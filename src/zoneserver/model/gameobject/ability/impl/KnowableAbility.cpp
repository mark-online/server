#include "ZoneServerPCH.h"
#include "KnowableAbility.h"
#include "../../Entity.h"
#include "../../EntityEvent.h"
#include "../../AbstractAnchor.h"
#include "gideon/3d/3d.h"
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver { namespace go {

namespace {

typedef sne::core::Vector<Entity*> Entities;

Entity* getNearestEntity(const Entities& entities, go::Entity& from)
{
    if (entities.empty()) {
        return nullptr;
    }
    if (entities.size() == 1) {
        return entities[0];
    }

    const Vector2& source = asVector2(from.getPosition());

    Entity* nearestEntity = nullptr;
    float32_t nearestDistanceSq = (std::numeric_limits<float32_t>::max)();
    for (Entity* entity : entities) {
        const Vector2 destin = asVector2(entity->getPosition());
        const float32_t distanceSq = squaredLength(destin - source);
        if (distanceSq < nearestDistanceSq) {
            nearestEntity = entity;
            nearestDistanceSq = distanceSq;
        }
    }

    return nearestEntity;
}

} // namespace {

// = KnowableAbility

void KnowableAbility::know(const EntityMap& entities)
{
    std::lock_guard<LockType> lock(lockKnowable_);


    for (const EntityMap::value_type& value : entities) {
        Entity* entity = value.second;
        know_i(*entity);
    }
}


void KnowableAbility::know(Entity& entity)
{
    std::lock_guard<LockType> lock(lockKnowable_);

    know_i(entity);
}


void KnowableAbility::forget(const EntityMap& entities)
{
    std::lock_guard<LockType> lock(lockKnowable_);

    for (const EntityMap::value_type& value : entities) {
        const GameObjectInfo& entityInfo = value.first;
        //Entity* entity = value.second;
        forget_i(entityInfo);
    }
}


void KnowableAbility::forget(Entity& entity)
{
    const GameObjectInfo& entityInfo = entity.getGameObjectInfo();

    std::lock_guard<LockType> lock(lockKnowable_);

    forget_i(entityInfo);
}


void KnowableAbility::forget(const GameObjectInfo& info)
{
    std::lock_guard<LockType> lock(lockKnowable_);

    forget_i(info);
}


void KnowableAbility::forgetAll()
{
    std::lock_guard<LockType> lock(lockKnowable_);

    knownEntities_.clear();
    knownPlayers_.clear();
}


void KnowableAbility::broadcast(EntityEvent::Ref event, bool exceptSelf)
{
    std::lock_guard<LockType> lock(lockKnowable_);

    for (const EntityMap::value_type& value : knownEntities_) {
        Entity* entity = value.second;
        if (! entity->isValid()) {
            continue;
        }

        if (exceptSelf) {
            if (entity->isSame(owner_)) {
                continue;
            }
        }

        (void)TASK_SCHEDULER->schedule(std::make_unique<EventCallTask>(*entity, event));
    }
}


bool KnowableAbility::enumerate(EnumerateCommand& command)
{
    EntityMap copiedKnownEntities;
    {
        std::lock_guard<LockType> lock(lockKnowable_);

        copiedKnownEntities = knownEntities_;
    }

    for (const EntityMap::value_type& value : copiedKnownEntities) {
        Entity* entity = value.second;
        if (! entity->isValid()) {
            continue;
        }

        if (command.execute(*entity)) {
            return true;
        }
    }

    return false;
}


bool KnowableAbility::enumeratePlayers(EnumerateCommand& command)
{
    EntityMap copiedKnownPlayers;
    {
        std::lock_guard<LockType> lock(lockKnowable_);

        copiedKnownPlayers = knownPlayers_;
    }

    for (const EntityMap::value_type& value : copiedKnownPlayers) {
        Entity* entity = value.second;
        if (! entity->isValid()) {
            continue;
        }

        if (command.execute(*entity)) {
            return true;
        }
    }

    return false;
}


bool KnowableAbility::doesKnow(const GameObjectInfo& entityInfo) const
{
    std::lock_guard<LockType> lock(lockKnowable_);

    return isKnownEntity(entityInfo);
}


EntityMap KnowableAbility::getKnownEntities() const
{
    std::lock_guard<LockType> lock(lockKnowable_);

    return knownEntities_;
}


// TODO: 최적화
GameObjects KnowableAbility::getKnownEntitiesInArea(const Position& center,
    float32_t radius, const QueryFilter* filter) const
{
    GameObjects entities;

    std::lock_guard<LockType> lock(lockKnowable_);

    entities.reserve(knownEntities_.size());

    const Vector2& posSource = asVector2(center);

    for (const EntityMap::value_type& value : knownEntities_) {
        const GameObjectInfo& goi = value.first;
        const Entity* entity = value.second;
        if (! entity->isValid()) {
            continue;;
        }

        // TODO: entity의 유효성 문제
        Vector2 dirTarget = (asVector2(entity->getPosition()) - posSource);
        const float32_t distanceSq = squaredLength(dirTarget);
        const float32_t newRadius = entity->getModelingRadiusSize() + radius;
        if (distanceSq > newRadius * newRadius) {
            continue;
        }
   

        if (filter != nullptr) {
            normalize(dirTarget, sqrtf(distanceSq));
            const Position dirTarget3d(dirTarget.x, dirTarget.y, 0.0f);
            if (! filter->isOk(*entity, dirTarget3d)) {
                continue;
            }
        }

        entities.push_back(goi);
    }

    return entities;
}


size_t KnowableAbility::getKnownPlayerCount() const
{
    std::lock_guard<LockType> lock(lockKnowable_);

    return knownPlayers_.size();
}


// TODO: 최적화
bool KnowableAbility::isAnyPlayerIn2dDistance(float32_t distanceSq) const
{
    std::lock_guard<LockType> lock(lockKnowable_);

    for (const EntityMap::value_type& value : knownPlayers_) {
        const Entity* entity = value.second;

        const float32_t lengthSq = owner_.getSquaredLength(entity->getPosition());
        if (lengthSq <= distanceSq) {
            return true;
        }
    }
    return false;
}


Entity* KnowableAbility::getEntity(const GameObjectInfo& id)
{
    std::lock_guard<LockType> lock(lockKnowable_);

    return getEntity_i(id);
}


Entity* KnowableAbility::getNearestEntityBy(DataCode entityCode)
{
    Entities entities;
    {
        std::lock_guard<LockType> lock(lockKnowable_);

        entities.reserve(knownEntities_.size());
        for (EntityMap::value_type& value : knownEntities_) {
            Entity* entity = value.second;
            if (entity->getEntityCode() == entityCode) {
                if (! entity->isSame(owner_)) {
                    entities.push_back(entity);
                }
            }
        }
    }

    return getNearestEntity(entities, owner_);
}


Entity* KnowableAbility::getNearestNpc()
{
    Entities entities;
    {
        std::lock_guard<LockType> lock(lockKnowable_);

        entities.reserve(knownEntities_.size());
        for (EntityMap::value_type& value : knownEntities_) {
            Entity* entity = value.second;
            if (entity->isNpc()) {
                if (! entity->isSame(owner_)) {
                    entities.push_back(entity);
                }
            }
        }
    }

    return getNearestEntity(entities, owner_);
}


Entity* KnowableAbility::getNearestMonster()
{
    Entities entities;
    {
        std::lock_guard<LockType> lock(lockKnowable_);

        entities.reserve(knownEntities_.size());
        for (EntityMap::value_type& value : knownEntities_) {
            Entity* entity = value.second;
            if (entity->isMonster()) {
                if (! entity->isSame(owner_)) {
                    entities.push_back(entity);
                }
            }
        }
    }

    return getNearestEntity(entities, owner_);
}


void KnowableAbility::know_i(Entity& entity)
{
    const GameObjectInfo& entityInfo = entity.getGameObjectInfo();
    if (isKnownSkip(entityInfo)) {
        return;
    }

    knownEntities_.erase(entityInfo);
    knownEntities_.insertEntity(entity);

    if (entityInfo.isPlayer()) {
        knownPlayers_.erase(entityInfo);
        knownPlayers_.insertEntity(entity);
    }
}


void KnowableAbility::forget_i(const GameObjectInfo& info)
{
    if (isKnownSkip(info)) {
        return;
    }

    knownEntities_.erase(info);

    if (info.isPlayer()) {
        knownPlayers_.erase(info);
    }
}


bool KnowableAbility::isKnownSkip(const GameObjectInfo& entityInfo) const
{
    if (! owner_.isPlayer()) {
        if (entityInfo.isHarvest() || entityInfo.isGraveStone() || entityInfo.isTreasure()) {
            return true;
        }
    }
    return false;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
