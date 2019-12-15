#pragma once

#include <gideon/cs/shared/data/Time.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Tickable
 * update tick ability
 */
class Tickable
{
public:
    virtual ~Tickable() {}
	
public:
    virtual void tick(GameTime diff) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
