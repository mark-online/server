#include "ZoneServerPCH.h"
#include "PointMovement.h"
#include "../../model/gameobject/Npc.h"
#include "../../controller/MoveController.h"

namespace gideon { namespace zoneserver {

template <>
void PointMovement<go::Npc>::initialize()
{
    getOwner().getMoveController().moveTo(destination_);
}


template <>
void PointMovement<go::Npc>::finalize()
{
    getOwner().getMoveController().stop();
}


template <>
bool PointMovement<go::Npc>::update(GameTime /*diff*/)
{
    gc::MoveController& moveController = getOwner().getMoveController();

    moveController.update();

    return moveController.isArrivedAtDestination();
}

}} // namespace gideon { namespace zoneserver {
