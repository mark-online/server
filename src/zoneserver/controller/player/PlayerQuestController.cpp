#include "ZoneServerPCH.h"
#include "PlayerQuestController.h"
#include "../EntityController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/gameobject/ability/Questable.h"
#include "../../model/gameobject/ability/Guildable.h"
#include "../../model/quest/QuestRepository.h"
#include "../../world/World.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerQuestController);

PlayerQuestController::PlayerQuestController(go::Entity* owner) :
    Controller(owner),
    accountId_(invalidAccountId),
    characterId_(invalidObjectId)
{
}


void PlayerQuestController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerQuestController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerQuestController::accepted(QuestCode questCode)
{	
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAcceptedQuest(accountId_, characterId_, questCode); 
}


void PlayerQuestController::accepted(QuestCode questCode, sec_t repeatAcceptTime)
{
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAcceptRepeatQuest(accountId_, characterId_, questCode, repeatAcceptTime); 
}


void PlayerQuestController::canceled(QuestCode questCode, bool isRepeatQuest)
{
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncCancelQuest(accountId_, characterId_, questCode, isRepeatQuest); 
}


void PlayerQuestController::completed(QuestCode questCode, bool isRepeatQuest)
{
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncCompleteQuest(accountId_, characterId_, questCode, isRepeatQuest); 
}


void PlayerQuestController::missionUpdated(QuestCode questCode, QuestMissionCode missionCode,
    const QuestGoalInfo& goalInfo, bool isRepeatQuest)
{
    evQuestMissionUpdated(questCode, missionCode, goalInfo);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUpdateQuestMission(accountId_, characterId_, questCode, 
        missionCode, goalInfo, isRepeatQuest); 
}


void PlayerQuestController::completeQuestRemoved(QuestCode questCode)
{
	DatabaseGuard db(SNE_DATABASE_MANAGER);
	db->asyncRemoveCompleteQuest(accountId_, characterId_, questCode); 
}


void PlayerQuestController::questToPartyResult(const GameObjectInfo& targetMember, QuestCode questCode,
    QuestToPartyResultType resultType)
{
    evQuestToPartyPushResult(questCode, targetMember, resultType);
}


void PlayerQuestController::questToPartyPushed(const GameObjectInfo& targetMember, QuestCode questCode)
{
    evQuestToPartyPushed(questCode, targetMember);
}


void PlayerQuestController::initialize()
{
	accountId_ = getOwner().getAccountId();
	characterId_ = getOwner().getObjectId();
}



void PlayerQuestController::finalize()
{
	accountId_ = invalidAccountId;
	characterId_ = invalidObjectId;
}

// = QuestRpc overriding

RECEIVE_SRPC_METHOD_2(PlayerQuestController, acceptQuest,
    QuestCode, questCode, GameObjectInfo, objectInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        owner.queryQuestable()->getQuestRepository().acceptQuest(questCode, objectInfo);
    onAcceptQuest(errorCode, questCode);

}


RECEIVE_SRPC_METHOD_1(PlayerQuestController, cancelQuest,
    QuestCode, questCode)
    {
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        owner.queryQuestable()->getQuestRepository().cancelQuest(questCode);
    onCancelQuest(errorCode, questCode);
}


RECEIVE_SRPC_METHOD_2(PlayerQuestController, completeTransportMission,
    QuestCode, questCode, QuestMissionCode, questMissionCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        owner.queryQuestable()->getQuestRepository().completeTransportMission(questCode, questMissionCode);

    onCompleteTransportMission(errorCode, questCode, questMissionCode);
}


RECEIVE_SRPC_METHOD_3(PlayerQuestController, completeQuest,
    QuestCode, questCode, ObjectId, npcId, DataCode, selectItemCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        owner.queryQuestable()->getQuestRepository().completeQuest(questCode, npcId, selectItemCode);
    onCompleteQuest(errorCode, questCode, selectItemCode);
}


