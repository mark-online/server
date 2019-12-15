#include "ZoneServerPCH.h"
#include "ConfusedMovement.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/Player.h"
#include "../../model/state/MoveState.h"
#include "../../controller/MoveController.h"

namespace gideon { namespace zoneserver {

/**
 * @class WaypointGenerator
 */
class WaypointGenerator
{
    enum { maxWaypoints = 36 };

public:
    WaypointGenerator() {
        generate();
    }

    const Position& getWaypoint(int32_t& prevIndex) const {
        for (;;) {
            const int32_t moveIndex = esut::random(0, maxWaypoints - 1);
            if (moveIndex != prevIndex) {
                prevIndex = moveIndex;
                break;
            }
        }

        return waypoints_[prevIndex];
    }

private:
    void generate() {
        for (int idx = 0; idx < maxWaypoints; ++idx) {
            const float32_t wanderX = esut::random(2.0f, 10.f);
            const float32_t wanderY = esut::random(2.0f, 10.f);

            waypoints_[idx].x_ = (wanderX * (esut::random(0, 1) ? -1.0f : 1.0f));
            waypoints_[idx].y_ = (wanderY * (esut::random(0, 1) ? -1.0f : 1.0f));
            waypoints_[idx].z_ = 0.0f;
        }
    }

private:
    Position waypoints_[maxWaypoints];
};

WaypointGenerator s_waypointGenerator;

template <typename T>
void ConfusedMovement<T>::initialize()
{
    reset();

    totalMoveTime_.reset(duration_);
    originalPosition_ = getOwner().getPosition();
    nextMoveIndex_ = -1;

    setDestination();
}


template <typename T>
void ConfusedMovement<T>::finalize()
{
    getOwner().getMoveController().stop();
}


template <typename T>
bool ConfusedMovement<T>::update(GameTime diff)
{
    MoveState* moveState = getOwner().queryMoveState();
    assert(moveState != nullptr);
    if (moveState->isRooted()) {
        return true;
    }

    totalMoveTime_.update(diff);
    if (totalMoveTime_.isPassed()) {
        return true;
    }

    gc::MoveController& moveController = getOwner().getMoveController();
    if (moveController.isArrivedAtDestination()) {
        restingTime_.update(diff);
        if (restingTime_.isPassed()) {
            setDestination();
        }
    }
    else {
        moveController.update();
    }

    return false;
}


template <typename T>
void ConfusedMovement<T>::setDestination()
{
    const Position& waypoint = s_waypointGenerator.getWaypoint(nextMoveIndex_);
    Position newPosition = originalPosition_;
    newPosition.x_ += waypoint.x_;
    newPosition.y_ += waypoint.y_;
    newPosition.z_ += waypoint.z_; // TODO: z축 처리
    getOwner().getMoveController().moveTo(newPosition);

    restingTime_.reset(esut::random(200, 500));
}

// = Npc

template void ConfusedMovement<go::Npc>::initialize();
template void ConfusedMovement<go::Npc>::finalize();
template bool ConfusedMovement<go::Npc>::update(GameTime diff);
template void ConfusedMovement<go::Npc>::setDestination();

template <>
void ConfusedMovement<go::Npc>::reset()
{
    getOwner().queryTargetSelectable()->unselectTarget();
}

// = Player

template void ConfusedMovement<go::Player>::initialize();
template void ConfusedMovement<go::Player>::finalize();
template bool ConfusedMovement<go::Player>::update(GameTime diff);
template void ConfusedMovement<go::Player>::setDestination();

template <>
void ConfusedMovement<go::Player>::reset()
{
}

}} // namespace gideon { namespace zoneserver {
