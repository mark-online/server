#include "ZoneServerPCH.h"
#include "WorldEventPlayState.h"
#include "../WorldEventPhase.h"
#include "../../../world/World.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/WorldEventCallback.h"
#include <gideon/cs/shared/data/WorldEventInfo.h>

namespace gideon { namespace zoneserver {


namespace {


/**
 * @class OpenWorldEventEvent
 */
class OpenWorldEventEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<OpenWorldEventEvent>
{
public:
    OpenWorldEventEvent(WorldEventCode worldEventCode) :
        worldEventCode_(worldEventCode) {}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* callback = entity.getController().queryWorldEventCallback();
        if (callback) {
            callback->worldEventOpend(worldEventCode_);
        }
    }

private:
    const WorldEventCode worldEventCode_;
};

/**
 * @class WorldEventResultEvent
 */
class WorldEventResultEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<WorldEventResultEvent>
{
public:
    WorldEventResultEvent(WorldEventCode worldEventCode,
        WorldEventStateType stateType) :
        worldEventCode_(worldEventCode),
        stateType_(stateType){}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* callback = entity.getController().queryWorldEventCallback();
        if (callback) {
            callback->worldEventResult(worldEventCode_, stateType_);
        }
    }

private:
    const WorldEventCode worldEventCode_;
    WorldEventStateType stateType_;
};


/**
 * @class CloseWorldEventEvent
 */
class CloseWorldEventEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CloseWorldEventEvent>
{
public:
    CloseWorldEventEvent(WorldEventCode worldEventCode) :
        worldEventCode_(worldEventCode) {}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* callback = entity.getController().queryWorldEventCallback();
        if (callback) {
            callback->worldEventClosed(worldEventCode_);
        }
    }

private:
    const WorldEventCode worldEventCode_;
};



/**
 * @class ChangeWorldEventPhaseEvent
 */
class ChangeWorldEventPhaseEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ChangeWorldEventPhaseEvent>
{
public:
    ChangeWorldEventPhaseEvent(WorldEventCode worldEventCode, uint32_t phase) :
        worldEventCode_(worldEventCode),
        phase_(phase) {}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* callback = entity.getController().queryWorldEventCallback();
        if (callback) {
            callback->worldEventPhaseChanged(worldEventCode_, phase_);
        }
    }

private:
    const WorldEventCode worldEventCode_;
    uint32_t phase_;
};



} // namespace

WorldEventPlayState::WorldEventPlayState(WorldEventScoreCallback& callback, 
    WorldEventCode worldEventCode,
    WorldEvent::EventPhaseMap& eventPhasesMap,
    sec_t closeSec, sec_t failSec, sec_t eventPhaseWaitSec) :
    scoreCallback_(callback),
    worldEventCode_(worldEventCode),
    eventPhasesMap_(eventPhasesMap),
    closeSec_(closeSec),
    failSec_(failSec),
    waitExpireTime_(0),
    eventPhaseWaitSec_(eventPhaseWaitSec)
{
}

// = WorldEventState overriding

bool WorldEventPlayState::shouldNextState() const
{
    if (startTime_ + closeSec_ < getTime()) {
        return true;
    }

    return isCompleteWorldEvent(stateType_);
}


bool WorldEventPlayState::canEventRewardForLoginPlayer() const
{
    if (isCompleteWorldEvent(stateType_) || isFailWorldEvent(stateType_)) {
        return true;
    }
    return false;
}


void WorldEventPlayState::changeNextState()
{    
    if (isCompleteWorldEvent(stateType_)) {
        scoreCallback_.reward(true);
    }

    auto event = std::make_shared<CloseWorldEventEvent>(worldEventCode_);
    WORLD->broadcast(event);

    stateType_ = westWait;
    startTime_ = 0;
    waitExpireTime_ = 0;
    WorldEvent::EventPhaseMap::iterator pos = eventPhasesMap_.begin();
    WorldEvent::EventPhaseMap::iterator end = eventPhasesMap_.end();
    for (; pos != end; ++pos) {
        WorldEventPhase* phase = (*pos).second;
        phase->deactive();        
    }
}


void WorldEventPlayState::reInitialize()
{
    stateType_ = westPlay;
    startTime_ = getTime();
    waitExpireTime_ = 0;
    currentEventPhase_ = 1;
    getCurrentEventPhase()->active();    

    auto event = std::make_shared<OpenWorldEventEvent>(worldEventCode_);
    WORLD->broadcast(event);    
}


void WorldEventPlayState::update()
{
    if (isCompleteWorldEvent(stateType_)) {
        return;
    }

    getCurrentEventPhase()->update();    

    if (canNextEventPhase()) {
        if (waitExpireTime_ == 0) {
            waitExpireTime_ = getTime() + eventPhaseWaitSec_;
        }
        if (waitExpireTime_ < getTime()) {
            changeNextEventPhase();
        }
        return;
    }

    updateEventState();
}


