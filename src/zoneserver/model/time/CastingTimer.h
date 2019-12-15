#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Code.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/core/container/Containers.h>

namespace gideon { namespace zoneserver {


/***
 * @class CastingTimer
 ***/
class CastingTimer
{
    typedef sne::core::Map<DataCode, GameTime> CastingTimeInfoMap;
public:
    bool isCasting(DataCode dataCode);
    void startCasting(DataCode dataCode, GameTime castingTime);
    void endCasting(DataCode dataCode);
    void cancelCasting(DataCode dataCode);

private:
    GameTime getCastingTime(DataCode dataCode) const;

    void removeTime(DataCode dataCode);

private:
    CastingTimeInfoMap timeMap_;
};

}} // namespace gideon { namespace zoneserver {