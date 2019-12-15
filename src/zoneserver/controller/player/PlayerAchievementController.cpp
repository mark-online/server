#include "ZoneServerPCH.h"
#include "PlayerAchievementController.h"
#include "../EntityController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/ability/CharacterTitleable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Rewardable.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerAchievementController);

PlayerAchievementController::PlayerAchievementController(go::Entity* owner) :
    Controller(owner),
    accountId_(owner->getAccountId()),
    characterId_(owner->getObjectId())
{
}


void PlayerAchievementController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerAchievementController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerAchievementController::achievementUpdated(AchievementCode code, const ProcessAchievementInfo& missionInfo)
{
    evAchievementUpdated(code, missionInfo);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUpdateProcessAchievement(accountId_, characterId_, code, missionInfo);    
}


void PlayerAchievementController::achievementCompleted(AchievementCode code, AchievementPoint point, sec_t now)
{
    evAchievementCompleted(code, point, now);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncCompleteAchievement(accountId_, characterId_, code, point); 
}


// = sne::srpc::RpcForwarder overriding

void PlayerAchievementController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerAchievementController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerAchievementController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerAchievementController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


RECEIVE_SRPC_METHOD_0(PlayerAchievementController, queryAchievements)
{
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncQueryAchievements(accountId_, characterId_);
}


FORWARD_SRPC_METHOD_3(PlayerAchievementController, onQueryAchievements,
    ErrorCode, errorCode, ProcessAchievementInfoMap, processMap,
    CompleteAchievementInfoMap, completeMap);


FORWARD_SRPC_METHOD_2(PlayerAchievementController, evAchievementUpdated,
    AchievementCode, code, ProcessAchievementInfo, missionInfo);


FORWARD_SRPC_METHOD_3(PlayerAchievementController, evAchievementCompleted,
    AchievementCode, code, AchievementPoint, point, sec_t, now);

}}}  // namespace gideon { namespace zoneserver { namespace gc {


