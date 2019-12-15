#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/AchievementCallback.h"
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/AchievementRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerAchievementController
 * 업적 담당
 */
class ZoneServer_Export PlayerAchievementController : public Controller,
    public rpc::AchievementRpc,           
    public AchievementCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerAchievementController);
public:
    PlayerAchievementController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

private:
    virtual void achievementUpdated(AchievementCode code, const ProcessAchievementInfo& missionInfo);
    virtual void achievementCompleted(AchievementCode code, AchievementPoint point, sec_t now);

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);


private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    OVERRIDE_SRPC_METHOD_0(queryAchievements);

    OVERRIDE_SRPC_METHOD_3(onQueryAchievements,
        ErrorCode, errorCode, ProcessAchievementInfoMap, processMap,
        CompleteAchievementInfoMap, completeMap);

    OVERRIDE_SRPC_METHOD_2(evAchievementUpdated,
        AchievementCode, code, ProcessAchievementInfo, missionInfo);
    OVERRIDE_SRPC_METHOD_3(evAchievementCompleted,
        AchievementCode, code, AchievementPoint, point, sec_t, now);

private:
    AccountId accountId_;
    ObjectId characterId_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
