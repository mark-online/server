#include "ZoneServerTestPCH.h"
#include "MockPlayerHarvestController.h"

using namespace gideon::zoneserver;

MockPlayerHarvestController::MockPlayerHarvestController(zoneserver::go::Entity* owner) :
    PlayerHarvestController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}

 
DEFINE_SRPC_METHOD_2(MockPlayerHarvestController, onAcquireHarvest,
    ErrorCode, errorCode, DataCode, itemCode)
{
    addCallCount("onAcquireHarvest");
    lastErrorCode_ = errorCode;
    itemCode;
}


DEFINE_SRPC_METHOD_3(MockPlayerHarvestController, evHarvestRewarded,
    ObjectId, haravestId, BaseItemInfos, itemInfos,
    QuestItemInfos, questItemInfos)
{
    addCallCount("evHarvestRewarded");
    lastHarvestItems_ = itemInfos;
    haravestId, questItemInfos;
}