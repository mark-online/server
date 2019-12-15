#include "ZoneServerPCH.h"
#include "PlayerMoveController.h"
#include "../EntityController.h"
#include "../callback/LifeStatusCallback.h"
#include "../../model/gameobject/Creature.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/ability/Moveable.h"
#include "../../model/gameobject/ability/Castnotificationable.h"
#include "../../model/gameobject/ability/Castable.h"
#include "../../model/gameobject/ability/Vehicleable.h"
#include "../../model/gameobject/ability/Gliderable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/state/CreatureState.h"
#include "../../model/state/MoveState.h"
#include "../../model/state/GliderState.h"
#include "../../model/state/VehicleState.h"
#include "../../model/state/CastState.h"
#include "../../world/WorldMap.h"
#include "../../world/region/RegionTriggerManager.h"
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/server/utility/Profiler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

#define LOG_NPC_ONLY

namespace gideon { namespace zoneserver { namespace gc {

namespace {


/**
 * @class DismountVehicleEvent
 */
class DismountVehicleEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<DismountVehicleEvent>
{
public:
    DismountVehicleEvent(const GameObjectInfo& creatureInfo, float32_t speed, bool isForce) :
        creatureInfo_(creatureInfo),
		speed_(speed),
        isForce_(isForce) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isCreature()) {
            return;
        }
        VehicleCallback* vehicleCallback =
            entity.getController().queryVehicleMountCallback();
        if (vehicleCallback != nullptr) {
            vehicleCallback->vehicleDismounted(creatureInfo_, speed_, isForce_);
        }
    }

private:
    const GameObjectInfo creatureInfo_;
	float32_t speed_;
    bool isForce_;
};


/**
 * @class DismountGliderEvent
 */
class DismountGliderEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<DismountGliderEvent>
{
public:
    DismountGliderEvent(const GameObjectInfo& creatureInfo, float32_t speed) :
        creatureInfo_(creatureInfo),
		speed_(speed){}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isCreature()) {
            return;
        }
        GliderCallback* gliderCallback =
            entity.getController().queryGliderCallback();
        if (gliderCallback != nullptr) {
            gliderCallback->gliderDismounted(creatureInfo_, speed_);
        }
    }

private:
    const GameObjectInfo creatureInfo_;
	float32_t speed_;
};


} // namespace {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerMoveController);

PlayerMoveController::PlayerMoveController(go::Entity* owner) :
    MoveController(owner)
{
}


void PlayerMoveController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerMoveController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


// = MovementCallback overriding

void PlayerMoveController::entityGoneToward(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
#ifdef LOG_NPC_ONLY
    if (entityInfo.isNpcOrMonster()) {
        SNE_LOG_DEBUG("entityGoneToward(%g,%g,%g,%d)",
            position.x_, position.y_, position.z_, position.heading_);
    }
#endif

    evEntityGoneToward(entityInfo, position);
}


void PlayerMoveController::entityMoved(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
#ifdef LOG_NPC_ONLY
    if (entityInfo.isNpcOrMonster()) {
        SNE_LOG_DEBUG("entityMoved(%g,%g,%g,%d)",
            position.x_, position.y_, position.z_, position.heading_);
    }
#endif

    // TODO: 필드 상에 있는지 검사해야 한다
    evEntityMoved(entityInfo, position);
}


void PlayerMoveController::entityStopped(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
#ifdef LOG_NPC_ONLY
    if (entityInfo.isNpcOrMonster()) {
        SNE_LOG_DEBUG("entityStopped(%g,%g,%g,%d)",
            position.x_, position.y_, position.z_, position.heading_);
    }
#endif

    // TODO: 필드 상에 있는지 검사해야 한다
    evEntityStopped(entityInfo, position);
}


void PlayerMoveController::entityTeleported(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
#ifdef LOG_NPC_ONLY
    if (entityInfo.isNpcOrMonster()) {
        SNE_LOG_DEBUG("entityTeleported(%g,%g,%g,%d)",
            position.x_, position.y_, position.z_, position.heading_);
    }
#endif

    // TODO: 필드 상에 있는지 검사해야 한다
    evEntityTeleported(entityInfo, position);
}


void PlayerMoveController::entityTurned(const GameObjectInfo& entityInfo,
    Heading heading)
{
#ifdef LOG_NPC_ONLY
    if (entityInfo.isNpcOrMonster()) {
        SNE_LOG_DEBUG("entityTurned(%d)", heading);
    }
#endif

    // TODO: 필드 상에 있는지 검사해야 한다
    evEntityTurned(entityInfo, heading);
}


void PlayerMoveController::entityJumped(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    evEntityJumped(entityInfo, position);
}


void PlayerMoveController::entityRun(const GameObjectInfo& entityInfo)
{
    evRun(entityInfo);
}


