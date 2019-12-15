#include "ZoneServerPCH.h"
#include "WorldEventService.h"
#include "WorldEvent.h"
#include "../../world/WorldMap.h"
#include "../time/GameTimer.h"
#include <gideon/cs/datatable/WorldEventTable.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver {

SNE_DEFINE_SINGLETON(WorldEventService);

WorldEventService::WorldEventService()
{
}


WorldEventService::~WorldEventService() 
{
    stopEventTask();
}


bool WorldEventService::initialize(WorldMap& worldMap, WorldMapHelper& worldMapHelper)
{
    MapCode worldMapCode = worldMap.getMapCode();
    const datatable::WorldEventTable::WorldEventMap& worldEventMap = WORLD_EVENT_TABLE->getWorldEventMap();
    for (const auto& value : worldEventMap) {
        const gdt::world_event_t* eventTemplate = value.second;
        if (eventTemplate->world_map_code() == worldMapCode) {
            auto worldEvent = std::make_unique<WorldEvent>(worldMap, worldMapHelper, *eventTemplate);
            if (! worldEvent->initialize()) {
                return false;
            }
            worldEventMap_.emplace(eventTemplate->world_event_code(), std::move(worldEvent));
        }
    }

    startEventTask();
    WorldEventService::instance(this);
    return true;
}


void WorldEventService::login(go::Entity& player)
{
    for (auto& value : worldEventMap_) {
        WorldEvent& worldEvent = *value.second;
        worldEvent.login(player);
    }
}


void WorldEventService::logout(ObjectId playerId)
{
    for (auto& value : worldEventMap_) {
        WorldEvent& worldEvent = *value.second;
        worldEvent.logout(playerId);
    }
}


void WorldEventService::fillWorldEventInfos(WorldEventInfos& worldEventInfos)
{
    for (auto& value : worldEventMap_) {
        WorldEvent& worldEvent = *value.second;
        worldEvent.fillWorldEventInfo(worldEventInfos);
    }
}


WorldEvent* WorldEventService::getWorldEvent(WorldEventCode worldEventCode)
{
    WorldEventMap::iterator pos = worldEventMap_.find(worldEventCode);
    if (pos != worldEventMap_.end()) {
        return ((*pos).second).get();
    }
    return nullptr;
}


void WorldEventService::startEventTask()
{
    lastUpdateTime_ = GAME_TIMER->msec();

    const msec_t eventInterval = 1000; //1 * 1000;
    eventTask_ = TASK_SCHEDULER->schedule(*this, 0, eventInterval);
}


void WorldEventService::stopEventTask()
{
    sne::base::Future::Ref eventTask = eventTask_.lock();
    if (eventTask.get() != nullptr) {
        eventTask->cancel();
        eventTask_.reset();
    }
}


// TODO: 코드가 이해가 안됨 T.T
void WorldEventService::run()
{    
    bool isUniqueEventPlaying = false;

    for (auto& value : worldEventMap_) {
        WorldEvent& worldEvent = *value.second;
        if (worldEvent.isPlaying() && worldEvent.isUniqueEvent()) {
            isUniqueEventPlaying = true;
            uniquePlayEvent_ = &worldEvent;
            break;
        }
    }

    for (auto& value : worldEventMap_) {
        WorldEvent& worldEvent = *value.second;
        if (!worldEvent.isUniqueEvent() && worldEvent.isWaiting() && isUniqueEventPlaying) {
            continue;
        }
        if (!uniquePlayEvent_ || (uniquePlayEvent_ == &worldEvent)) {
            worldEvent.update();
            if (worldEvent.isPlaying() && worldEvent.isUniqueEvent()) {
                isUniqueEventPlaying = true;
                uniquePlayEvent_ = &worldEvent;
                break;
            }
        }
    }

    if (uniquePlayEvent_ && (!isUniqueEventPlaying)) {
        for (auto& value : worldEventMap_) {
            WorldEvent& worldEvent = *value.second;
            if (worldEvent.isUniqueEvent() && &worldEvent != uniquePlayEvent_) {
                worldEvent.setInitWaitState();
            }
        }
        uniquePlayEvent_ = nullptr;
    }
}

}} // namespace gideon { namespace zoneserver {
