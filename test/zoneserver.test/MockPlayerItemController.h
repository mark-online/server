#pragma once

#include "ZoneServer/controller/player/PlayerItemController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerItemController
 *
 * 테스트 용 mock MockPlayerItemController
 */
class MockPlayerItemController :
    public zoneserver::gc::PlayerItemController,
    public sne::test::CallCounter
{
public:
    MockPlayerItemController(zoneserver::go::Entity* owner);

public:
    ErrorCode lastErrorCode_;
};