void PlayerMoveController::entityWalked(const GameObjectInfo& entityInfo)
{
    evWalked(entityInfo);
}


// = VehicleCallback overriding
void PlayerMoveController::mountVehicle()
{
	go::Entity& owner = getOwner();
	EntityVehicleInfo info = owner.queryVehicleable()->getEntityVehicle();
	VehicleState* state = owner.queryVehicleState();
	if (state) {
		state->mountVehicle(info.vehicleCode_, info.harnessCode_);
	}

    go::Castable* castable = owner.queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

	go::Moveable* moveable = owner.queryMoveable();
	moveable->updateSpeed();

	go::CastNotificationable* notificationable = owner.queryCastNotificationable();
	if (notificationable) {
		CompleteCastResultInfo comleteInfo;
		comleteInfo.set(owner.getGameObjectInfo(), info.vehicleCode_, info.harnessCode_, moveable->getSpeed());
		notificationable->notifyCompleteCasting(comleteInfo);
	}
}


void PlayerMoveController::vehicleDismounted(const GameObjectInfo& creatureInfo, float32_t speed, bool isForce)
{
    evVehicleDismounted(creatureInfo, speed, isForce);
}

// = GliderCallback overriding

void PlayerMoveController::mountGlider()
{
	go::Entity& owner = getOwner();
	GliderCode code = owner.queryGliderable()->getGliderCode();

	GliderState* state = owner.queryGliderState();
	if (state) {
		state->mountGlider(code);
	}

    go::Castable* castable = owner.queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

	go::Moveable* moveable = owner.queryMoveable();
	moveable->updateSpeed();

	go::CastNotificationable* notificationable = owner.queryCastNotificationable();
	if (notificationable) {
		CompleteCastResultInfo comleteInfo;
		comleteInfo.set(owner.getGameObjectInfo(), code, moveable->getSpeed());
		notificationable->notifyCompleteCasting(comleteInfo);
	}
}


void PlayerMoveController::gliderDismounted(const GameObjectInfo& creatureInfo, float32_t speed)
{
    evGliderDismounted(creatureInfo, speed);
}


// = rpc::CharacterMovementRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerMoveController, goToward,
    ObjectPosition, destination)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! owner.queryMoveState()->canMove()) {
        return;
    }

    if (owner.queryCastState()->isCastingAndNotMovingCast()) {
        owner.queryCastable()->cancelCasting();
    }

    MoveController::goToward(destination);
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, move,
    ObjectPosition, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! owner.queryMoveState()->canMove()) {
        return;
    }

    if (owner.queryCastState()->isCastingAndNotMovingCast()) {
        owner.queryCastable()->cancelCasting();
    }

    MoveController::move(position);
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, stop,
    ObjectPosition, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (owner.queryCastState()->isCastingAndNotMovingCast()) {
        owner.queryCastable()->cancelCasting();
    }

	MoveController::stop(&position);
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, turn,
    Heading, heading)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! owner.queryMoveState()->canMove()) {
        return;
    }

    MoveController::turn(heading);
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, jump,
    ObjectPosition, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! owner.queryMoveState()->canMove()) {
        return;
    }

    if (owner.queryCastState()->isCastingAndNotMovingCast()) {
        owner.queryCastable()->cancelCasting();
    }

    MoveController::jump(position);
}


RECEIVE_SRPC_METHOD_0(PlayerMoveController, run)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (owner.queryCastState()->isCastingAndNotMovingCast()) {
        owner.queryCastable()->cancelCasting();
    }

    MoveController::run();
}


RECEIVE_SRPC_METHOD_0(PlayerMoveController, walk)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (owner.queryCastState()->isCastingAndNotMovingCast()) {
        owner.queryCastable()->cancelCasting();        
    }

    MoveController::walk();
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, correctionPosition,
    ObjectPosition, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    MoveController::correctionPosition(position);
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, regionEntered,
    RegionCode, regionCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const WorldMap* worldMap = owner.getCurrentWorldMap();
    if (worldMap != nullptr) {
        worldMap->getRegionTriggerManager().regionEntered(owner, regionCode);
    }
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, regionLeft,
    RegionCode, regionCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const WorldMap* worldMap = owner.getCurrentWorldMap();
    if (worldMap != nullptr) {
        worldMap->getRegionTriggerManager().regionLeft(owner, regionCode);
    }
}


RECEIVE_SRPC_METHOD_0(PlayerMoveController, fallen)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    getOwner().queryMoveState()->fallen();
}


