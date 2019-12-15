#pragma once

#include <gideon/cs/shared/data/WorldEventInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/PartyInfo.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

class WorldEventPlayCallback;

/***
 * @class WorldEventState
 ***/
class WorldEventState
{   
public:
    enum StateType {
        stWait,
        stReady,
        stPlay,
        stCount
    };
public:
    WorldEventState() {}
    virtual ~WorldEventState() {}
    virtual WorldEventPlayCallback* queryWorldEventPlayCallback() {
        return nullptr;
    }
    virtual WorldEventState::StateType getNextStateType() const = 0;
    virtual WorldEventState::StateType getStateType() const = 0;

    virtual bool shouldNextState() const = 0;
    virtual bool canEventRewardForLoginPlayer() const = 0;

    virtual void changeNextState() = 0;
    virtual void reInitialize() = 0;
    virtual void update() {}

    virtual void fillWorldEventInfo(WorldEventInfos& /*worldEventInfos*/) {}
    virtual void setParty(WorldEventMissionCode /*missionCode*/, PartyId /*partyId*/) {}

    virtual void enter(go::Entity& /*player*/, RegionCode /*regionCode*/) {}
    virtual void leave(ObjectId /*playerId*/, RegionCode /*regionCode*/) {}
};

}} // namespace gideon { namespace zoneserver {