RECEIVE_SRPC_METHOD_4(PlayerQuestController, completeTransportQuest,
    QuestCode, questCode, QuestMissionCode, questMissionCode, ObjectId, npcId, DataCode, selectItemCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        owner.queryQuestable()->getQuestRepository().completeTransportQuest(questCode, questMissionCode,
        npcId, selectItemCode);
    onCompleteTransportQuest(errorCode, questCode, questMissionCode, npcId, selectItemCode);
}


RECEIVE_SRPC_METHOD_2(PlayerQuestController,  doContentsQuest, 
    QuestContentMissionType, contentMissionType, DataCode, dataCode)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    owner.queryQuestable()->getQuestRepository().doContents(contentMissionType, dataCode);
}


RECEIVE_SRPC_METHOD_1(PlayerQuestController,  pushQuestToParty, 
    QuestCode, questCode)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        owner.queryQuestable()->getQuestRepository().pushQuestToParty(questCode);
    onPushQuestToParty(errorCode, questCode);
}


RECEIVE_SRPC_METHOD_3(PlayerQuestController, responsePushQuestToParty, 
    QuestCode, questCode, GameObjectInfo, pusherInfo, bool, isAccept)
{    
    go::Entity& owner = getOwner();
    go::Entity* pusher = WORLD->getPlayer(pusherInfo.objectId_);
    ErrorCode errorCode = ecOk;
    if (pusher) {
        QuestToPartyResultType resultType = qtprtTargetRefusal;
        if (isAccept) {
            errorCode = owner.queryQuestable()->getQuestRepository().acceptQuest(questCode, pusherInfo);
            if (isSucceeded(errorCode)) {
                resultType = qtprtAccepted;
            }
            else {
                resultType = qtprtNotAcceptCondtion;
            }
        }
     
        pusher->getController().queryQuestCallback()->questToPartyResult(owner.getGameObjectInfo(), questCode, resultType);
    }
    else {
        errorCode = ecQuestNotFindQuestPusher;
    }

    onResponsePushQuestToParty(errorCode, questCode, isAccept);
}


FORWARD_SRPC_METHOD_2(PlayerQuestController, onAcceptQuest,
    ErrorCode, errorCode, QuestCode, questCode);


FORWARD_SRPC_METHOD_2(PlayerQuestController, onCancelQuest,
    ErrorCode, errorCode, QuestCode, questCode);


FORWARD_SRPC_METHOD_3(PlayerQuestController, onCompleteTransportMission,
    ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode);


FORWARD_SRPC_METHOD_3(PlayerQuestController, onCompleteQuest,
    ErrorCode, errorCode, QuestCode, questCode, DataCode, selectItemCode);


FORWARD_SRPC_METHOD_5(PlayerQuestController, onCompleteTransportQuest,
    ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode,
    ObjectId, npcId, DataCode, selectItemCode);


FORWARD_SRPC_METHOD_2(PlayerQuestController, onPushQuestToParty, 
    ErrorCode, errorCode, QuestCode, questCode);


FORWARD_SRPC_METHOD_3(PlayerQuestController, onResponsePushQuestToParty, 
    ErrorCode, errorCode, QuestCode, questCode, bool, isAccept);


FORWARD_SRPC_METHOD_3(PlayerQuestController, evQuestMissionUpdated,
    QuestCode, questCode, QuestMissionCode, missionCode,
    QuestGoalInfo, questGoalInfo);


FORWARD_SRPC_METHOD_2(PlayerQuestController, evQuestToPartyPushed, 
    QuestCode, questCode, GameObjectInfo, member);


FORWARD_SRPC_METHOD_3(PlayerQuestController, evQuestToPartyPushResult, 
    QuestCode, questCode, GameObjectInfo, member, QuestToPartyResultType, resultType);

// = sne::srpc::RpcForwarder overriding

void PlayerQuestController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerQuestController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerQuestController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerQuestController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {