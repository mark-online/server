#pragma once

#include <gideon/cs/shared/rpc/player/BuildingRpc.h>

namespace gideon { namespace zoneserver {

/**
 * @class BuildingMarkCallback
 *
 * 스킬 서비스
 */
class BuildingMarkCallback
{
public:
    virtual void addBuildingMarkInfo(ObjectId buildingId, const BuildingMarkInfo& buildingMarkInfo) = 0;
    virtual void removeBuildingMarkInfo(ObjectId buildingId) = 0;
};


}} // namespace gideon { namespace zoneserver {