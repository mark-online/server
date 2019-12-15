#include "ZoneServerPCH.h"
#include "PlayerMailController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../service/mail/MailService.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon { namespace zoneserver { namespace gc {

// = PlayerMailController
IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerMailController);

void PlayerMailController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerMailController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerMailController::mailSynchronized(const MailTitleInfos& titles)
{
    evMailSynchronized(titles);
}


void PlayerMailController::mailReceived(MailId mailId)
{
    evMailReceived(mailId);
}


RECEIVE_SRPC_METHOD_1(PlayerMailController, sendMail,
    SendMailInfo, sendMailInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = MAIL_SERVICE->sendMail(owner, sendMailInfo);
    onSendMail(errorCode, sendMailInfo, owner.queryMoneyable()->getGameMoney());    
}


RECEIVE_SRPC_METHOD_0(PlayerMailController, synchronizeMail)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = MAIL_SERVICE->syncMail(owner.getObjectId());
    onSynchronizeMail(errorCode);
}


RECEIVE_SRPC_METHOD_1(PlayerMailController, queryMailTitle,
    ObjectId, mailId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    MailTitleInfo mailInfo;
    const ErrorCode errorCode = 
        MAIL_SERVICE->queryMailTitle(mailInfo, owner.getObjectId(), mailId);
    onMailTitle(errorCode, mailInfo);
}


RECEIVE_SRPC_METHOD_1(PlayerMailController, queryMailContent,
    ObjectId, mailId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    MailContentInfo mailInfo;
    const ErrorCode errorCode = 
        MAIL_SERVICE->queryMailContent(mailInfo, owner.getObjectId(), mailId);
    onMailContent(errorCode, mailInfo);
}


RECEIVE_SRPC_METHOD_1(PlayerMailController, aquireItemInMail,
    ObjectId, mailId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    const ErrorCode errorCode = 
        MAIL_SERVICE->aquireMailItems(owner, mailId);
    onAquireItemInMail(errorCode, mailId);
}


RECEIVE_SRPC_METHOD_1(PlayerMailController, deleteMail,
    ObjectId, mailId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    const ErrorCode errorCode = 
        MAIL_SERVICE->deleteMail(owner.getObjectId(), mailId);
    onDeleteMail(errorCode, mailId);
}


FORWARD_SRPC_METHOD_3(PlayerMailController, onSendMail,
    ErrorCode, errorCode, SendMailInfo, mailInfo, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_1(PlayerMailController, onSynchronizeMail,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(PlayerMailController, onMailTitle,
    ErrorCode, errorCode, MailTitleInfo, mailTitleInfo);


FORWARD_SRPC_METHOD_2(PlayerMailController, onMailContent,
    ErrorCode, errorCode, MailContentInfo, mailContentInfo);    


FORWARD_SRPC_METHOD_2(PlayerMailController, onAquireItemInMail,
     ErrorCode, errorCode, ObjectId, mailId);


FORWARD_SRPC_METHOD_2(PlayerMailController, onDeleteMail,
    ErrorCode, errorCode, ObjectId, mailId);


FORWARD_SRPC_METHOD_1(PlayerMailController, evMailSynchronized,
    MailTitleInfos, mailTitleInfos);


FORWARD_SRPC_METHOD_1(PlayerMailController, evMailReceived,
    ObjectId, mailId);

// = sne::srpc::RpcForwarder overriding

void PlayerMailController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerCastController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerMailController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerCastController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {