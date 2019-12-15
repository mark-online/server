#include "ZoneServerPCH.h"
#include "PlayerWorldEventController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../service/world_event/WorldEventService.h"
#include "../../service/world_event/WorldEvent.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerWorldEventController);

PlayerWorldEventController::PlayerWorldEventController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerWorldEventController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerWorldEventController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerWorldEventController::worldEventOpend(WorldEventCode worldEventCode)
{
    evWorldEventOpened(worldEventCode);
}


void PlayerWorldEventController::worldEventJoined(WorldEventCode worldEventCode,
    WorldEventMissionCode missionCode, uint32_t currentPhase, PartyId partyId)
{
    evWorldEventMissionJoined(worldEventCode, missionCode, static_cast<uint8_t>(currentPhase), partyId);
}


void PlayerWorldEventController::worldEventPhaseChanged(WorldEventCode worldEventCode, uint32_t currentPhase)
{
    evWorldEventPhaseChanged(worldEventCode, static_cast<uint8_t>(currentPhase));
}


void PlayerWorldEventController::worldEventMissionPhaseChanged(WorldEventCode worldEventCode, 
    WorldEventMissionCode missionCode, uint32_t currentPhase)
{
    evWorldEventMissionPhaseChanged(worldEventCode, missionCode, static_cast<uint8_t>(currentPhase));
}


void PlayerWorldEventController::worldEventMissionResult(WorldEventCode worldEventCode, 
    WorldEventMissionCode missionCode, WorldEventMissionStateType stateType)
{
    evWorldEventMissionResult(worldEventCode, missionCode, stateType);
}


void PlayerWorldEventController::worldEventClosed(WorldEventCode worldEventCode)
{
    evWorldEventClosed(worldEventCode);
}


void PlayerWorldEventController::worldEventResult(WorldEventCode worldEventCode, 
    WorldEventStateType stateType)
{
    evWorldEventResult(worldEventCode, stateType);
}


void PlayerWorldEventController::worldEventTopRankerRewardInfo(WorldEventCode worldEventCode, 
    const RankingRewardInfos& rakingRewardInfos)
{
    evWorldEventTopRankerRewardInfo(worldEventCode, rakingRewardInfos);
}


void PlayerWorldEventController::worldEventEntityMoved(const GameObjectInfo& entityInfo, DataCode datacode, const Position& position)
{
    evWorldEventEntityMoved(entityInfo, datacode, position);
}


void PlayerWorldEventController::worldEventEntityDespawned(const GameObjectInfo& entityInfo)
{
    evWorldEventEntityDespawned(entityInfo);
}


void PlayerWorldEventController::worldEventKillMissionInfo(WorldEventCode worldEventCode,
    WorldEventMissionCode missionCode, uint8_t currentMissionPhase,
    const WorldEventKillMissionInfoMap& infoMap)
{
    evWorldEventKillMissionInfo(worldEventCode, missionCode, currentMissionPhase, infoMap);
}


void PlayerWorldEventController::worldEventKillMissionInfoUpdated(WorldEventCode worldEventCode,
    WorldEventMissionCode missionCode, DataCode entityCode, uint8_t currentCount)
{
    evWorldEventKillMissionInfoUpdated(worldEventCode, missionCode, entityCode, currentCount);
}


void PlayerWorldEventController::worldEventRewarded(WorldEventCode worldEventCode, 
    CostType costType, uint32_t rewardValue)
{
    if (rewardValue == 0) {
        return;
    }
    uint64_t currentValue = 0;
    go::Moneyable* moneyable = getOwner().queryMoneyable();
    if (moneyable) {
        if (costType == ctGameMoney) {
            moneyable->upGameMoney(rewardValue);
            currentValue = moneyable->getGameMoney();
        }
        else if (costType == ctEventCoin) {
            moneyable->upEventCoin(rewardValue);
            currentValue = moneyable->getEventCoin();
        }
        else if (costType == ctForgeCoin) {
            moneyable->upForgeCoin(rewardValue);
            currentValue = moneyable->getForgeCoin();
        }
        else {
            assert(false);
        }
    }

    evWorldEventRewarded(worldEventCode, costType, currentValue);
}



RECEIVE_SRPC_METHOD_0(PlayerWorldEventController, queryWorldEventInfo)
{
    WorldEventInfos worldEventInfos;
    WORLD_EVENT_SERVICE->fillWorldEventInfos(worldEventInfos);
    onWorldEventInfo(worldEventInfos);
}


RECEIVE_SRPC_METHOD_1(PlayerWorldEventController, queryWorldEventRankingScoreInfo,
    WorldEventCode, worldEventCode)
{
    WorldEventRankerInfos infos;
    uint32_t myScore = 0;
    WorldEvent* worldEvent = WORLD_EVENT_SERVICE->getWorldEvent(worldEventCode);
    if (worldEvent) {
        worldEvent->fillWorldEventRankerInfo(infos, myScore, getOwner().getObjectId());
    }
    onWorldEventRankingScoreInfo(infos, myScore);
}


FORWARD_SRPC_METHOD_1(PlayerWorldEventController, onWorldEventInfo,
    WorldEventInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerWorldEventController, onWorldEventRankingScoreInfo,
    WorldEventRankerInfos, infos, uint32_t, myScore);


FORWARD_SRPC_METHOD_1(PlayerWorldEventController, evWorldEventOpened,
    WorldEventCode, worldEventCode);


FORWARD_SRPC_METHOD_4(PlayerWorldEventController, evWorldEventMissionJoined,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, 
    uint8_t, currentPhase, PartyId, partyId);


FORWARD_SRPC_METHOD_2(PlayerWorldEventController, evWorldEventPhaseChanged,
    WorldEventCode, worldEventCode, uint8_t, currentEventPhase);


FORWARD_SRPC_METHOD_3(PlayerWorldEventController, evWorldEventMissionPhaseChanged,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, uint8_t, currentMissionPhase);


FORWARD_SRPC_METHOD_3(PlayerWorldEventController, evWorldEventMissionResult,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode,
    WorldEventMissionStateType, missionStateType);


FORWARD_SRPC_METHOD_1(PlayerWorldEventController, evWorldEventClosed,
    WorldEventCode, worldEventCode);


FORWARD_SRPC_METHOD_2(PlayerWorldEventController, evWorldEventResult,
    WorldEventCode, worldEventCode, WorldEventStateType, missionStateType);


FORWARD_SRPC_METHOD_3(PlayerWorldEventController, evWorldEventRewarded,
    WorldEventCode, worldEventCode, CostType, costType, uint64_t, currentValue);


FORWARD_SRPC_METHOD_2(PlayerWorldEventController, evWorldEventTopRankerRewardInfo, 
    WorldEventCode, worldEventCode, RankingRewardInfos, rakingRewardInfos);


FORWARD_SRPC_METHOD_3(PlayerWorldEventController, evWorldEventEntityMoved,
    GameObjectInfo, objectInfo, DataCode, dataCode, Position, position);


FORWARD_SRPC_METHOD_1(PlayerWorldEventController, evWorldEventEntityDespawned,
    GameObjectInfo, objectInfo);


FORWARD_SRPC_METHOD_4(PlayerWorldEventController, evWorldEventKillMissionInfo,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode,
    uint8_t, currentMissionPhase, WorldEventKillMissionInfoMap, infoMap);


FORWARD_SRPC_METHOD_4(PlayerWorldEventController, evWorldEventKillMissionInfoUpdated,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode,
    DataCode, entityCode, uint8_t, currentCount);

// = sne::srpc::RpcForwarder overriding

void PlayerWorldEventController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTreasureController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerWorldEventController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTreasureController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {