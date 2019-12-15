#include "CommunityServerPCH.h"
#include "CommunityLoginServerProxy.h"
#include "../CommunityService.h"
#include "../user/CommunityUserManager.h"
#include "../user/CommunityUser.h"
#include <gideon/cs/shared/data/ServerType.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace communityserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(CommunityLoginServerProxy);

CommunityLoginServerProxy::CommunityLoginServerProxy(
    const sne::server::ServerInfo& serverInfo,
    const sne::server::ClientSideSessionConfig& config) :
    sne::server::ClientSideProxy(serverInfo, config),
    isRendezvoused_(false),
    isReadied_(false)
{
    sne::base::Session* session = getSession();
    if (session != nullptr) {
        sne::sgp::RpcingExtension* extension =
            session->getImpl().getExtension<sne::sgp::RpcingExtension>();
        extension->registerRpcForwarder(*this);
        extension->registerRpcReceiver(*this);
    }
}


CommunityLoginServerProxy::~CommunityLoginServerProxy()
{
}


void CommunityLoginServerProxy::ready()
{
    isReadied_ = true;

    m2l_ready();
}

// = sne::server::ClientSideProxy overriding

void CommunityLoginServerProxy::onAuthenticated()
{
    sne::server::ClientSideProxy::onAuthenticated();

    SNE_LOG_INFO("LoginServer is connected.");

    const AccountIds onlineUsers = COMMUNITYUSER_MANAGER->getOnlineUsers();
    m2l_rendezvous(onlineUsers);
}


void CommunityLoginServerProxy::onDisconnected()
{
    sne::server::ClientSideProxy::onDisconnected();

    SNE_LOG_ERROR("LoginServer is disconnected.");

    isRendezvoused_ = false;
}

// = sne::srpc::RpcForwarder overriding

void CommunityLoginServerProxy::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityLoginServerProxy::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void CommunityLoginServerProxy::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityLoginServerProxy::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::CommunityLoginRpc overriding

FORWARD_SRPC_METHOD_1(CommunityLoginServerProxy, m2l_rendezvous,
    AccountIds, onlineUsers);


RECEIVE_SRPC_METHOD_0(CommunityLoginServerProxy, m2l_onRendezvous)
{
    isRendezvoused_ = true;

    if (isReadied_) {
        m2l_ready();
    }
}


FORWARD_SRPC_METHOD_0(CommunityLoginServerProxy, m2l_ready);


FORWARD_SRPC_METHOD_1(CommunityLoginServerProxy, m2l_loginCommunityUser,
    Certificate, certificate);


RECEIVE_SRPC_METHOD_3(CommunityLoginServerProxy, m2l_onLoginCommunityUser,
    ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate)
{
    COMMUNITYUSER_MANAGER->loginResponsed(errorCode, accountInfo, reissuedCertificate);
}


FORWARD_SRPC_METHOD_1(CommunityLoginServerProxy, m2l_logoutUser,
    AccountId, accountId);


RECEIVE_SRPC_METHOD_2(CommunityLoginServerProxy, m2l_evUserExpelled,
    AccountId, accountId, ExpelReason, expelReason)
{
    COMMUNITYUSER_MANAGER->userExpelledFromLoginServer(accountId, expelReason);
}

}} // namespace gideon { namespace communityserver {
