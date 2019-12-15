#pragma once

#include "ZoneServer/controller/player/PlayerNpcController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerNpcController
 *
 * 테스트 용 mock MockPlayerNpcController
 */
class MockPlayerNpcController :
    public zoneserver::gc::PlayerNpcController,
    public sne::test::CallCounter
{
public:
    MockPlayerNpcController(zoneserver::go::Entity* owner);

    // = rpc::NpcDialogRpc overriding
    OVERRIDE_SRPC_METHOD_2(evDialogOpened,
        GameObjectInfo, npc, GameObjectInfo, requester);
    OVERRIDE_SRPC_METHOD_2(evDialogClosed,
        GameObjectInfo, npc, GameObjectInfo, requester);

public:
    ErrorCode lastErrorCode_;
};
