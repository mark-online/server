#pragma once

#include "../zoneserver_export.h"
#include "Controller.h"
#include "callback/MovementCallback.h"
#include "../world/SectorAddress.h"
#include "../service/time/GameTimer.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver {
class MoveState;
}} // namespace gideon { namespace zoneserver 

namespace gideon { namespace zoneserver { namespace gc {

namespace {

inline bool isAdjacent(float32_t distanceSq, float32_t tolerance)
{
    const float32_t toleranceSq = square(tolerance);
    return distanceSq <= toleranceSq;
}


inline bool isAdjacent(const Position& p1, const Position& p2, float32_t tolerance)
{
    // TODO: z축 고려
    return isAdjacent(get2dDistanceSqTo(p1, p2), tolerance);
}


inline Position advancePosition(const Position& source, const Vector2& direction,
    float32_t distance)
{
    Position position(source);
    position.x_ += direction.x * distance;
    position.y_ += direction.y * distance;
    //position.z_ = ;
    return position;
}

} // namespace


/**
 * @class MoveController
 * 이동을 담당
 */
class ZoneServer_Export MoveController : public Controller,
	public MovementCallback
{
public:
    MoveController(go::Entity* owner);

    bool goToward(const ObjectPosition& destination);

    bool move(const ObjectPosition& position);

    bool turn(Heading heading);

    bool jump(const ObjectPosition& position);

    bool run();

    bool walk();

    bool correctionPosition(const ObjectPosition& position);

    bool stop(const ObjectPosition* position = nullptr, bool shouldForce = false);

    /// for controlled movement
    void update();

    /// for controlled movement
    void moveTo(const Position& newDestination, float32_t moveTolerance = 0.1f);

public:
    void vehicleMounted();
    void vehicleDismounted();

    void gliderMounted();
    void gliderDismounted();

public:
    virtual void initialize() {
        reset();
    }
    virtual void finalize() {}

public:
    /// for controlled movement
    bool isArrivedAtDestination() const {
        return (! isMoving_) || isArrivedAtDestination_;
    }

protected:
    void goToward(const Position& newDestination, Heading newHeading) {
        static_cast<Position&>(destination_) = newDestination;
        destination_.heading_ = newHeading;
        isArrivedAtDestination_ = false;

        moving();
        MoveController::goToward(destination_);
    }

    void moveToPosition(const ObjectPosition& position) {
        moving();
        MoveController::move(position);
    }

    void moving() {
        isMoving_ = true;
        lastMoveTime_ = GAME_TIMER->msec();
    }

protected:
    ObjectPosition getCurrentPosition() const;

    GameTime getElapsedMoveTime() const {
        return GAME_TIMER->getElapsedTime(lastMoveTime_);
    }

    ObjectPosition getDestination() const {
        return destination_;
    }

    bool isMoving() const {
        return isMoving_;
    }

private:
    void reset() {
        isMoving_ = false;
        lastMoveTime_ = 0;
        destination_.reset();
        destinationTolerance_ = 0.0f;
    }

private:
    void goneToward(const ObjectPosition& destination);
    void moved(const SectorAddress& sectorAddress, const ObjectPosition& position);
    void stopped(const SectorAddress& sectorAddress, const ObjectPosition& position,
        bool shouldForce);
    void turned(Heading heading);
    void jumped(const SectorAddress& sectorAddress, const ObjectPosition& position);

private:
    bool moveEntity(const ObjectPosition& position, bool shouldStop, bool byJump,
        bool shouldForce = false);

private:
    bool isMoving_;
    GameTime lastMoveTime_;
    ObjectPosition destination_;
    float32_t destinationTolerance_;
    bool isArrivedAtDestination_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
