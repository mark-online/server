#pragma once

#include "../../model/gameobject/allocator/EntityAllocator.h"
#include "../../model/gameobject/Entity.h"
#include <sne/base/memory/ObjectPool.h>
#include <boost/noncopyable.hpp>
#include <atomic>

namespace gideon { namespace zoneserver {

namespace {

using BaseEntityPool =
    sne::base::ObjectPool<go::Entity, go::EntityAllocator, std::mutex> ;

} // namespace {

/**
 * @class EntityPool
 */
class EntityPool : private BaseEntityPool
{
    static const bool growable = true;

public:
    EntityPool(go::EntityAllocator& entityAllocator, size_t poolSize) :
        BaseEntityPool(poolSize, entityAllocator, growable) {
        initialize();
    }

    void set(ObjectId objectId) {
        objectIdGenerator_ = objectId;
    }

public:
    template <typename EntityClass>
    EntityClass* acquireEntity() {
        return static_cast<EntityClass*>(acquire());
    }

    void releaseEntity(go::Entity* object) {
        release(object);
    }

public:
    ObjectId generateObjectId() {
        return ++objectIdGenerator_;
    }

private:
    std::atomic<ObjectId> objectIdGenerator_;
};

}} // namespace gideon { namespace zoneserver {
