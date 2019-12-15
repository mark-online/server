#pragma once

#include "ZoneServer/controller/player/PlayerGraveStoneController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerGraveStoneController
 *
 * 테스트 용 mock MockPlayerGraveStoneController
 */
class MockPlayerGraveStoneController :
    public zoneserver::gc::PlayerGraveStoneController,
    public sne::test::CallCounter
{
public:
    MockPlayerGraveStoneController(zoneserver::go::Entity* owner);

public:
    ErrorCode lastErrorCode_;

    LootInvenItemInfoMap lastGraveStoneInsideinfo_;
};
