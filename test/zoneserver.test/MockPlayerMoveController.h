#pragma once

#include "ZoneServer/controller/player/PlayerMoveController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerMoveController
 *
 * 테스트 용 mock MockPlayerMoveController
 */
class MockPlayerMoveController :
    public zoneserver::gc::PlayerMoveController,
    public sne::test::CallCounter
{
public:
    MockPlayerMoveController(zoneserver::go::Entity* owner);

    // = rpc::CharacterMovementRpc overriding
    OVERRIDE_SRPC_METHOD_2(evEntityGoneToward,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityMoved,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityStopped,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityTeleported,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityTurned,
        GameObjectInfo, entityInfo, Heading, heading);
    OVERRIDE_SRPC_METHOD_2(evEntityJumped,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(evRun,
        GameObjectInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_1(evWalked,
        GameObjectInfo, entityInfo);
    // = rpc::VehicleRpc overriding

    OVERRIDE_SRPC_METHOD_1(onDismountVehicle,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_3(evVehicleDismounted,
        GameObjectInfo, creatureInfo, float32_t, speed, bool, isForce);

    // = rpc::GliderRpc overriding
	OVERRIDE_SRPC_METHOD_2(onDismountGlider,
		ErrorCode, errorCode, uint32_t, currentDurability);
    OVERRIDE_SRPC_METHOD_2(evGliderDismounted,
        GameObjectInfo, creatureInfo, float32_t, speed);


public:
    ErrorCode lastErrorCode_;
};
