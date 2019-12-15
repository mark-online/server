#pragma once

#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Rate.h>
#include <gideon/cs/shared/data/Time.h>
#include <memory>

namespace gideon { namespace zoneserver {
class MovementManager;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace gc {
class MoveController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class Moveable
 * 이동할 수 있다
 */
class Moveable
{
public:
    virtual ~Moveable() {}

    virtual MovementManager& getMovementManager() = 0;

public:
    virtual void setDestination(const ObjectPosition& destin) = 0;
    virtual void setMoving(bool isMoving) = 0;

    virtual ObjectPosition getDestination() const = 0;

    virtual void setSpeedRate(permil_t totalSpeedRate) = 0;
    virtual void resetSpeedRate() = 0;
    virtual void updateSpeed() = 0;

    // 바로 처리 안한다.
    virtual void reserveReleaseUpSpeed() = 0;
    virtual void reserveReleaseDownSpeed() = 0;

public:
    virtual bool isMoving() const = 0;

    virtual float32_t getSpeed() const = 0;

    /// 현 위치에서 최대 이동 가능 거리
    virtual float32_t getMaxMoveDistance() const = 0;

public:
    virtual std::unique_ptr<gc::MoveController> createMoveController() = 0;

public:
    virtual gc::MoveController& getMoveController() = 0;
    virtual const gc::MoveController& getMoveController() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