void WorldEventPlayState::fillWorldEventInfo(WorldEventInfos& worldEventInfos)
{
    WorldEventInfo eventInfo;
    eventInfo.worldEventCode_ = worldEventCode_;
    eventInfo.startEventTime_ = startTime_;
    eventInfo.eventPhase_ = static_cast<uint8_t>(currentEventPhase_);    

    getCurrentEventPhase()->fillWorldEventMissionInfo(eventInfo.infos_);
    worldEventInfos.push_back(eventInfo);
}


bool WorldEventPlayState::canNextEventPhase() const
{
    if (isLastEventPhase()) {
        return false;
    }

    return getCurrentEventPhase()->isComplete();
}


bool WorldEventPlayState::isLastEventPhase() const
{
    return eventPhasesMap_.size() == currentEventPhase_;
}


bool WorldEventPlayState::isFailEventPhase() const
{
    WorldEvent::EventPhaseMap::iterator pos = eventPhasesMap_.begin();
    WorldEvent::EventPhaseMap::iterator end = eventPhasesMap_.end();
    for (pos; pos != end; ++pos) {
        WorldEventPhase* currentPhases = (*pos).second;
        if (currentPhases->isFail()) {
            return true;
        }
    }
    return false;
}


bool WorldEventPlayState::isCompleteEventPhase() const
{
    WorldEvent::EventPhaseMap::iterator pos = eventPhasesMap_.begin();
    WorldEvent::EventPhaseMap::iterator end = eventPhasesMap_.end();
    for (pos; pos != end; ++pos) {
        WorldEventPhase* currentPhases = (*pos).second;
        if (! currentPhases->isComplete()) {
            return false;
        }
    }
    return true;
}


void WorldEventPlayState::changeNextEventPhase()
{
    waitExpireTime_ = 0;
    ++currentEventPhase_;
    getCurrentEventPhase()->active();
    auto event = std::make_shared<ChangeWorldEventPhaseEvent>(worldEventCode_, currentEventPhase_);
    WORLD->broadcast(event);
}


void WorldEventPlayState::enter(go::Entity& player, WorldEventMissionCode missionCode)
{
    WorldEventPhase* phase = getWorldEventPhase(missionCode);
    if (phase) {
        phase->enter(missionCode, player);
    }   
}


void WorldEventPlayState::leave(ObjectId playerId, WorldEventMissionCode missionCode)
{
    WorldEventPhase* phase = getWorldEventPhase(missionCode);
    if (phase) {
        phase->leave(missionCode, playerId);
    }
}


void WorldEventPlayState::updateEventState()
{
    if (! isPlayWorldEvent(stateType_)) {
        return;
    }

    if (startTime_ + failSec_ < getTime()) {
        getCurrentEventPhase()->setFailMission();
        stateType_ = westFailed;
    }

    if (isFailEventPhase()) {
        stateType_ = westFailed;
        getCurrentEventPhase()->setFailMission();
        auto event = std::make_shared<WorldEventResultEvent>(worldEventCode_, stateType_);
        WORLD->broadcast(event);
        scoreCallback_.reward(false);
        return;
    }

    if (isLastEventPhase()) {
        if (isCompleteEventPhase()) {
            stateType_ = westComplete;
            auto event = std::make_shared<WorldEventResultEvent>(worldEventCode_, stateType_);
            WORLD->broadcast(event);
            scoreCallback_.reward(true);
        }
    }
}


WorldEventPhase* WorldEventPlayState::getWorldEventPhase(WorldEventMissionCode missionCode)
{
    WorldEvent::EventPhaseMap::iterator pos = eventPhasesMap_.begin();
    WorldEvent::EventPhaseMap::iterator end = eventPhasesMap_.end();

    for (pos; pos != end; ++pos) {
        WorldEventPhase* currentPhases = (*pos).second;

        if (currentPhases->hasMission(missionCode)) {
            return currentPhases;        
        }
    }

    return nullptr;
}


const WorldEventPhase* WorldEventPlayState::getCurrentEventPhase() const
{
    WorldEvent::EventPhaseMap::const_iterator pos = eventPhasesMap_.find(currentEventPhase_);
    if (pos != eventPhasesMap_.end()) {
        return (*pos).second;
    }
    assert(false);
    return nullptr;
}


WorldEventPhase* WorldEventPlayState::getCurrentEventPhase()
{
    WorldEvent::EventPhaseMap::iterator pos = eventPhasesMap_.find(currentEventPhase_);
    if (pos != eventPhasesMap_.end()) {
        return (*pos).second;
    }
    assert(false);
    return nullptr;
}

}} // namespace gideon { namespace zoneserver {
