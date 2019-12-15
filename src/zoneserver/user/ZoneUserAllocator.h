#pragma once

#include "../model/gameobject/allocator/EntityAllocator.h"
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

class ZoneUser;

/**
* @class ZoneUserAllocator
*/
class ZoneUserAllocator : public boost::noncopyable
{
public:
    ZoneUserAllocator(go::EntityAllocator& playerAllocator) :
        playerAllocator_(playerAllocator) {}
    virtual ~ZoneUserAllocator() {}
    
    virtual ZoneUser* malloc() = 0;
    virtual void free(ZoneUser* user) = 0;

protected:
    go::EntityAllocator& getPlayerAllocator() {
        return playerAllocator_;
    }

private:
    go::EntityAllocator& playerAllocator_;
};

}} // namespace gideon { namespace zoneserver {
