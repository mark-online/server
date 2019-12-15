#pragma once

#include <gideon/cs/shared/data/CraftInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Craftable
 * 제작 있다
 */
class Craftable
{
public:
    virtual ~Craftable() {}
	
public:
    virtual bool hasCraftFunction(CraftType craftType) const = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
