#include "ZoneServerPCH.h"
#include "RegionTriggerManager.h"
#include "../../world/WorldMapHelper.h"
#include "../../model/gameobject/Player.h"
#include "../../model/gameobject/ability/Questable.h"
#include "../../model/quest/QuestRepository.h"
#include "../../controller/player/PlayerTeleportController.h"
#include "../../ZoneService.h"
#include "../../service/world_event/WorldEvent.h"
#include "../../service/world_event/WorldEventService.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include "../WorldMap.h"

namespace gideon { namespace zoneserver {

namespace {

bool isInRegion(const Position& position,
    const datatable::RegionCoordinates::CoordinatesList& coordinates)
{
    const float32_t delta = 5.0f;
    for (const datatable::CoordinatesTemplate& coord : coordinates) {
        if (coord.isContainedIn2d(position, delta)) {
            return true;
        }
    }
    return false;
}

} // namespace {

// = RegionTriggerManager

RegionTriggerManager::RegionTriggerManager(const datatable::RegionTable* regionTable,
    const datatable::RegionCoordinates* regionCoordinates) :
    regionTable_(regionTable),
    regionCoordinates_(regionCoordinates)
{
}


RegionTriggerManager::~RegionTriggerManager()
{
}


void RegionTriggerManager::regionEntered(go::Entity& player, RegionCode regionCode) const
{
    assert(player.isPlayer());

    if ((! regionTable_) || (! regionCoordinates_)) {
        assert(ZONE_SERVICE->isArenaServer());
        return;
    }

    const datatable::RegionTemplate* regionTemplate = regionTable_->getRegion(regionCode);
    if (! regionTemplate) {
        SNE_LOG_WARNING("RegionTriggerManager::regionEntered(%" PRIu64 ",%u) - region not found.",
            player.getObjectId(), regionCode);
        return;
    }

    const datatable::RegionCoordinates::CoordinatesList* coordinates =
        regionCoordinates_->getCoordinates(regionCode);
    if (! regionTemplate) {
        SNE_LOG_WARNING("RegionTriggerManager::regionEntered(%" PRIu64 ",%u) - coordinates not found.",
            player.getObjectId(), regionCode);
        return;
    }

    if (! isInRegion(player.getPosition(), *coordinates)) {
        SNE_LOG_WARNING("RegionTriggerManager::regionEntered(%" PRIu64 ",%u) - too far.",
            player.getObjectId(), regionCode);
        return;
    }

    if (regionTemplate->isQuestProbeRegion()) {
        player.queryQuestable()->getQuestRepository().completeProbeMission(
            regionTemplate->getQuestCode(), regionTemplate->getQuestMissionCode());
        // FYI: 응답 없음!
        //onCompleteProbeMission(errorCode, questCode, questMissionCode);
    }
    else if (regionTemplate->isPortalRegion()) {
        static_cast<go::Player&>(player).getPlayerTeleportController().teleportTo(
            regionTemplate->getTargetMapCode(), regionTemplate->getTargetRegionCode());
    }
    else if (regionTemplate->isWorldEventRegion()) {
        WorldEvent* worldEvent = WORLD_EVENT_SERVICE->getWorldEvent(regionTemplate->param1_);
        if (worldEvent) {
            worldEvent->enter(player, regionTemplate->param2_);
        }
    }
    else {
        // TODO:
    }
}


void RegionTriggerManager::regionLeft(go::Entity& player, RegionCode regionCode) const
{
    assert(player.isPlayer());

    if ((! regionTable_) || (! regionCoordinates_)) {
        assert(ZONE_SERVICE->isArenaServer());
        return;
    }

    const datatable::RegionTemplate* regionTemplate = regionTable_->getRegion(regionCode);
    if (! regionTemplate) {
        SNE_LOG_WARNING("RegionTriggerManager::regionLeft(%" PRIu64 ",%u) - region not found.",
            player.getObjectId(), regionCode);
        return;
    }

    if (regionTemplate->isWorldEventRegion()) {
        WorldEvent* worldEvent = WORLD_EVENT_SERVICE->getWorldEvent(regionTemplate->param1_);
        if (worldEvent) {
            worldEvent->leave(player.getObjectId(), regionTemplate->param2_);
        }
    }
}

}} // namespace gideon { namespace zoneserver {