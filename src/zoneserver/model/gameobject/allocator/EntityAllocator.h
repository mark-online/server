#pragma once

#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class EntityAllocator
 */
class EntityAllocator : public boost::noncopyable
{
public:
    virtual ~EntityAllocator() {}

    virtual go::Entity* malloc() = 0;
    virtual void free(go::Entity* entity) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
