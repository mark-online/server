#pragma once

#include <gideon/cs/shared/data/BuildingInfo.h>


namespace gideon { namespace zoneserver { namespace go {

/**
 * @class StaticObjectOwnerable
 */
class StaticObjectOwnerable
{
public:
    virtual ~StaticObjectOwnerable() {}

public:
    virtual const BuildingOwnerInfo& getBuildingOwnerInfo() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
