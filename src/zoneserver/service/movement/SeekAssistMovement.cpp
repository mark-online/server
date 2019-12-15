#include "ZoneServerPCH.h"
#include "SeekAssistMovement.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/ability/Liveable.h"
#include "../../model/gameobject/status/CreatureStatus.h"
#include "../../controller/npc/NpcMoveController.h"
#include "../../model/state/MoveState.h"

namespace gideon { namespace zoneserver {

template<>
void SeekAssistMovement<go::Npc>::initialize()
{
    setDestination();
}


template<>
void SeekAssistMovement<go::Npc>::finalize()
{
    getOwner().getMoveController().stop();
}


template<>
bool SeekAssistMovement<go::Npc>::update(GameTime diff)
{
    MoveState* moveState = getOwner().queryMoveState();
    assert(moveState != nullptr);
    if (moveState->isRooted()) {
        return true;
    }

    nextCheckTime_.update(diff);
    if (nextCheckTime_.isPassed()) {
        setDestination();
        return false;
    }

    gc::NpcMoveController& npcMoveController = getOwner().getNpcMoveController();

    npcMoveController.update();
    return npcMoveController.isArrivedAtDestination();
}


template<>
void SeekAssistMovement<go::Npc>::setDestination()
{
    nextCheckTime_.reset(esut::random(800, 1500));

    getOwner().getNpcMoveController().chaseTo(assist_);
}

}} // namespace gideon { namespace zoneserver {
