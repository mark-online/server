#pragma once

#include "Movement.h"
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
class Creature;
} // namespace go {


/**
 * @class MovementManager
 */
class MovementManager : public boost::noncopyable
{
    enum MovementSlot {
        mvsUnknown = -1,
        mvsActive = 0, //< 스스로 이동
        mvsControlled = 1, //< 스킬 등에 의해 강제로 이동
        mvsCount
    };

    bool isValid(MovementSlot style) const {
        return (mvsUnknown < style) && (style < mvsCount);
    }

    typedef std::array<std::unique_ptr<Movement>, mvsCount> Movements;

    typedef std::mutex LockType;

public:
    MovementManager(go::Creature& owner);
    ~MovementManager();

    void initialize();
    void finalize();
    /// despawn할 때 호출해야 한다
    void reset();

    void setIdleMovement();
    void setWanderMovement();
    void setPathMovement();
    void setMarchMovement();
    void setReturnMovement();
    void setChaseMovement();
    void setTimedFleeingMovement(go::Entity* fright, GameTime duration = 0);
    void setSeekAssistMovement(go::Entity& assist);
    void setPointMovement(const Position& destination);
    void setConfusedMovement(GameTime duration);
    void unsetConfusedMovement();
    //void setKnockbackMovement(const Position& destination);

    /// @return 목적지에 도착?
    void updateMovement(GameTime diff);

public:
    void setTimedFleeParameter(msec_t duration) {
        fleeDuration_ = duration;
    }

public:
    /// mvsActive 이동을 완료했는가?
    bool isCompleted() const;

private:
    void setMovement(std::unique_ptr<Movement> movement, MovementSlot movementSlot = mvsActive);
    void setMovement_i(std::unique_ptr<Movement> movement, MovementSlot movementSlot);

private:
    Movement* getCurrentMovement() {
        assert(isValid(currentMovementSlot_));
        return movements_[currentMovementSlot_].get();
    }

    const Movement* getCurrentMovement() const {
        assert(isValid(currentMovementSlot_));
        return movements_[currentMovementSlot_].get();
    }

private:
    go::Creature& owner_;

    Movements movements_;
    MovementSlot currentMovementSlot_;
    bool isCompleted_;

    msec_t fleeDuration_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {
