#pragma once

#include <gideon/cs/shared/data/WorldEventInfo.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/ItemInfo.h>
#include <gideon/cs/shared/data/NpcInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class WorldEventCallback
 */
class WorldEventCallback
{
public:
    virtual ~WorldEventCallback() {}

public:
	virtual void worldEventOpend(WorldEventCode worldEventCode) = 0;
    virtual void worldEventJoined(WorldEventCode worldEventCode, WorldEventMissionCode missionCode, 
        uint32_t currentPhase, PartyId partyId) = 0;
    virtual void worldEventPhaseChanged(WorldEventCode worldEventCode, uint32_t currentPhase) = 0;
    virtual void worldEventMissionPhaseChanged(WorldEventCode worldEventCode, 
        WorldEventMissionCode missionCode, uint32_t currentPhase) = 0;
    virtual void worldEventRewarded(WorldEventCode worldEventCode, 
        CostType costType, uint32_t rewardValue) = 0;
    virtual void worldEventMissionResult(WorldEventCode worldEventCode, 
        WorldEventMissionCode missionCode, WorldEventMissionStateType stateType) = 0;
    virtual void worldEventClosed(WorldEventCode worldEventCode) = 0;
    virtual void worldEventResult(WorldEventCode worldEventCode, 
        WorldEventStateType stateType) = 0;
    virtual void worldEventTopRankerRewardInfo(WorldEventCode worldEventCode, 
        const RankingRewardInfos& rakingRewardInfos) = 0;
    virtual void worldEventEntityMoved(const GameObjectInfo& entityInfo, DataCode datacode, const Position& position) = 0;
    virtual void worldEventEntityDespawned(const GameObjectInfo& entityInfo) = 0;
    virtual void worldEventKillMissionInfo(WorldEventCode worldEventCode,
        WorldEventMissionCode missionCode, uint8_t currentMissionPhase,
        const WorldEventKillMissionInfoMap& infoMap) = 0;
    virtual void worldEventKillMissionInfoUpdated(WorldEventCode worldEventCode,
        WorldEventMissionCode missionCode, DataCode entityCode, uint8_t currentCount) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
