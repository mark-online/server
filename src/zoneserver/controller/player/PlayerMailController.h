#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/MailCallback.h"
#include <gideon/cs/shared/rpc/player/MailRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerMailController
 * 메일 담당
 */
class ZoneServer_Export PlayerMailController : public Controller,
    public rpc::MailRpc,
    public MailCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerMailController);
public:
    PlayerMailController(go::Entity* owner) :
        Controller(owner) {}

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

    virtual void initialize() {}

    virtual void mailSynchronized(const MailTitleInfos& titles);
    virtual void mailReceived(MailId mailId);

public:
    OVERRIDE_SRPC_METHOD_1(sendMail,
        SendMailInfo, sendMailInfo);
    OVERRIDE_SRPC_METHOD_0(synchronizeMail);
    OVERRIDE_SRPC_METHOD_1(queryMailTitle,
        ObjectId, mailId);
    OVERRIDE_SRPC_METHOD_1(queryMailContent,
        ObjectId, mailId);
    OVERRIDE_SRPC_METHOD_1(aquireItemInMail,
        ObjectId, mailId);
    OVERRIDE_SRPC_METHOD_1(deleteMail,
        ObjectId, mailId);

    OVERRIDE_SRPC_METHOD_3(onSendMail,
        ErrorCode, errorCode, SendMailInfo, mailInfo, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_1(onSynchronizeMail,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onMailTitle,
        ErrorCode, errorCode, MailTitleInfo, mailTitleInfo);    
    OVERRIDE_SRPC_METHOD_2(onMailContent,
        ErrorCode, errorCode, MailContentInfo, mailContentInfo);    
    OVERRIDE_SRPC_METHOD_2(onAquireItemInMail,
        ErrorCode, errorCode, ObjectId, mailId);
    OVERRIDE_SRPC_METHOD_2(onDeleteMail,
        ErrorCode, errorCode, ObjectId, mailId);

    OVERRIDE_SRPC_METHOD_1(evMailSynchronized,
        MailTitleInfos, mailTitleInfos);
    OVERRIDE_SRPC_METHOD_1(evMailReceived,
        ObjectId, mailId);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {