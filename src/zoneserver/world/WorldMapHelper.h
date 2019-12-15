#pragma once

#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace datatable {
class RegionTable;
class RegionCoordinates;
class RegionSpawnTable;
class PositionSpawnTable;
class EntityPathTable;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

/**
 * @class WorldMapHelper
 */
class WorldMapHelper
{
public:
    virtual ~WorldMapHelper() {}

    virtual ObjectId generateMapId() const = 0;

    virtual const datatable::RegionTable* getRegionTable(MapCode mapCode) const = 0;

    virtual const datatable::RegionCoordinates* getRegionCoordinates(MapCode mapCode) const = 0;

    virtual const datatable::RegionSpawnTable* getRegionSpawnTable(MapCode mapCode) const = 0;

    virtual const datatable::PositionSpawnTable* getPositionSpawnTable(MapCode mapCode) const = 0;

    virtual const datatable::EntityPathTable* getEntityPathTable(MapCode mapCode) const = 0;
};

}} // namespace gideon { namespace zoneserver {
