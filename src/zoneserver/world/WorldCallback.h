#pragma once

#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class WorldCallback
 * World callback
 */
class WorldCallback
{
public:
    virtual ~WorldCallback() {}

    virtual void worldMapOpened(MapCode worldMapCode, ObjectId mapId) = 0;

    virtual void worldMapClosed(MapCode worldMapCode, ObjectId mapId) = 0;
};

}} // namespace gideon { namespace zoneserver {
