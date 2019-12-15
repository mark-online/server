#pragma once

#include "WorldEventState.h"
#include "../WorldEvent.h"
#include "../callback/WorldEventPlayCallback.h"
#include "../callback/WorldEventScoreCallback.h"
#include <gideon/cs/shared/data/Time.h>


namespace gdt {
class world_event_t;
} // namespace gideon { namespace datatable {


namespace gideon { namespace zoneserver {


/***
 * @class WorldEventPlayState
 ***/
class WorldEventPlayState : public WorldEventState,
     public WorldEventPlayCallback
{   
    typedef std::mutex LockType;   
public:
    WorldEventPlayState(WorldEventScoreCallback& callback, WorldEventCode worldEventCode,
        WorldEvent::EventPhaseMap& eventPhasesMap, sec_t closeSec, sec_t failSec,
        sec_t eventPhaseWaitSec);
    virtual ~WorldEventPlayState() {}

private:
    // = WorldEventState overriding
    virtual WorldEventPlayCallback* queryWorldEventPlayCallback() {
        return this;
    }

    virtual WorldEventState::StateType getNextStateType() const {
        return stWait;
    }
    virtual WorldEventState::StateType getStateType() const {
        return stPlay;
    }

    virtual bool shouldNextState() const;
    virtual bool canEventRewardForLoginPlayer() const;

    virtual void changeNextState();
    virtual void reInitialize();
    virtual void update();

    virtual void fillWorldEventInfo(WorldEventInfos& worldEventInfos);
    
private:
    // = WorldEventPlayCallback overriding
    virtual void enter(go::Entity& player, RegionCode regionCode);
    virtual void leave(ObjectId playerId, RegionCode regionCode);
   
private:
    bool canNextEventPhase() const;
    bool isLastEventPhase() const;
    bool isFailEventPhase() const;
    bool isCompleteEventPhase() const;
    
    void updateEventState();
    void changeNextEventPhase();

private:
    WorldEventPhase* getWorldEventPhase(WorldEventMissionCode missionCode);
    
    const WorldEventPhase* getCurrentEventPhase() const;
    WorldEventPhase* getCurrentEventPhase();

private:
    WorldEvent::EventPhaseMap& eventPhasesMap_;
    WorldEventScoreCallback& scoreCallback_;
    PlayerScoreInfoMap playerScoreInfoMap_;
    WorldEventCode worldEventCode_;
    const sec_t closeSec_;
    const sec_t failSec_;
    const sec_t eventPhaseWaitSec_;
    sec_t startTime_;
    sec_t waitExpireTime_; // 미션 판 넘어갈때 대기 시간
    uint32_t currentEventPhase_;
    WorldEventStateType stateType_;
};

}} // namespace gideon { namespace zoneserver {
