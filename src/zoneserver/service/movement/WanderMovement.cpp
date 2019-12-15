#include "ZoneServerPCH.h"
#include "WanderMovement.h"
#include "../../ai/Brain.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/ability/Formable.h"
#include "../../controller/npc/NpcMoveController.h"
#include "../../service/formation/NpcFormation.h"
#include "../../world/region/MapRegion.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver {

template<>
void WanderMovement<go::Npc>::initialize()
{
    const datatable::NpcTemplate& npcTemplate = getOwner().getNpcTemplate();

    homePosition_ = getOwner().getHomePosition();

    walkDistance_ = npcTemplate.getMaxWanderDistance();
    assert(walkDistance_ > 0.0f);
    isWalkingToNextDestination_ = false;

    setNextMoveTime();
}


template<>
void WanderMovement<go::Npc>::finalize()
{
    getOwner().getMoveController().stop();
}


template<>
bool WanderMovement<go::Npc>::update(GameTime diff)
{
    nextMoveTime_.update(diff);
    if (nextMoveTime_.isPassed()) {
        setDestination();
        return false;
    }

    if (! checkArrivedToDestination()) {
        getOwner().getNpcMoveController().update();
    }
    return false;
}


template<>
void WanderMovement<go::Npc>::setNextMoveTime()
{
    const datatable::NpcTemplate& npcTemplate = getOwner().getNpcTemplate();
    const GameTime nextDelay =
        esut::random(int(npcTemplate.getInfo().ai_min_walk_delay()),
        int(npcTemplate.getInfo().ai_max_walk_delay()));
    nextMoveTime_.reset(nextDelay);
}


template<>
void WanderMovement<go::Npc>::setDestination()
{
    const Heading heading = esut::random() % maxHeading;
    const float range = esut::random(0.6f, 1.0f) * walkDistance_;
    const Vector2 direction = getDirection(heading);
    const float distanceX = range * direction.x;
    const float distanceY = range * direction.y;

    destination_.x_ = homePosition_.x_ + distanceX;
    destination_.y_ = homePosition_.y_ + distanceY;
    //destination_.x_ = homePosition_.x_ + (esut::random(-0.6f, 0.6f) * walkDistance_);
    //destination_.y_ = homePosition_.y_ + (esut::random(-0.6f, 0.6f) * walkDistance_);

    getOwner().getMapRegion().normalizeToRegion(destination_, getOwner().getRegionSequence());

    //SNE_LOG_DEBUG8("WanderMovement<go::Npc>::setDestination: %d, (%f,%f,%f) -> (%f,%f,%f)",
    //    currentPathIndex_, npc_.getPosition().x_, npc_.getPosition().y_, npc_.getPosition().z_,
    //    destination_.x_, destination_.y_, destination_.z_);

    setNextMoveTime();

    isWalkingToNextDestination_ = true;

    getOwner().getMoveController().moveTo(destination_);

    NpcFormationRefPtr formation = getOwner().queryFormable()->getFormation();
    if ((formation.get() != nullptr) && (formation->isLeader(getOwner()))) {
        formation->leaderMoved(destination_);
    }
}


template<>
bool WanderMovement<go::Npc>::checkArrivedToDestination()
{
    if (! isWalkingToNextDestination_) {
        return true;
    }

    if (! getOwner().getNpcMoveController().isArrivedAtDestination()) {
        return false;
    }

    getOwner().queryThinkable()->getBrain().wanderPaused();

    isWalkingToNextDestination_ = false;
    return true;
}

}} // namespace gideon { namespace zoneserver {
