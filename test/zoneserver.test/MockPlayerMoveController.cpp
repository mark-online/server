#include "ZoneServerTestPCH.h"
#include "MockPlayerMoveController.h"

using namespace gideon::zoneserver;

MockPlayerMoveController::MockPlayerMoveController(zoneserver::go::Entity* owner) :
    PlayerMoveController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}



// = rpc::CharacterMovementRpc overriding

DEFINE_SRPC_METHOD_2(MockPlayerMoveController, evEntityGoneToward,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    addCallCount("evEntityGoneToward");

    entityInfo, position;
}


DEFINE_SRPC_METHOD_2(MockPlayerMoveController, evEntityMoved,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    addCallCount("evEntityMoved");

    entityInfo, position;
}


DEFINE_SRPC_METHOD_2(MockPlayerMoveController, evEntityStopped,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    addCallCount("evEntityStopped");

    entityInfo, position;
}


DEFINE_SRPC_METHOD_2(MockPlayerMoveController, evEntityTeleported,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    addCallCount("evEntityTeleported");

    entityInfo, position;
}


DEFINE_SRPC_METHOD_2(MockPlayerMoveController, evEntityTurned,
    GameObjectInfo, entityInfo, Heading, heading)
{
    addCallCount("evEntityTurned");

    entityInfo, heading;
}


DEFINE_SRPC_METHOD_2(MockPlayerMoveController, evEntityJumped,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    addCallCount("evEntityJumped");

    entityInfo, position;
}


DEFINE_SRPC_METHOD_1(MockPlayerMoveController, evRun,
    GameObjectInfo, entityInfo)
{
    addCallCount("evRun");

    entityInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerMoveController, evWalked,
    GameObjectInfo, entityInfo)
{
    addCallCount("evWalked");

    entityInfo;
}

// = rpc::VehicleRpc overriding

DEFINE_SRPC_METHOD_1(MockPlayerMoveController, onDismountVehicle,
    ErrorCode, errorCode)
{
    addCallCount("onDismountVehicle");

    lastErrorCode_ = errorCode;
}



DEFINE_SRPC_METHOD_3(MockPlayerMoveController, evVehicleDismounted,
    GameObjectInfo, creatureInfo, float32_t, speed, bool, isForce)
{
    addCallCount("evVehicleDismounted");

    creatureInfo, speed, isForce;
}


// = rpc::GliderRpc overriding
DEFINE_SRPC_METHOD_2(MockPlayerMoveController, onDismountGlider,
    ErrorCode, errorCode, uint32_t, currentDurability)
{
    addCallCount("onDismountGlider");
    lastErrorCode_ = errorCode;
	currentDurability;
}


DEFINE_SRPC_METHOD_2(MockPlayerMoveController, evGliderDismounted,
    GameObjectInfo, creatureInfo, float32_t, speed)
{
    addCallCount("evGliderDismounted");

    creatureInfo, speed;
}
