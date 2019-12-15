#include "ZoneServerPCH.h"
#include "NpcMoveController.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/state/MoveState.h"
#include "../../model/state/CreatureState.h"
#include "../../model/state/CastState.h"
#include "../../ai/Brain.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver { namespace gc {

NpcMoveController::NpcMoveController(go::Entity* owner) :
    MoveController(owner)
{
}


void NpcMoveController::chaseTo(const go::Entity& target)
{
    // 값이 클수록 움직임이 둔하고, 스킬 거리를 벗어날 확률이 높다
    const float32_t moveTolerance = 0.2f;

    if (! target.isValid()) {
        return;
    }

    if (getOwner().queryCreatureState()->isSpawnProtection()) {
        return;
    }

    if (getOwner().queryMoveState()->isRooted()) {
        if (getOwner().queryCastState()->isCastingAndNotMovingCast()) {
            headTo(target);
        }
        stop();
        return;
    }

    const Position targetPosition = target.getPosition();
    const ObjectPosition currentPosition = getCurrentPosition();
    Vector2 directionToTarget = asVector2(targetPosition) - asVector2(currentPosition);
    float32_t distanceToMove = glm::length(directionToTarget);

    if (distanceToMove > moveTolerance) {
        normalize(directionToTarget, distanceToMove);
    }
    else {
        directionToTarget = getDirection(currentPosition.heading_);
    }

    go::Npc& owner = getOwnerAs<go::Npc>();
    const datatable::NpcTemplate& ownerNpcTemplate = owner.getNpcTemplate();

    float32_t realMaxApproachDistance = ownerNpcTemplate.getMaxApproachDistance();
    go::Moveable* moveable = const_cast<go::Entity&>(target).queryMoveable();
    const bool isTargetMoving = (moveable != nullptr) && moveable->isMoving();
    if (isTargetMoving) {
        realMaxApproachDistance *= 0.7f; // 상대가 이동 중이면 30%만큼 더 접근한다
    }

    if (distanceToMove > realMaxApproachDistance) {
        distanceToMove -= realMaxApproachDistance; // 선호하는 스킬을 시전할 수 있도록 이동
    }
    else {
        float32_t minApproachDistance = ownerNpcTemplate.getMinApproachDistance();
        if (target.isNpcOrMonster()) {
            const float32_t targetMinApproachDistance =
                static_cast<const go::Npc&>(target).getNpcTemplate().getMinApproachDistance();
            if (minApproachDistance > targetMinApproachDistance) {
                minApproachDistance = targetMinApproachDistance;
            }
        }

        if ((distanceToMove >= minApproachDistance) || isTargetMoving) {
            if (isMoving()) {
                stop(&currentPosition);
            }
            else {
                const Heading newHeading = getHeading(directionToTarget);
                if (newHeading != currentPosition.heading_) {
                    MoveController::turn(newHeading);
                }
            }
            return;
        }

        distanceToMove -= minApproachDistance; // 거리를 벌리기 위해 minApproachDistance_ 만큼 이동
    }

    if (fabs(distanceToMove) <= moveTolerance) {
        const Heading newHeading = getHeading(directionToTarget);
        if (newHeading != currentPosition.heading_) {
            MoveController::turn(newHeading);
        }
        return;
    }

    Position newDestination =
        advancePosition(currentPosition, directionToTarget, distanceToMove);
    Vector2 directionToDestin = asVector2(newDestination) - asVector2(currentPosition);
    const float32_t distanceSqToDestin = squaredLength(directionToDestin);

    if (isMoving()) {
        if (isAdjacent(newDestination, getDestination(), moveTolerance)) {
            return;
        }
    }
    else {
        if (isAdjacent(distanceSqToDestin, moveTolerance)) {
            if (distanceSqToDestin > moveTolerance) {
                const Heading newHeading = getHeading(directionToTarget);
                if (newHeading != currentPosition.heading_) {
                    MoveController::turn(newHeading);
                }
            }
            return;
        }
    }

    // NPC 뭉치기 방지 (임시)
    const float32_t maxShiftDistance = 5;
    const float32_t distanceToDestin = sqrtf(distanceSqToDestin);
    if ((distanceToDestin > 0) && (distanceToDestin <= maxShiftDistance) && (esut::random(0, 99) < 20)) {
        const Heading headingToTarget = getHeading(directionToTarget);
        const int halfShiftAngle = 80 * int(maxShiftDistance / distanceToDestin);
        const auto shiftAngle = static_cast<float32_t>(esut::random(-halfShiftAngle, halfShiftAngle));
        const float32_t newDistance = distanceToDestin / cosf(toRadians(shiftAngle));
        const Vector2 moveDirection = getDirection(fixHeading(headingToTarget + toHeading(shiftAngle)));
        if (! isZero(moveDirection)) {
            asVector2(newDestination) = asVector2(currentPosition) + (moveDirection * newDistance);
        }
    }

    goToward(newDestination, getHeading(directionToTarget));
}


void NpcMoveController::headTo(const go::Entity& target)
{
    if (isMoving()) {
        return;
    }

    const ObjectPosition currentPosition = getOwner().getPosition();
    const Heading newHeading = getHeading(target.getPosition(), currentPosition);
    if (newHeading != currentPosition.heading_) {
        MoveController::turn(newHeading);
    }
}

// = MoveController overriding

void NpcMoveController::initialize()
{
    MoveController::initialize();
}


void NpcMoveController::finalize()
{
    MoveController::finalize();
}

// = MovementCallback overriding

void NpcMoveController::entityGoneToward(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
    entityInfo, position;
}


void NpcMoveController::entityMoved(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
    entityInfo, position;
}


void NpcMoveController::entityStopped(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
    position;

    go::Knowable* knowable = getOwner().queryKnowable();
    assert(knowable != nullptr);
    go::Entity* entity = knowable->getEntity(entityInfo);
    if (! entity) {
        return;
    }

    getOwner().queryThinkable()->getBrain().movedInLineOfSight(*entity);
}


void NpcMoveController::entityTeleported(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
    entityInfo, position;
}


void NpcMoveController::entityTurned(const GameObjectInfo& entityInfo,
    Heading heading)
{
    entityInfo, heading;
}


void NpcMoveController::entityJumped(const GameObjectInfo& entityInfo,
    const ObjectPosition& position)
{
    entityInfo, position;
}


void NpcMoveController::entityRun(const GameObjectInfo& entityInfo)
{
    entityInfo;
}


void NpcMoveController::entityWalked(const GameObjectInfo& entityInfo)
{
    entityInfo;
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
