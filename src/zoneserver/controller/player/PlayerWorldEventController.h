#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/WorldEventCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/WorldEventRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerInventoryController
 * 아이템 담당
 */
class ZoneServer_Export PlayerWorldEventController : public Controller,
    public rpc::WorldEventRpc,
    public WorldEventCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerWorldEventController);
public:
    PlayerWorldEventController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = WorldEventCallback overriding
    virtual void worldEventOpend(WorldEventCode worldEventCode);
    virtual void worldEventJoined(WorldEventCode worldEventCode, WorldEventMissionCode missionCode, 
        uint32_t currentPhase, PartyId partyId);
    virtual void worldEventPhaseChanged(WorldEventCode worldEventCode, uint32_t currentPhase);
    virtual void worldEventMissionPhaseChanged(WorldEventCode worldEventCode, 
        WorldEventMissionCode missionCode, uint32_t currentPhase);
    virtual void worldEventRewarded(WorldEventCode worldEventCode, 
        CostType costType, uint32_t rewardValue);
    virtual void worldEventMissionResult(WorldEventCode worldEventCode, 
        WorldEventMissionCode missionCode, WorldEventMissionStateType stateType);
    virtual void worldEventClosed(WorldEventCode worldEventCode);
    virtual void worldEventResult(WorldEventCode worldEventCode, 
        WorldEventStateType stateType);
    virtual void worldEventTopRankerRewardInfo(WorldEventCode worldEventCode, 
        const RankingRewardInfos& rakingRewardInfos);
    virtual void worldEventEntityMoved(const GameObjectInfo& entityInfo, DataCode datacode, const Position& position);
    virtual void worldEventEntityDespawned(const GameObjectInfo& entityInfo);

    virtual void worldEventKillMissionInfo(WorldEventCode worldEventCode,
        WorldEventMissionCode missionCode, uint8_t currentMissionPhase,
        const WorldEventKillMissionInfoMap& infoMap);
    virtual void worldEventKillMissionInfoUpdated(WorldEventCode worldEventCode,
        WorldEventMissionCode missionCode, DataCode entityCode, uint8_t currentCount);

public:
    // 존에 입장시에 정보를 요청한다.
    OVERRIDE_SRPC_METHOD_0(queryWorldEventInfo);
    OVERRIDE_SRPC_METHOD_1(queryWorldEventRankingScoreInfo,
        WorldEventCode, worldEventCode);
public:
    OVERRIDE_SRPC_METHOD_1(onWorldEventInfo,
        WorldEventInfos, infos);

    OVERRIDE_SRPC_METHOD_2(onWorldEventRankingScoreInfo,
        WorldEventRankerInfos, infos, uint32_t, myScore);

public:
    // 월드에서 이벤트가 열렸을때
    OVERRIDE_SRPC_METHOD_1(evWorldEventOpened,
        WorldEventCode, worldEventCode);

    OVERRIDE_SRPC_METHOD_4(evWorldEventMissionJoined,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, 
        uint8_t, currentPhase, PartyId, partyId);

    OVERRIDE_SRPC_METHOD_2(evWorldEventPhaseChanged,
        WorldEventCode, worldEventCode, uint8_t, currentEventPhase);

    OVERRIDE_SRPC_METHOD_3(evWorldEventMissionPhaseChanged,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, uint8_t, currentMissionPhase);

    OVERRIDE_SRPC_METHOD_3(evWorldEventMissionResult,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode,
        WorldEventMissionStateType, missionStateType);
    OVERRIDE_SRPC_METHOD_1(evWorldEventClosed,
        WorldEventCode, worldEventCode);    
    OVERRIDE_SRPC_METHOD_2(evWorldEventResult,
        WorldEventCode, worldEventCode, WorldEventStateType, stateType);
    OVERRIDE_SRPC_METHOD_3(evWorldEventRewarded,
        WorldEventCode, worldEventCode, CostType, costType, uint64_t, currentValue);

    OVERRIDE_SRPC_METHOD_2(evWorldEventTopRankerRewardInfo, 
        WorldEventCode, worldEventCode, RankingRewardInfos, rakingRewardInfos);

    OVERRIDE_SRPC_METHOD_3(evWorldEventEntityMoved,
        GameObjectInfo, objectInfo, NpcCode, npcCode, Position, position);
    
    OVERRIDE_SRPC_METHOD_1(evWorldEventEntityDespawned,
        GameObjectInfo, objectInfo);

    OVERRIDE_SRPC_METHOD_4(evWorldEventKillMissionInfo,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode,
        uint8_t, currentMissionPhase, WorldEventKillMissionInfoMap, infoMap);
    OVERRIDE_SRPC_METHOD_4(evWorldEventKillMissionInfoUpdated,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode,
        DataCode, entityCode, uint8_t, currentCount);
private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
