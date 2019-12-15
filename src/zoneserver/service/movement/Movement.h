#pragma once

#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver {

/**
 * @enum MovementType
 */
enum MovementType
{
    mvtUnknown = -1,

    mvtIdle,
    mvtWander,
    mvtPath,
    mvtReturn,
    mvtChase,
    mvtTimedFleeing,
    mvtSeekAssist,
    mvtPoint,
    mvtConfused,

    mvtBoundary
};

inline bool isValid(MovementType value)
{
    return (mvtUnknown < value) && (value < mvtBoundary);
}


/**
 * @class Movement
 */
class Movement
{
public:
    virtual ~Movement() {}

    virtual void initialize() = 0;
    virtual void finalize() = 0;

    /// @return 목적지에 도착?
    virtual bool update(GameTime diff) = 0;

    /// 해당 위치로 이동한다(PointMovement에서만 구현)
    virtual void moveTo(const Position& position) {
        position;
    }
};


/**
 * @class MovementMedium
 */
template<typename T>
class MovementMedium : public Movement
{
public:
    MovementMedium(T& owner) :
        owner_(owner) {}

protected:
    T& getOwner() {
        return owner_;
    }

    const T& getOwner() const {
        return owner_;
    }

private:
    T& owner_;
};

}} // namespace gideon { namespace zoneserver {