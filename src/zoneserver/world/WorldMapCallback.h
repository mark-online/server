#pragma once

#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

/**
 * @class WorldMapCallback
 */
class WorldMapCallback
{
public:
    virtual ~WorldMapCallback() {}

    virtual void entitySpawned(ObjectId mapId, MapCode mapCode, go::Entity& entity) = 0;

    virtual void entityDespawned(go::Entity& entity, MapCode mapCode) = 0;
};

}} // namespace gideon { namespace zoneserver {
