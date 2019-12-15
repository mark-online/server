#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/CheatInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CheatCallback
 */
class CheatCallback
{
public:
    virtual ~CheatCallback() {}

    virtual void cheatRewardExp(ExpPoint point) = 0;

    virtual void cheatZoneInUsers(const ZoneUserInfos& infos) = 0;

    virtual void cheatWorldInUsers(const WorldUserInfos& infos) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
