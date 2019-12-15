#pragma once

#include <gideon/cs/shared/data/Time.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class CastGameTimeable
 * 시전에 관한 시간을 다룰수있다
 */
class CastGameTimeable
{
public:
    virtual ~CastGameTimeable() {}

    virtual void setCooldown(DataCode dataCode, GameTime coolTime,
        uint32_t index, GameTime globalCoolDownTime) = 0;
    virtual void cancelCooldown(DataCode dataCode, uint32_t index) = 0;
	virtual void cancelPreCooldown() = 0;

    virtual bool isGlobalCooldown(uint32_t index) const = 0;
    virtual bool isLocalCooldown(DataCode dataCode) const = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {