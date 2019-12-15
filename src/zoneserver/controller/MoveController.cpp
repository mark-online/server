#include "ZoneServerPCH.h"
#include "MoveController.h"
#include "EntityController.h"
#include "callback/MovementCallback.h"
#include "../world/WorldMap.h"
#include "../world/Sector.h"
#include "../model/gameobject/Creature.h"
#include "../model/gameobject/Npc.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/ability/Moveable.h"
#include "../model/gameobject/ability/ArenaMatchable.h"
#include "../service/arena/mode/Arena.h"
#include "../model/gameobject/ability/VehicleMountable.h"
#include "../model/gameobject/ability/GliderMountable.h"
#include "../model/gameobject/ability/Partyable.h"
#include "../model/state/MoveState.h"
#include "../model/state/CreatureState.h"
#include "../service/party/Party.h"
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {

namespace {

// TODO: config에서 읽어오도록 수정
const GameTime moveInterval = 400;


/**
 * @class GoneTowardEvent
 */
class GoneTowardEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<GoneTowardEvent>
{
public:
    GoneTowardEvent(const GameObjectInfo& entityInfo, const ObjectPosition& position) :
        entityInfo_(entityInfo),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityGoneToward(entityInfo_, position_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const ObjectPosition position_;
};


/**
 * @class MoveEvent
 */
class MoveEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<MoveEvent>
{
public:
    MoveEvent(const GameObjectInfo& entityInfo, const ObjectPosition& position) :
        entityInfo_(entityInfo),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityMoved(entityInfo_, position_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const ObjectPosition position_;
};


/**
 * @class StopEvent
 */
class StopEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<StopEvent>
{
public:
    StopEvent(const GameObjectInfo& entityInfo, const ObjectPosition& position) :
        entityInfo_(entityInfo),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityStopped(entityInfo_, position_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const ObjectPosition position_;
};


/**
 * @class TeleportEvent
 */
class TeleportEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<TeleportEvent>
{
public:
    TeleportEvent(const GameObjectInfo& entityInfo, const ObjectPosition& position) :
        entityInfo_(entityInfo),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityTeleported(entityInfo_, position_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const ObjectPosition position_;
};


/**
 * @class TurnEvent
 */
class TurnEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<TurnEvent>
{
public:
    TurnEvent(const GameObjectInfo& entityInfo, Heading heading) :
        entityInfo_(entityInfo),
        heading_(heading) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityTurned(entityInfo_, heading_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const Heading heading_;
};


/**
 * @class JumpEvent
 */
class JumpEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<JumpEvent>
{
public:
    JumpEvent(const GameObjectInfo& entityInfo, const ObjectPosition& position) :
        entityInfo_(entityInfo),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityJumped(entityInfo_, position_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const ObjectPosition position_;
};


/**
 * @class RunEvent
 */
class RunEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<RunEvent>
{
public:
    RunEvent(const GameObjectInfo& entityInfo) :
        entityInfo_(entityInfo) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityRun(entityInfo_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
};


/**
 * @class WalkEvent
 */
class WalkEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<WalkEvent>
{
public:
    WalkEvent(const GameObjectInfo& entityInfo) :
        entityInfo_(entityInfo) {}

private:
    virtual void call(go::Entity& entity) {
        MovementCallback* movementCallback =
            entity.getController().queryMovementCallback();
        if (movementCallback != nullptr) {
            movementCallback->entityWalked(entityInfo_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
};


inline SectorAddress moveInMap(WorldMap& worldMap, go::Entity& owner,
    const ObjectPosition& source, const ObjectPosition& destination)
{
    const Sector* destinSector = worldMap.migrate(owner, source, destination);
    if (! destinSector) {
        if (worldMap.getSector(owner.getSectorAddress()) != nullptr) {
            return owner.getSectorAddress();
        }
        else {
            return SectorAddress();
        }
    }

    return destinSector->getAddress();
}


} // namespace

// = MoveController

MoveController::MoveController(go::Entity* owner) :
	Controller(owner)
{
    reset();
}


bool MoveController::goToward(const ObjectPosition& destination)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());

    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        return false;
    }

    const SectorAddress sectorAddress = owner.getSectorAddress();
    const Sector* sector = worldMap->getSector(sectorAddress);
    if (! sector) {
        return true;
    }

    isMoving_ = true;
    const ObjectPosition newPosition = worldMap->adjustPosition(destination);
    static_cast<Position&>(destination_) = newPosition;
    destination_.heading_ = newPosition.heading_;
    goneToward(newPosition);
    return true;
}


bool MoveController::move(const ObjectPosition& position) 
{
	if (! moveEntity(position, false, false)) {
        return false;
    }

    isMoving_ = true;
    return true;
}


bool MoveController::turn(Heading heading)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());
	
    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        return false;
    }

    turned(fixHeading(heading));
    return true;
}


bool MoveController::jump(const ObjectPosition& position) 
{
	return moveEntity(position, false, true);
}


bool MoveController::run()
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	MoveState* moveState = owner.queryMoveState();
	if (! moveState) {
		return false;
	}
    if (moveState->isRunning()) {
        return false;
    }

    moveState->run();

    auto event = std::make_shared<RunEvent>(owner.getGameObjectInfo());
    owner.queryKnowable()->broadcast(event);
    return true;
}


bool MoveController::walk()
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	MoveState* moveState = owner.queryMoveState();
	if (! moveState) {
		return false;
	}

    if (! moveState->isRunning()) {
        return false;
    }

    moveState->walked();

    auto event = std::make_shared<WalkEvent>(owner.getGameObjectInfo());
    owner.queryKnowable()->broadcast(event);
    return true;
}


bool MoveController::correctionPosition(const ObjectPosition& position)
{    
    return moveEntity(position, true, false, true);
}


bool MoveController::stop(const ObjectPosition* position, bool shouldForce)
{
    isArrivedAtDestination_ = true;
    getOwner().queryMoveable()->setMoving(false);

    if ((! shouldForce) && (! isMoving_)) {
        return false;
    }

    const ObjectPosition stopPosition = (position != nullptr) ? *position : getCurrentPosition();
    if (! moveEntity(stopPosition, true, false, shouldForce)) {
        return false;
    }

    destination_ = stopPosition;
    isMoving_ = false;
    lastMoveTime_ = 0;
    return true;
}


void MoveController::update()
{
    if (! isMoving()) {
        return;
    }

    if (getOwner().queryMoveState()->isRooted()) {
        stop();
        return;
    }

    const GameTime elapsedTime = getElapsedMoveTime();
    if (elapsedTime < moveInterval) {
        return;
    }

    const ObjectPosition currentPosition = getCurrentPosition();
    const float32_t distanceToDestin = get2dDistanceTo(getDestination(), currentPosition);

    //SNE_LOG_DEBUG("update: (%.1f,%.1f)->(%.1f,%.1f), %.1f",
    //    getOwner().getPosition().x_, getOwner().getPosition().y_,
    //    currentPosition.x_, currentPosition.y_,
    //    distanceToDestin);

    if (distanceToDestin <= destinationTolerance_) {
        stop(&currentPosition);
    }
    else {
        //SNE_LOG_DEBUG("moveToNextPosition(%.1f, %.1f, %.1f)",
        //    elapsedTime, distanceToDestin, minApproachDistance_);
        moveToPosition(currentPosition);
    }
}


void MoveController::moveTo(const Position& newDestination, float32_t moveTolerance)
{
    if (getOwner().queryMoveState()->isRooted()) {
        return;
    }

    const ObjectPosition currentPosition = getCurrentPosition();

    if (isAdjacent(newDestination, currentPosition, moveTolerance)) {
        return;
    }

    destinationTolerance_ = moveTolerance;
    goToward(newDestination, getHeading(newDestination, currentPosition));
}


bool MoveController::moveEntity(const ObjectPosition& position,
    bool shouldStop, bool byJump, bool shouldForce)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());

    WorldMap* worldMap = owner.getCurrentWorldMap();

    if (! worldMap) {
        return false;
    }
	MoveState* moveState = owner.queryMoveState();
	if (! moveState) {
		return false;
	}

    const ObjectPosition source = owner.getPosition();
    const ObjectPosition destination = worldMap->adjustPosition(position);

    const SectorAddress sectorAddress =
        moveInMap(*worldMap, owner, source, destination);
    if (! sectorAddress.isValid()) {
        return false;
    }

    if (shouldStop) {
        stopped(sectorAddress, destination, shouldForce);
    }
    else {
        if (byJump) {
            jumped(sectorAddress, destination);
        }
        else {
            moved(sectorAddress, destination);
        }
    }

    moveState->moved();

    return true;
}


ObjectPosition MoveController::getCurrentPosition() const
{
    ObjectPosition position = getOwner().getPosition();

    if (! isMoving_) {
        return position;
    }

    const GameTime elapsedTime = GAME_TIMER->getElapsedTime(lastMoveTime_);

    const Vector2& destination = asVector2(destination_);
    const Vector2 dirDestination = destination - asVector2(position);
    const float32_t distanceToDestin = glm::length(dirDestination);
    const Vector2 direction = normalizeTo(dirDestination, distanceToDestin);

    const float32_t moveSpeed = getOwnerAs<go::Creature>().getSpeed() / 1000.0f; // m/ms
    const float32_t moveDistance = moveSpeed * elapsedTime;
    Vector2 delta = direction * moveDistance;
    if (squaredLength(delta) > std::pow(distanceToDestin, 2)) {
        delta = direction * distanceToDestin;
    }

    position.x_ += delta.x;
    position.y_ += delta.y;
    //position.z_ = position.z_ + delta.z;
    position.heading_ = destination_.heading_;
    return position;
}


void MoveController::goneToward(const ObjectPosition& destination)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());

    go::Moveable* moveable = owner.queryMoveable();
    if (! moveable) {
        return;
    }
	MoveState* moveState = owner.queryMoveState();
	if (! moveState) {
		return;
	}

    moveable->setDestination(destination);
    moveable->setMoving(true);
    moveState->moved();
    
    CreatureState* state = owner.queryCreatureState();
    if (state) {
        if (state->isHidden()) {
            return;
        }
    }

    go::Knowable* knowable = owner.queryKnowable();
    if (knowable != nullptr) {
        auto event = std::make_shared<GoneTowardEvent>(owner.getGameObjectInfo(),
            moveable->getDestination());
        knowable->broadcast(event);
    }
}


void MoveController::moved(const SectorAddress& sectorAddress,
    const ObjectPosition& position)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());

    owner.setSectorAddress(sectorAddress);
    owner.setPosition(position);

    bool moveNotify = false;
    go::ArenaMatchable* arenaMatchabe = owner.queryArenaMatchable();
    if (arenaMatchabe) {
        Arena* arena = arenaMatchabe->getArena();
        if (arena) {
            arena->notifyMoved(owner.getObjectId(), position.x_, position.y_);
            moveNotify = true;
        }
    }

    if (! moveNotify) {
        go::Partyable* partyable = owner.queryPartyable();
        if (partyable) {
            PartyPtr party = partyable->getParty();
            if (party.get() != nullptr) {
                party->notifyMove(owner.getObjectId(), position, false);
            }
        }
        go::WorldEventable* worldeventable = owner.queryWorldEventable();
        if (worldeventable) {
            if (worldeventable->shouldNotifyMovementable()) {
                worldeventable->notifyMovemoent(position, false);
            }
        }
    }

    CreatureState* state = owner.queryCreatureState();
    if (state) {
        if (state->isHidden()) {
            return;
        }
    }

    go::Knowable* knowable = owner.queryKnowable();
    if (knowable != nullptr) {
        auto event = std::make_shared<MoveEvent>(owner.getGameObjectInfo(), owner.getPosition());
        knowable->broadcast(event);
    }
}


void MoveController::stopped(const SectorAddress& sectorAddress,
    const ObjectPosition& position, bool shouldForce)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());

    owner.setSectorAddress(sectorAddress);
    owner.setPosition(position);

    go::Moveable* moveable = owner.queryMoveable();
    if (moveable != nullptr) {
        moveable->setMoving(false);
    }

    go::Partyable* partyable = owner.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->notifyMove(owner.getObjectId(), position, true);
        }
    }

    go::WorldEventable* worldeventable = owner.queryWorldEventable();
    if (worldeventable) {
        if (worldeventable->shouldNotifyMovementable()) {
            worldeventable->notifyMovemoent(position, true);
        }
    }

    CreatureState* state = owner.queryCreatureState();
    if ((state != nullptr) && (state->isHidden())) {
        return;
    }

    go::Knowable* knowable = owner.queryKnowable();
    if (knowable != nullptr) {
        if (shouldForce) {
            auto event = std::make_shared<TeleportEvent>(
                owner.getGameObjectInfo(), owner.getPosition());
            knowable->broadcast(event);
        }
        else {
            auto event = std::make_shared<StopEvent>(
                owner.getGameObjectInfo(), owner.getPosition());
            knowable->broadcast(event);
        }
    }
}


void MoveController::turned(Heading heading)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());

	MoveState* moveState = owner.queryMoveState();
	if (! moveState) {
		return;
	}

    owner.setHeading(heading);
    moveState->moved(true);

    CreatureState* state = owner.queryCreatureState();
    if (state) {
        if (state->isHidden()) {
            return;
        }
    }

    go::Knowable* knowable = owner.queryKnowable();
    if (knowable != nullptr) {
        auto event = std::make_shared<TurnEvent>(owner.getGameObjectInfo(), heading);
        knowable->broadcast(event);
    }
}