RECEIVE_SRPC_METHOD_0(PlayerMoveController, landing)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
 
    HitPoint damage = owner.queryMoveState()->getFallDamage();
    if (hpMin != damage) {
        if (! owner.queryCreatureState()->isInvincible()) {
            static_cast<go::Creature&>(owner).getCreatureStatus().reduceHp(damage, true);
        }
        if (static_cast<go::Creature&>(owner).getCreatureStatus().isDied()) {
            CreatureState* creatureState = owner.queryCreatureState();
            if (creatureState && creatureState->isDied()) {
                return;
            }
            gc::LifeStatusCallback* lifeStatusCallback = 
                owner.getController().queryLifeStatusCallback();
            if (lifeStatusCallback) {
                lifeStatusCallback->died(&owner);
            }
        }
    }

    onLanding(damage);
}


RECEIVE_SRPC_METHOD_1(PlayerMoveController, environmentEntered,
    EnvironmentType, type)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    owner.queryMoveState()->environmentEntered(type);
}


RECEIVE_SRPC_METHOD_0(PlayerMoveController, environmentLeft)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    owner.queryMoveState()->environmentLeft();
}


FORWARD_SRPC_METHOD_1(PlayerMoveController, onLanding,
    HitPoint, damage)

FORWARD_SRPC_METHOD_2(PlayerMoveController, evEntityGoneToward,
    GameObjectInfo, entityInfo, ObjectPosition, position);


FORWARD_SRPC_METHOD_2(PlayerMoveController, evEntityMoved,
    GameObjectInfo, entityInfo, ObjectPosition, position);


FORWARD_SRPC_METHOD_2(PlayerMoveController, evEntityStopped,
    GameObjectInfo, entityInfo, ObjectPosition, position);


FORWARD_SRPC_METHOD_2(PlayerMoveController, evEntityTeleported,
    GameObjectInfo, entityInfo, ObjectPosition, position);


FORWARD_SRPC_METHOD_2(PlayerMoveController, evEntityTurned,
    GameObjectInfo, entityInfo, Heading, heading);


FORWARD_SRPC_METHOD_2(PlayerMoveController, evEntityJumped,
    GameObjectInfo, entityInfo, ObjectPosition, position);


FORWARD_SRPC_METHOD_1(PlayerMoveController, evRun,
    GameObjectInfo, entityInfo);


FORWARD_SRPC_METHOD_1(PlayerMoveController, evWalked,
    GameObjectInfo, entityInfo);


// = rpc::VehicleRpc overriding


RECEIVE_SRPC_METHOD_1(PlayerMoveController, dismountVehicle,
    bool, isForce)
{
    sne::server::Profiler profiler(__FUNCTION__);
    // todo 나중 처리
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! owner.queryVehicleState()->canDismountVehicle()) {
        onDismountVehicle(ecVehicleCannotDismountState);
        return;
    }
    owner.queryVehicleState()->dismountVehicle();
	go::Moveable* moveable = owner.queryMoveable();
	moveable->updateSpeed();

	auto event = std::make_shared<DismountVehicleEvent>(owner.getGameObjectInfo(),
        moveable->getSpeed(), isForce);
    owner.queryKnowable()->broadcast(event);
}


FORWARD_SRPC_METHOD_1(PlayerMoveController, onDismountVehicle,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_3(PlayerMoveController, evVehicleDismounted,
    GameObjectInfo, creatureInfo, float32_t, speed, bool, isForce);


// = rpc::GliderRpc overriding


RECEIVE_SRPC_METHOD_0(PlayerMoveController, dismountGlider)
{
    sne::server::Profiler profiler(__FUNCTION__);
    // todo 나중 처리
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! owner.queryGliderState()->isMonutGliding()) {
        onDismountGlider(ecGliderIsNotGliding, 0);
        return;
    }

    owner.queryGliderState()->dismountGlider();
	onDismountGlider(ecOk, owner.queryGliderable()->getGliderDurability());

	go::Moveable* moveable = owner.queryMoveable();
	moveable->updateSpeed();

    auto event = std::make_shared<DismountGliderEvent>(owner.getGameObjectInfo(),
        moveable->getSpeed());
    owner.queryKnowable()->broadcast(event);
}


FORWARD_SRPC_METHOD_2(PlayerMoveController, onDismountGlider,
    ErrorCode, errorCode, uint32_t, currentDurability);


FORWARD_SRPC_METHOD_2(PlayerMoveController, evGliderDismounted,
    GameObjectInfo, creatureInfo, float32_t, speed);


// = sne::srpc::RpcForwarder overriding

void PlayerMoveController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
#ifndef LOG_NPC_ONLY
    SNE_LOG_DEBUG("PlayerMoveController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
#else
    rpcId;
#endif

}

// = sne::srpc::RpcReceiver overriding

void PlayerMoveController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
#ifndef LOG_NPC_ONLY
    SNE_LOG_DEBUG("PlayerMoveController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
#else
    rpcId;
#endif
}

}}} // namespace gideon { namespace zoneserver { namespace gc {

