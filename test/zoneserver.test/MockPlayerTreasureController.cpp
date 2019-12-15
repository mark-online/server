#include "ZoneServerTestPCH.h"
#include "MockPlayerTreasureController.h"

using namespace gideon::zoneserver;

MockPlayerTreasureController::MockPlayerTreasureController(zoneserver::go::Entity* owner) :
    PlayerTreasureController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}


DEFINE_SRPC_METHOD_2(MockPlayerTreasureController, onCloseTreasure,
    ErrorCode, errorCode, ObjectId, treasureId)
{
    addCallCount("onCloseTreasure");

    lastErrorCode_ = errorCode;
    treasureId;
}
