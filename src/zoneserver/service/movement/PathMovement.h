#pragma once

#include "Movement.h"
#include "../time/GameTimer.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class BasePathMovement
 */
template <typename T>
class BasePathMovement : public MovementMedium<T>
{
public:
    BasePathMovement(T& owner) :
        MovementMedium<T>(owner) {}

public:
    virtual void initialize();
    virtual void finalize();

    virtual bool update(GameTime diff);

private:
    void startToMove();
    void updateNextMoveTime();
    bool advancePath();

    ObjectPosition getCurrentPathNodePosition() const;
    bool isValidCurrentPathIndex() const;
    bool isLastPathNode() const;

private:
    GameTimeTracker nextMoveTime_;
    bool isWalkingToNextDestination_;
    ObjectPosition destination_;
    int currentPathIndex_;
};


/**
 * @class PathMovement
 */
template <typename T>
class PathMovement : public BasePathMovement<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<PathMovement<T> >
{
public:
    PathMovement(T& owner) :
        BasePathMovement<T>(owner) {}
};

/**
 * @class MarchMovement
 * 침략자 행군 이동
 */
template <typename T>
class MarchMovement : public BasePathMovement<T>,
    public sne::core::ThreadSafeMemoryPoolMixin<MarchMovement<T> >
{
public:
    MarchMovement(T& owner) :
        BasePathMovement<T>(owner) {}

public:
    virtual void initialize();
    virtual void finalize();
};

}} // namespace gideon { namespace zoneserver {