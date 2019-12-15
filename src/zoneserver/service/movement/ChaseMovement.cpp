#include "ZoneServerPCH.h"
#include "ChaseMovement.h"
#include "../../model/gameobject/Npc.h"
#include "../../controller/npc/NpcMoveController.h"

namespace gideon { namespace zoneserver {

template<>
void ChaseMovement<go::Npc>::initialize()
{
    setDestination();
}


template<>
void ChaseMovement<go::Npc>::finalize()
{
    getOwner().getMoveController().stop();
}


template<>
bool ChaseMovement<go::Npc>::update(GameTime diff)
{
    nextCheckTime_.update(diff);
    if (nextCheckTime_.isPassed()) {
        setDestination();
        return false;
    }

    getOwner().getNpcMoveController().update();
    return false;
}


template<>
void ChaseMovement<go::Npc>::setDestination()
{
    //nextCheckTime_.reset(esut::random(400, 800));
    nextCheckTime_.reset(600);

    go::Entity* target = getOwner().queryTargetSelectable()->getSelectedTarget();
    if (! target) {
        return;
    }

    getOwner().getNpcMoveController().chaseTo(*target);
}

}} // namespace gideon { namespace zoneserver {
