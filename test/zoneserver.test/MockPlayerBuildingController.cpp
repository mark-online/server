#include "ZoneServerTestPCH.h"
#include "MockPlayerBuildingController.h"

using namespace gideon::zoneserver;

MockPlayerBuildingController::MockPlayerBuildingController(zoneserver::go::Entity* owner) :
    PlayerBuildingController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}




DEFINE_SRPC_METHOD_1(MockPlayerBuildingController, onCreateBuilding,
    ErrorCode, errorCode)
{
    addCallCount("onCreateBuilding");

    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_2(MockPlayerBuildingController, evBuildingStartBuilt,
    GameObjectInfo, buildingInfo, sec_t, startBuildTime)
{
    addCallCount("evBuildingStartBuilt");

    buildingInfo, startBuildTime;
}


DEFINE_SRPC_METHOD_1(MockPlayerBuildingController, evBuildingBroken,
    GameObjectInfo, buildingInfo)
{
    buildingInfo;
}