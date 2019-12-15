#include "ZoneServerPCH.h"
#include "FleeingMovement.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/state/MoveState.h"
#include "../../controller/npc/NpcMoveController.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <esut/Random.h>

namespace gideon { namespace zoneserver {

namespace {
const float32_t minQuietDistance = 5.0f;
const float32_t maxQuietDistance = 10.0f;
} // namespace {

// = FleeingMovement

template <>
void FleeingMovement<go::Npc>::initialize()
{
    if (! fright_) {
        fright_ = getOwner().queryTargetSelectable()->getSelectedTarget();
    }

    if (! fright_) {
        moveDirection_ = getDirection(getOwner().getPosition().heading_);
    }

    setDestination();
}


template <>
void FleeingMovement<go::Npc>::finalize()
{
    getOwner().getMoveController().stop();
}


template <>
bool FleeingMovement<go::Npc>::update(GameTime diff)
{
    MoveState* moveState = getOwner().queryMoveState();
    assert(moveState != nullptr);
    if (moveState->isRooted()) {
        return true;
    }

    gc::MoveController& moveController = getOwner().getMoveController();

    nextCheckTime_.update(diff);
    if (nextCheckTime_.isPassed() || moveController.isArrivedAtDestination()) {
        setDestination();
        return false;
    }

    moveController.update();
    return false;
}


template <typename T>
void FleeingMovement<T>::setDestination()
{
    nextCheckTime_.reset(esut::random(1000, 2000));

    const ObjectPosition ownerPosition = getOwner().getPosition();
    if (fright_ != nullptr) {
        // 대상의 반대 방향으로 +/-90도 범위 내에서 이동
        const float32_t degree = toDegrees(getYaw(asVector2(ownerPosition) - asVector2(fright_->getPosition())));
        moveDirection_ = getDirection(toHeading(degree + esut::random(-90, 90)));
        if (isZero(moveDirection_)) {
            moveDirection_ = getDirection(ownerPosition.heading_);
        }
    }

    const float32_t distance = getOwner().getSpeed() * nextCheckTime_.getExpiry() / 1000.0f;
    Position destination = ownerPosition;
    destination.x_ += moveDirection_.x * distance;
    destination.y_ += moveDirection_.y * distance;
    //destination.z_ = ;

    getOwner().getNpcMoveController().moveTo(destination);
}

// = TimedFleeingMovement

template <>
void TimedFleeingMovement<go::Npc>::initialize()
{
    FleeingMovement<go::Npc>::initialize();

    if (duration_ <= 0) {
        const datatable::NpcTemplate& npcTemplate = getOwner().getNpcTemplate();
        const float32_t maxDistance = npcTemplate.getMaxMoveDistance() * esut::random(0.7f, 1.0f);
        duration_ = GameTime(1000.0f * (maxDistance / getOwner().getSpeed()));
    }
    totalFleeTime_.reset(duration_);
}


//template <>
//void TimedFleeingMovement<go::Npc>::finalize()
//{
//    FleeingMovement<go::Npc>::finalize();
//}


template <>
bool TimedFleeingMovement<go::Npc>::update(GameTime diff)
{
    totalFleeTime_.update(diff);
    if (totalFleeTime_.isPassed()) {
        return true;
    }

    return FleeingMovement<go::Npc>::update(diff);
}

}} // namespace gideon { namespace zoneserver {
