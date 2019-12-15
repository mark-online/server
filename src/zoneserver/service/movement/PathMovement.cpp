#include "ZoneServerPCH.h"
#include "PathMovement.h"
#include "../../ai/Brain.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/ability/Formable.h"
#include "../../model/gameobject/ability/Marchable.h"
#include "../../controller/npc/NpcMoveController.h"
#include "../formation/NpcFormation.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/EntityPathTable.h>

namespace gideon { namespace zoneserver {

// = BasePathMovement

template<>
void BasePathMovement<go::Npc>::initialize()
{
    assert(getOwner().hasPath());

    currentPathIndex_ = getOwner().getNearestPathNodeIndex();
    getOwner().setCurrentPathIndex(currentPathIndex_);
    startToMove();
}


template<>
void BasePathMovement<go::Npc>::finalize()
{
    getOwner().getMoveController().stop();
}


template<>
bool BasePathMovement<go::Npc>::update(GameTime diff)
{
    if (isWalkingToNextDestination_) {
        gc::NpcMoveController& npcMoveController = getOwner().getNpcMoveController();
        if (npcMoveController.isArrivedAtDestination()) {
            updateNextMoveTime();
            isWalkingToNextDestination_ = false;

            getOwner().queryThinkable()->getBrain().pathNodeArrived();
            return false;
        }
        npcMoveController.update();
    }
    else {
        nextMoveTime_.update(diff);
        if (! nextMoveTime_.isPassed()) {
            return false;
        }

        if (! advancePath()) {
            return true;
        }
        startToMove();
    }

    return false;
}


template<>
void BasePathMovement<go::Npc>::startToMove()
{
    // 노드와 노드 사이에 좀 더 부드럽게 이동하도록 하기 위함(너무 높으면 패쓰를 그만큼 벗어남)
    const float32_t moveTolerance = 1.0f;

    destination_ = getCurrentPathNodePosition();

    getOwner().getMoveController().moveTo(destination_, moveTolerance);

    isWalkingToNextDestination_ = true;

    getOwner().setHomePosition(destination_);

    NpcFormationRefPtr formation = getOwner().queryFormable()->getFormation();
    if ((formation.get() != nullptr) && (formation->isLeader(getOwner()))) {
        formation->leaderMoved(destination_);
    }
}


template<>
void BasePathMovement<go::Npc>::updateNextMoveTime()
{
    assert(isValidCurrentPathIndex());
    const gdt::path_node_t& pathNode = getOwner().getPath()->path_node()[currentPathIndex_];
    const GameTime nextDelay = pathNode.rest_time();
    if ((nextDelay > 0) || isLastPathNode()) {
        getOwner().getNpcMoveController().turn(Heading(pathNode.heading()));
    }

    nextMoveTime_.reset(nextDelay);
}


template<>
bool BasePathMovement<go::Npc>::advancePath()
{
    assert(isValidCurrentPathIndex());
    if (getOwner().isPathForward()) {
        if (! isLastPathNode()) {
            ++currentPathIndex_;
        }
        else if (getOwner().getPath()->is_looping()) {
            getOwner().setPathForward(false);
            --currentPathIndex_;
        }
        else {
            return false;
        }
    }
    else {
        if (currentPathIndex_ > 0) {
            --currentPathIndex_;
        }
        else if (getOwner().getPath()->is_looping()) {
            getOwner().setPathForward(true);
            ++currentPathIndex_;
        }
        else {
            return false;
        }
    }
    getOwner().setCurrentPathIndex(currentPathIndex_);
    return true;
}


template<>
ObjectPosition BasePathMovement<go::Npc>::getCurrentPathNodePosition() const
{
    const gdt::path_node_t& pathNode = getOwner().getPath()->path_node()[currentPathIndex_];
    const Position position(pathNode.x(), pathNode.y(), pathNode.z());
    return ObjectPosition(position, getHeading(position, getOwner().getPosition()));
}


template<>
bool BasePathMovement<go::Npc>::isValidCurrentPathIndex() const
{
    if (! getOwner().hasPath()) {
        return true;
    }
    return (currentPathIndex_ >= 0) &&
        (currentPathIndex_ < getOwner().getPath()->path_node().size());
}


template<>
bool BasePathMovement<go::Npc>::isLastPathNode() const
{
    return currentPathIndex_ >= (getOwner().getPath()->path_node().size() - 1);
}

// = MarchMovement

template<>
void MarchMovement<go::Npc>::initialize()
{
    BasePathMovement<go::Npc>::initialize();

    getOwner().queryMarchable()->marchStarted();
}


template<>
void MarchMovement<go::Npc>::finalize()
{
    BasePathMovement<go::Npc>::finalize();

    getOwner().queryMarchable()->marchStopped();
}

}} // namespace gideon { namespace zoneserver {