void MoveController::jumped(const SectorAddress& sectorAddress,
    const ObjectPosition& position)
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());

    owner.setSectorAddress(sectorAddress);
    owner.setPosition(position);

    CreatureState* state = owner.queryCreatureState();
    if (state) {
        if (state->isHidden()) {
            return;
        }
    }

    go::Knowable* knowable = owner.queryKnowable();
    if (knowable != nullptr) {
        auto event = std::make_shared<JumpEvent>(owner.getGameObjectInfo(), position);
        knowable->broadcast(event);
    }
}


void MoveController::vehicleMounted()
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());
    assert(owner.isCreature());
    if (! owner.isCreature()) {
        return;
    }

    go::VehicleMountable* vehicleMountable = owner.queryVehicleMountable();
    if (vehicleMountable != nullptr) {
        vehicleMountable->setRiding(true);
    }
}


void MoveController::vehicleDismounted()
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());
    assert(owner.isCreature());
    if (! owner.isCreature()) {
        return;
    }

    go::VehicleMountable* vehicleMountable = owner.queryVehicleMountable();
    if (vehicleMountable != nullptr) {
        vehicleMountable->setRiding(false);
    }
}


void MoveController::gliderMounted()
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());
    assert(owner.isCreature());
    if (! owner.isCreature()) {
        return;
    }

    go::GliderMountable* gliderMountable = owner.queryGliderMountable();
    if (gliderMountable != nullptr) {
        gliderMountable->setGliding(true);
    }
}


void MoveController::gliderDismounted()
{
    go::Entity& owner = getOwner();
    assert(owner.isValid());
    assert(owner.isCreature());
    if (! owner.isCreature()) {
        return;
    }

    go::GliderMountable* gliderMountable = owner.queryGliderMountable();
    if (gliderMountable != nullptr) {
        gliderMountable->setGliding(false);
    }
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
