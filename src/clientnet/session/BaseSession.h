#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/UserId.h>
#include <sne/client/session/ClientSessionCallback.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace client {
class ClientSessionTick;
}} // namespace sne { namespace client {

namespace gideon { namespace clientnet {

struct ServerProxyConfig;

namespace {
class PollingClientSession;
} // namespace {

/**
 * @class BaseSession
 */
class BaseSession :
    protected sne::srpc::RpcForwarder,
    protected sne::srpc::RpcReceiver,

    public sne::client::ClientSessionCallback
{
    DECLARE_SRPC_EVENT_DISPATCHER(BaseSession);

public:
    BaseSession(const ServerProxyConfig& config,
        sne::client::ClientSessionTick& tick);
    virtual ~BaseSession();

    void initialize() {}
    void finalize();

    void registerRpcForwarder(sne::srpc::RpcForwarder& forwarder);
    void registerRpcReceiver(sne::srpc::RpcReceiver& receiver);

    void unregisterRpcForwarder(sne::srpc::RpcForwarder& forwarder);
    void unregisterRpcReceiver(sne::srpc::RpcReceiver& receiver);

    void disconnect();

    void tick();

    void handleMessages();

public:
    bool isConnected() const;

    bool isLoggedIn() const;

protected:
    /// 서버에 연결한다. 이미 연결되어 있으면, 기존 연결을 해제한다.
    bool connect(const std::string& ipAddress, uint16_t port);

protected:
    const ServerProxyConfig& getConfig() const {
        return config_;
    }

    const std::string& getIpAddress() const {
        return ipAddress_;
    }

    uint16_t getPort() const {
        return port_;
    }

protected:
    virtual void disconnectFromServer();

    virtual bool isLoggedInServer() const {
        return false;
    }

protected: // = sne::client::ClientSessionCallback overriding
    virtual void onValidated();
    virtual void onSendError();
    virtual void onReceiveError();
    virtual void onDisconnected();

private: // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId) override;

private: // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId) override;

private:
    const ServerProxyConfig& config_;
    std::unique_ptr<PollingClientSession> session_;
    bool isValidated_;

    std::string ipAddress_;
    uint16_t port_;
};

}} // namespace gideon { namespace clientnet {
