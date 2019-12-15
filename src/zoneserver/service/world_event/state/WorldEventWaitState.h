#pragma once

#include "WorldEventState.h"

namespace gdt {
class world_event_t;
} // namespace gideon { namespace datatable {


namespace gideon { namespace zoneserver {

class WorldEventScoreCallback;

/***
 * @class WorldEventWaitState
 ***/
class WorldEventWaitState : public WorldEventState
{   
public:
    WorldEventWaitState(WorldEventScoreCallback& callback, const gdt::world_event_t& eventTemplate);
    virtual ~WorldEventWaitState() {}

    virtual WorldEventState::StateType getNextStateType() const {
        return stReady;
    }
    virtual WorldEventState::StateType getStateType() const {
        return stWait;
    }

    virtual bool shouldNextState() const;
    virtual bool canEventRewardForLoginPlayer() const {
        return true;
    }
    virtual void changeNextState();
    virtual void reInitialize();

private:
    WorldEventCode worldEventCode_;
    WorldEventOpenType eventOpenType_;;
    uint32_t openParam1_;
    uint32_t openParam2_;
    sec_t randomOpenTime_;
    WorldEventScoreCallback& callback_;
};

}} // namespace gideon { namespace zoneserver {
