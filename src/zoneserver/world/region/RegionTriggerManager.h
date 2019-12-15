#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/RegionInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace datatable {
class RegionTable;
class RegionCoordinates;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

/**
 * @class RegionTriggerManager
 */
class ZoneServer_Export RegionTriggerManager : public boost::noncopyable
{    
public:
    RegionTriggerManager(const datatable::RegionTable* regionTable,
        const datatable::RegionCoordinates* regionCoordinates);
    ~RegionTriggerManager();

    void regionEntered(go::Entity& player, RegionCode regionCode) const;
    void regionLeft(go::Entity& player, RegionCode regionCode) const;

private:
    const datatable::RegionTable* regionTable_;
    const datatable::RegionCoordinates* regionCoordinates_;
};

}} // namespace gideon { namespace zoneserver {
