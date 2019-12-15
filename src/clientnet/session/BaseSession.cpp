#include "ClientNetPCH.h"
#include "BaseSession.h"
#include <gideon/clientnet/ServerProxyConfig.h>
#include <sne/sgp/protocol/security/SecurePacketCoderFactory.h>
#include <sne/client/session/ClientSession.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace clientnet {

namespace {

/**
 * @class PollingClientSession
 */
class PollingClientSession : public sne::client::ClientSession
{
    struct Message {
        sne::base::MessageType messageType_;
        std::unique_ptr<sne::base::MemoryBlock> mblock_;

        Message(sne::base::MessageType messageType,
            const sne::base::MemoryBlock& mblock) :
            messageType_(messageType) {
            const size_t length = mblock.length();
            mblock_ = std::make_unique<sne::base::MemoryBlock>(length);
            mblock_->copyFrom(mblock.base(), mblock.length());
        }
    };

    typedef sne::core::Queue<Message> Messages;

public:
    PollingClientSession(sne::client::ClientSessionTick& tick,
        sne::sgp::PacketCoderFactory& packetCoderFactory,
        msec_t heartbeatInterval,
        sne::client::ClientSessionCallback* callback,
        bool usePolling) :
        sne::client::ClientSession(tick, packetCoderFactory, heartbeatInterval, callback),
        usePolling_(usePolling) {}

    virtual ~PollingClientSession() {
        reset();
    }

    void disconnectNow() {
        disconnect();
        reset();
    }

    void reset() {
        std::unique_lock<std::mutex> lock(lock_);

        while (! messages_.empty()) {
            messages_.pop();
        }
    }

    void handleMessages() {
        std::unique_lock<std::mutex> lock(lock_);

        while (! messages_.empty()) {
            Message& message = messages_.front();
            getImpl().handleMessage(message.messageType_, *message.mblock_);
            messages_.pop();
        }
    }

private:
    // = SessionCallback overriding
    virtual bool onMessageArrived(sne::base::MessageType messageType,
        sne::base::MemoryBlock& mblock) {
        if (! usePolling_) {
            return getImpl().handleMessage(messageType, mblock);
        }

        {
            std::unique_lock<std::mutex> lock(lock_);
            messages_.emplace(messageType, mblock);
        }
        return true;
    }

private:
    bool usePolling_;
    Messages messages_;

    std::mutex lock_;
};

} // namespace {

// = BaseSession

IMPLEMENT_SRPC_EVENT_DISPATCHER(BaseSession);

#pragma warning(push)
#pragma warning(disable : 4355 4748)

BaseSession::BaseSession(const ServerProxyConfig& config,
    sne::client::ClientSessionTick& tick) :
    config_(config),
    isValidated_(false)
{
    sne::sgp::SecurePacketCoderFactory packetCoderFactory(
        config_.packetCipherAlorightm_);

    session_ = std::make_unique<PollingClientSession>(tick, packetCoderFactory,
        config_.heartbeatInterval_, this, config.usePolling_);

    session_->registerRpcForwarder(*this);
    session_->registerRpcReceiver(*this);
}

#pragma warning(pop)

BaseSession::~BaseSession()
{
}


void BaseSession::finalize()
{
    if (session_.get() != nullptr) {
        session_->finalize();
    }
}


void BaseSession::registerRpcForwarder(sne::srpc::RpcForwarder& forwarder)
{
    session_->registerRpcForwarder(forwarder);
}


void BaseSession::registerRpcReceiver(sne::srpc::RpcReceiver& receiver)
{
    session_->registerRpcReceiver(receiver);
}


void BaseSession::unregisterRpcForwarder(sne::srpc::RpcForwarder& forwarder)
{
    session_->unregisterRpcForwarder(forwarder);
}


void BaseSession::unregisterRpcReceiver(sne::srpc::RpcReceiver& receiver)
{
    session_->unregisterRpcReceiver(receiver);
}


bool BaseSession::connect(const std::string& ipAddress, uint16_t port)
{
    session_->disconnectNow();

    ipAddress_ = ipAddress;
    port_ = port;

    isValidated_ = false;

    const int timeout = config_.connectionTimeout_;
    if (! session_->connect(ipAddress, port, timeout)) {
        SNE_LOG_ERROR("Failed to connect (%s:%d)",
            ipAddress_.c_str(), port_);
        return false;
    }

    SNE_LOG_INFO("Connected to (%s:%d)",
        ipAddress_.c_str(), port_);
    return true;
}


void BaseSession::disconnect()
{
    disconnectFromServer();
}


void BaseSession::tick()
{
    if (! config_.usePolling_) {
        session_->tick();
    }
    else {
        assert(false);
    }
}


void BaseSession::handleMessages()
{
    if (config_.usePolling_) {
        session_->handleMessages();
    }
    else {
        session_->tick();
    }
}


bool BaseSession::isConnected() const
{
    return session_->isConnected() && isValidated_;
}


bool BaseSession::isLoggedIn() const
{
    return isConnected() && isLoggedInServer();
}


void BaseSession::disconnectFromServer()
{
    isValidated_ = false;

    session_->disconnectGracefully();
}

// = sne::client::ClientSessionCallback overriding

void BaseSession::onValidated()
{
    isValidated_ = true;

    SNE_LOG_DEBUG(__FUNCTION__);
}


void BaseSession::onSendError()
{
    SNE_LOG_DEBUG(__FUNCTION__);
}


void BaseSession::onReceiveError()
{
    SNE_LOG_DEBUG(__FUNCTION__);
}


void BaseSession::onDisconnected()
{
    isValidated_ = false;
    session_->reset();

    SNE_LOG_INFO("Disconnected from (%s:%d)",
        ipAddress_.c_str(), port_);
}

// = sne::srpc::RpcForwarder overriding

void BaseSession::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    assert(isValidated_);

    SNE_LOG_DEBUG(__FUNCTION__ "(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void BaseSession::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG(__FUNCTION__ "(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}} // namespace gideon { namespace clientnet {

