#pragma once

#include "ZoneServer/controller/player/PlayerCastController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerCastController
 *
 * 테스트 용 mock MockPlayerCastController
 */
class MockPlayerCastController:
    public zoneserver::gc::PlayerCastController,
    public sne::test::CallCounter
{
public:
    MockPlayerCastController(zoneserver::go::Entity* owner);

    // = rpc::CastEvent overriding
    OVERRIDE_SRPC_METHOD_2(onStartCasting,
        ErrorCode, errorCode, StartCastInfo, startInfo);

    OVERRIDE_SRPC_METHOD_1(evCasted,
        StartCastResultInfo, startInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCanceled,
        CancelCastResultInfo, cancelInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCompleted,
        CompleteCastResultInfo, completeInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCompleteFailed,
        FailCompletedCastResultInfo, failInfo);

public:
    ErrorCode lastErrorCode_;
};
