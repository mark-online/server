#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/RegionInfo.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {



/***
 * @class WorldEventMissionCallback
 ***/
class WorldEventPlayCallback
{
public:
    WorldEventPlayCallback() {}
    virtual ~WorldEventPlayCallback() {}

    virtual void enter(go::Entity& player, RegionCode regionCode) = 0;
    // 리전을 벗어날때
    virtual void leave(ObjectId playerId, RegionCode regionCode) = 0;
};

}} // namespace gideon { namespace zoneserver {
