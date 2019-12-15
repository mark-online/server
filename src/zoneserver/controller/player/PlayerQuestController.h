#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/QuestCallback.h"
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/QuestRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver {
class TradeState;
}} // namespace gideon { namespace zoneserver {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerQuestController
 * 퀘스트 담당
 */
class ZoneServer_Export PlayerQuestController : public Controller,
    public QuestCallback,
    public rpc::QuestRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerQuestController);
public:
    PlayerQuestController(go::Entity* owner);
public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    virtual void accepted(QuestCode questCode);
    virtual void accepted(QuestCode questCode, sec_t repeatAcceptTime);
    virtual void canceled(QuestCode questCode, bool isRepeatQuest);
    virtual void completed(QuestCode questCode, bool isRepeatQuest);
    virtual void missionUpdated(QuestCode questCode, QuestMissionCode missionCode,
        const QuestGoalInfo& goalInfo, bool isRepeatQuest);
    virtual void completeQuestRemoved(QuestCode questCode);
    virtual void questToPartyResult(const GameObjectInfo& targetMember, QuestCode questCode,
        QuestToPartyResultType resultType);
    virtual void questToPartyPushed(const GameObjectInfo& targetMember, QuestCode questCode);

public:
    virtual void initialize();
    virtual void finalize();

public:
    // = QuestRpc overriding
    OVERRIDE_SRPC_METHOD_2(acceptQuest,
        QuestCode, questCode, GameObjectInfo, objectInfo);
    OVERRIDE_SRPC_METHOD_1(cancelQuest,
        QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_2(completeTransportMission,
        QuestCode, questCode, QuestMissionCode, questMissionCode);   
    OVERRIDE_SRPC_METHOD_3(completeQuest,
        QuestCode, questCode, ObjectId, npcId, DataCode, selectItemCode);
    OVERRIDE_SRPC_METHOD_4(completeTransportQuest,
        QuestCode, questCode, QuestMissionCode, questMissionCode, ObjectId, npcId, DataCode, selectItemCode);
    
    OVERRIDE_SRPC_METHOD_2(doContentsQuest, 
        QuestContentMissionType, contentMissionType, DataCode, dataCode);
    
    OVERRIDE_SRPC_METHOD_1(pushQuestToParty, 
        QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_3(responsePushQuestToParty, 
        QuestCode, questCode, GameObjectInfo, pusherInfo, bool, isAccept);

    OVERRIDE_SRPC_METHOD_2(onAcceptQuest,
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_2(onCancelQuest,
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_3(onCompleteTransportMission,
        ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode);
    OVERRIDE_SRPC_METHOD_3(onCompleteQuest,
        ErrorCode, errorCode, QuestCode, questCode, DataCode, selectItemCode);
    OVERRIDE_SRPC_METHOD_5(onCompleteTransportQuest,
        ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode, ObjectId, npcId, DataCode, selectItemCode);
    OVERRIDE_SRPC_METHOD_2(onPushQuestToParty, 
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_3(onResponsePushQuestToParty, 
        ErrorCode, errorCode, QuestCode, questCode, bool, isAccept);


    OVERRIDE_SRPC_METHOD_3(evQuestMissionUpdated,
        QuestCode, questCode, QuestMissionCode, missionCode,
        QuestGoalInfo, questGoalInfo);
    OVERRIDE_SRPC_METHOD_2(evQuestToPartyPushed, 
        QuestCode, questCode, GameObjectInfo, member);
    OVERRIDE_SRPC_METHOD_3(evQuestToPartyPushResult, 
        QuestCode, questCode, GameObjectInfo, member, QuestToPartyResultType, resultType);
private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    AccountId accountId_;
    ObjectId characterId_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
