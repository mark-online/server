#pragma once

#include "ZoneServer/controller/player/PlayerTreasureController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerTreasureController
 *
 * 테스트 용 mock MockPlayerTreasureController
 */
class MockPlayerTreasureController :
    public zoneserver::gc::PlayerTreasureController,
    public sne::test::CallCounter
{
public:
    MockPlayerTreasureController(zoneserver::go::Entity* owner);

    OVERRIDE_SRPC_METHOD_2(onCloseTreasure,
        ErrorCode, errorCode, ObjectId, treasureId);
public:
    ErrorCode lastErrorCode_;
};
