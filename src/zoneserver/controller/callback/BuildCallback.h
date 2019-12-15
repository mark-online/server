#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Time.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class BuildCallback
 */
class BuildCallback
{
public:
    virtual ~BuildCallback() {}

public:
    virtual void buildingStartBuilt(const GameObjectInfo& buildingInfo, sec_t startTime) = 0;
    virtual void buildingCompleted(const GameObjectInfo& buildingInfo) = 0;
    virtual void buildingToFoundationReverted(const GameObjectInfo& buildingInfo) = 0;
    virtual void buildingBroken(const GameObjectInfo& buildingInfo) = 0;
    virtual void buildingDestoryed(const GameObjectInfo& buildingInfo) = 0;
    virtual void buildingInvincibleStateChanged(const GameObjectInfo& buildingInfo, bool isActivate) = 0;
	virtual void buildingActivateStateChanged(const GameObjectInfo& buildingInfo, bool isActivate) = 0;
    virtual void buildingDoorToggled(const GameObjectInfo& buildingInfo, bool isOpen) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
