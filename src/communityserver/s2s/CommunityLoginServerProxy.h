#pragma once

#include "../communityserver_export.h"
#include <gideon/server/rpc/CommunityLoginRpc.h>
#include <sne/server/s2s/ClientSideProxy.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace communityserver {

/**
 * @class CommunityLoginServerProxy
 *
 * 로그인 서버 Proxy
 */
class CommunityServer_Export CommunityLoginServerProxy :
    public sne::server::ClientSideProxy,
    public rpc::CommunityLoginRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(CommunityLoginServerProxy);

public:
    CommunityLoginServerProxy(const sne::server::ServerInfo& serverInfo,
        const sne::server::ClientSideSessionConfig& config =
            sne::server::ClientSideSessionConfig());
    virtual ~CommunityLoginServerProxy();

    void ready();

public:
    virtual bool isActivated() const {
        return sne::server::ClientSideProxy::isActivated() &&
            isRendezvoused_;
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    // = rpc::CommunityLoginRpc overriding
    OVERRIDE_SRPC_METHOD_1(m2l_rendezvous,
        AccountIds, onlineUsers);
    OVERRIDE_SRPC_METHOD_0(m2l_onRendezvous);

    OVERRIDE_SRPC_METHOD_0(m2l_ready);

    OVERRIDE_SRPC_METHOD_1(m2l_loginCommunityUser,
        Certificate, certificate);
    OVERRIDE_SRPC_METHOD_3(m2l_onLoginCommunityUser,
        ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);

    OVERRIDE_SRPC_METHOD_1(m2l_logoutUser,
        AccountId, accountId);

    OVERRIDE_SRPC_METHOD_2(m2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);

private:
    // = sne::server::ClientSideProxy overriding
    virtual void onAuthenticated();
    virtual void onDisconnected();

private:
    bool isRendezvoused_;
    bool isReadied_;
};

}} // namespace gideon { namespace communityserver {
