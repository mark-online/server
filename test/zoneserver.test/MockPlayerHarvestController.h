#pragma once

#include "ZoneServer/controller/player/PlayerHarvestController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerHarvestController
 *
 * 테스트 용 mock MockPlayerHarvestController
 */
class MockPlayerHarvestController :
    public zoneserver::gc::PlayerHarvestController,
    public sne::test::CallCounter
{
public:
    MockPlayerHarvestController(zoneserver::go::Entity* owner);

public:
    OVERRIDE_SRPC_METHOD_2(onAcquireHarvest,
        ErrorCode, errorCode, DataCode, itemCode);
    OVERRIDE_SRPC_METHOD_3(evHarvestRewarded,
        ObjectId, haravestId, BaseItemInfos, itemInfos,
        QuestItemInfos, questItemInfos);

public:
    ErrorCode lastErrorCode_;
    BaseItemInfos lastHarvestItems_;
};
