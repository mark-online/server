#include "CommunityServerPCH.h"
#include "CommunityClientSession.h"
#include "../CommunityService.h"
#include "../user/CommunityUserManager.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace communityserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(CommunityClientSession);

CommunityClientSession::CommunityClientSession(
    const sne::server::ServerSideSessionConfig& sessionConfig,
    const sne::server::ServerSpec& serverSpec,
    std::unique_ptr<sne::base::SessionImpl> impl) :
    sne::server::ServerSideSession(sessionConfig, serverSpec, std::move(impl)),
    isExpelled_(false)
{
    sne::sgp::RpcingExtension* extension =
        getImpl().getExtension<sne::sgp::RpcingExtension>();
    extension->registerRpcForwarder(*this);
    extension->registerRpcReceiver(*this);
}


CommunityClientSession::~CommunityClientSession()
{
}

// = sne::server::ServerSideSession overriding

bool CommunityClientSession::onConnected()
{
    if (! sne::server::ServerSideSession::onConnected()) {
        return false;
    }

    accountId_ = invalidAccountId;
    isExpelled_ = false;
    return true;
}


void CommunityClientSession::onDisconnected()
{
    sne::server::ServerSideSession::onDisconnected();

    if (! isExpelled_) {
        (void)COMMUNITYUSER_MANAGER->logout(accountId_);
    }
}


void CommunityClientSession::onThrottling(size_t readBytes,
    size_t maxBytesPerSecond)
{
    SNE_LOG_INFO("Client(C%" PRIu64 ") is THROTTLED(%d > %d), delaying read.",
        getClientId(), readBytes, maxBytesPerSecond);
}

// = CommunityClientSessionCallback overrding

void CommunityClientSession::loginResponsed(ErrorCode errorCode,
    const Certificate& reissuedCertificate)
{
    c2m_onLogin(errorCode, reissuedCertificate);

    if (isSucceeded(errorCode)) {
        authenticated();
    }
    else {
        disconnectGracefully();
    }
}


void CommunityClientSession::expelledFromServer()
{
    isExpelled_ = true;
    disconnectGracefully();
}

// = sne::srpc::RpcForwarder overriding

void CommunityClientSession::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityClientSession::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void CommunityClientSession::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityClientSession::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::CommunitySessionRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityClientSession, c2m_login,
    Certificate, certificate)
{
    sne::server::Profiler profiler(__FUNCTION__);

    // 로그인 서버에 사용자 로그인을 요청하고, 그에 대한 응답이 올 경우
    // 클라이언트에게 로그인 응답을 전송한다

    const ErrorCode errorCode = COMMUNITY_SERVICE->login(certificate, *this);
    if (isSucceeded(errorCode)) {
        accountId_ = certificate.accountId_;
        extendAuthenticationTimeout();
    }
    else {
        c2m_onLogin(errorCode, Certificate());
        disconnectGracefully();
    }
}


FORWARD_SRPC_METHOD_2(CommunityClientSession, c2m_onLogin,
    ErrorCode, errorCode, Certificate, reissuedCertificate);

}} // namespace gideon { namespace communityserver {
