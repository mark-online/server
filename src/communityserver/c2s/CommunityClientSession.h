#pragma once

#include "CommunityClientSessionCallback.h"
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/rpc/detail/LoginRpc.h>
#include <sne/server/session/ServerSideSession.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace communityserver {

/**
 * @class CommunityClientSession
 *
 * Server-side client session
 */
class CommunityClientSession :
    public sne::server::ServerSideSession,
    public CommunityClientSessionCallback,
    public rpc::CommunitySessionRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(CommunityClientSession);

public:
    CommunityClientSession(const sne::server::ServerSideSessionConfig& sessionConfig,
        const sne::server::ServerSpec& serverSpec,
        std::unique_ptr<sne::base::SessionImpl> impl);
    virtual ~CommunityClientSession();

private:
    // = sne::server::ServerSideSession overriding
    virtual bool onConnected();
    virtual void onDisconnected();
    virtual void onThrottling(size_t readBytes, size_t maxBytesPerSecond);

private:
    // = CommunityClientSessionCallback overrding
    virtual void loginResponsed(ErrorCode errorCode, const Certificate& reissuedCertificate);
    virtual void expelledFromServer();
    virtual sne::base::Session* getCurrentSession() {
        return this;
    }
    virtual sne::server::ClientId getCurrentClientId() const {
        return getClientId();
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    // = rpc::CommunitySessionRpc overriding
    OVERRIDE_SRPC_METHOD_1(c2m_login,
        Certificate, certificate);
    OVERRIDE_SRPC_METHOD_2(c2m_onLogin,
        ErrorCode, errorCode, Certificate, reissuedCertificate);

private:
    AccountId accountId_;

    bool isExpelled_;
};

}} // namespace gideon { namespace communityserver {
