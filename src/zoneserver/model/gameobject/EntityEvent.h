#pragma once

#include <sne/base/concurrent/Runnable.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <memory>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class EntityEvent
 */
class EntityEvent
{
public:
    typedef std::shared_ptr<EntityEvent> Ref;

public:
    virtual ~EntityEvent() {}

    virtual void call(go::Entity& entity) = 0;
};


/**
 * @class EventCallTask
 */
class EventCallTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<EventCallTask>
{
public:
    EventCallTask(go::Entity& entity, go::EntityEvent::Ref event) :
        entity_(entity),
        event_(event) {}

private:
    virtual void run();

private:
    go::Entity& entity_;
    go::EntityEvent::Ref event_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
