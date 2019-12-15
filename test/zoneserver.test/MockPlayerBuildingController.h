#pragma once

#include "ZoneServer/controller/player/PlayerBuildingController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerBuildingController
 *
 * 테스트 용 mock MockPlayerBuildingController
 */
class MockPlayerBuildingController :
    public zoneserver::gc::PlayerBuildingController,
    public sne::test::CallCounter
{
public:
    MockPlayerBuildingController(zoneserver::go::Entity* owner);

    OVERRIDE_SRPC_METHOD_1(onCreateBuilding,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_2(evBuildingStartBuilt,
        GameObjectInfo, buildingInfo, sec_t, startBuildTime);

    OVERRIDE_SRPC_METHOD_1(evBuildingBroken,
        GameObjectInfo, buildingInfo);

public:
    ErrorCode lastErrorCode_;
    ObjectId lastAnchorId_;
    ObjectId lastOwnerId_;
};
