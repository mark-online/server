#include "ZoneServerPCH.h"
#include "ReturnMovement.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/state/MoveState.h"
#include "../../controller/MoveController.h"
#include <gideon/cs/datatable/template/entity_path_table.hxx>

namespace gideon { namespace zoneserver {

namespace {

inline Vector3 getNodePosition(const go::Entity& entity, int pathIndex)
{
    const gdt::path_node_t& pathNode = entity.getPath()->path_node()[pathIndex];
    return Vector3(pathNode.x(), pathNode.y(), pathNode.z());
}


Position getDestination(const go::Entity& entity)
{
    if (! entity.hasPath()) {
        return entity.getHomePosition();
    }

    const int currentPathIndex = entity.getCurrentPathIndex();
    int nextPathIndex = currentPathIndex;
    if (entity.isPathForward()) {
        if (currentPathIndex < entity.getLastPathIndex()) {
            nextPathIndex = currentPathIndex + 1;
        }
    }
    else {
        if (currentPathIndex > 0) {
            nextPathIndex = currentPathIndex - 1;
        }
    }

    const Vector3 currentNodePosition = getNodePosition(entity, currentPathIndex);
    if (nextPathIndex == currentPathIndex) {
        return asPosition(currentNodePosition);
    }

    const Vector3 nextNodePosition = getNodePosition(entity, nextPathIndex);
    return asPosition((nextNodePosition + currentNodePosition) / 2.0f);
}

} // namespace {

template<>
void ReturnMovement<go::Npc>::initialize()
{
    getOwner().getMoveController().moveTo(getDestination(getOwner()));
}


template<>
void ReturnMovement<go::Npc>::finalize()
{
    getOwner().getMoveController().stop();
}


template<>
bool ReturnMovement<go::Npc>::update(GameTime /*diff*/)
{
    MoveState* moveState = getOwner().queryMoveState();
    assert(moveState != nullptr);
    if (moveState->isRooted()) {
        return true;
    }

    gc::MoveController& moveController = getOwner().getMoveController();

    moveController.update();

    return moveController.isArrivedAtDestination();
}

}} // namespace gideon { namespace zoneserver {
