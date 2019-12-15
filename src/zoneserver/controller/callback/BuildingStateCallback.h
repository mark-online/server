#pragma once

#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class BuildStateCallback
 */
class BuildingStateCallback
{
public:
    virtual ~BuildingStateCallback() {}

public:
    virtual void buildingCreated() = 0;
    virtual void buildingToFoundationReverted() = 0;
    virtual void buildingStartBuilt(sec_t startTime) = 0;
    virtual void buildCompleted() = 0;
    virtual void buildingBroken() = 0;
    virtual void buildDestroyed() = 0;
    virtual void buildOwnerUpdated() = 0;
    virtual void buildingInvincibleStateChanged(bool isActivate) = 0;
    virtual void buildCurrentStateSaved() = 0;

    virtual ErrorCode toggleDoor(go::Entity& player) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
