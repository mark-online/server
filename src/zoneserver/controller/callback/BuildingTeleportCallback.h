#pragma once

#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class BuildingTeleportCallback
 */
class BuildingTeleportCallback
{
public:
    virtual ~BuildingTeleportCallback() {}

public:
    virtual void teleportToOrderZone(ErrorCode errorCode, BuildingCode toBuildingCode,
		ObjectId fromBuildingId, const WorldPosition& worldPosition) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
