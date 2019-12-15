#pragma once

#include "WorldEventState.h"
#include "../WorldEvent.h"

namespace gdt {
class world_event_t;
} // namespace gideon { namespace datatable {


namespace gideon { namespace zoneserver {


/***
 * @class WorldEventReadyState
 ***/
class WorldEventReadyState : public WorldEventState
{   
public:
    WorldEventReadyState(WorldEvent::EventPhaseMap& eventPhaseMap);
    virtual ~WorldEventReadyState() {}

    virtual WorldEventState::StateType getNextStateType() const {
        return stPlay;
    }
    virtual WorldEventState::StateType getStateType() const {
        return stReady;
    }

    virtual bool shouldNextState() const;
    virtual bool canEventRewardForLoginPlayer() const {return false;}
    virtual void changeNextState() {}
    virtual void reInitialize();
    virtual void setParty(WorldEventMissionCode missionCode, PartyId partyId);

private:
    WorldEventPhase* getWorldEventPhase(WorldEventMissionCode missionCode);

private:
    WorldEvent::EventPhaseMap& eventPhaseMap_;
};

}} // namespace gideon { namespace zoneserver {
