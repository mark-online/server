#pragma once

#include "../../zoneserver_export.h"
#include "../MoveController.h"
#include "../callback/VehicleCallback.h"
#include "../callback/GliderCallback.h"
#include <gideon/cs/shared/rpc/player/EntityMovementRpc.h>
#include <gideon/cs/shared/rpc/player/VehicleRpc.h>
#include <gideon/cs/shared/rpc/player/GliderRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerMoveController 
 * 이동을 담당
 */
class ZoneServer_Export PlayerMoveController :
    public VehicleCallback,
    public GliderCallback,
    public MoveController,
    public rpc::EntityMovementRpc,
    public rpc::VehicleRpc,
    public rpc::GliderRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerMoveController);

public:
    enum { moveInterval = 500 };

public:
    PlayerMoveController(go::Entity* owner);

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

protected:
    // = MovementCallback overriding
    virtual void entityGoneToward(const GameObjectInfo& entityInfo,
        const ObjectPosition& position);
    virtual void entityMoved(const GameObjectInfo& entityInfo,
        const ObjectPosition& position);
    virtual void entityStopped(const GameObjectInfo& entityInfo,
        const ObjectPosition& position);
    virtual void entityTeleported(const GameObjectInfo& entityInfo,
        const ObjectPosition& position);
    virtual void entityTurned(const GameObjectInfo& entityInfo,
        Heading heading);
    virtual void entityJumped(const GameObjectInfo& entityInfo,
        const ObjectPosition& position);
    virtual void entityRun(const GameObjectInfo& entityInfo);
    virtual void entityWalked(const GameObjectInfo& entityInfo);

    // = VehicleCallback overriding
	virtual void mountVehicle();
    virtual void vehicleDismounted(const GameObjectInfo& creatureInfo, float32_t speed, bool isForce);

    // = GliderCallback overriding
	virtual void mountGlider();
    virtual void gliderDismounted(const GameObjectInfo& creatureInfo, float32_t speed);

public:
    // = rpc::CharacterMovementRpc overriding
    OVERRIDE_SRPC_METHOD_1(goToward,
        ObjectPosition, destination);
    OVERRIDE_SRPC_METHOD_1(move,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(stop,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(turn,
        Heading, heading);
    OVERRIDE_SRPC_METHOD_1(jump,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_0(run);
    OVERRIDE_SRPC_METHOD_0(walk);
    OVERRIDE_SRPC_METHOD_1(correctionPosition,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(regionEntered,
        RegionCode, regionCode);
    OVERRIDE_SRPC_METHOD_1(regionLeft,
        RegionCode, regionCode);
    OVERRIDE_SRPC_METHOD_0(fallen);
    OVERRIDE_SRPC_METHOD_0(landing);
    OVERRIDE_SRPC_METHOD_1(environmentEntered,
        EnvironmentType, type);
    OVERRIDE_SRPC_METHOD_0(environmentLeft);

    OVERRIDE_SRPC_METHOD_1(onLanding,
        HitPoint, damage);

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
    OVERRIDE_SRPC_METHOD_1(dismountVehicle,
        bool, isForce);

    OVERRIDE_SRPC_METHOD_1(onDismountVehicle,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_3(evVehicleDismounted,
        GameObjectInfo, creatureInfo, float32_t, speed, bool, isForce);


    // = rpc::GliderRpc overriding
    OVERRIDE_SRPC_METHOD_0(dismountGlider);

    OVERRIDE_SRPC_METHOD_2(onDismountGlider,
        ErrorCode, errorCode, uint32_t, currentDurability);

    OVERRIDE_SRPC_METHOD_2(evGliderDismounted,
        GameObjectInfo, creatureInfo, float32_t, speed);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
