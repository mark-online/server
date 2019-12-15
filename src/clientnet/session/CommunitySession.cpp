#include "ClientNetPCH.h"
#include "CommunitySession.h"
#include "SessionCallback.h"
#include <gideon/clientnet/ServerProxyConfig.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/utility/SystemUtil.h>

namespace gideon { namespace clientnet {

CommunitySession::CommunitySession(const ServerProxyConfig& config,
    CommunitySessionCallback& sessionCallback,
    sne::client::ClientSessionTick& tick) :
    BaseSession(config, tick),
    sessionCallback_(sessionCallback),
    currentState_(stOffline)
{
}


CommunitySession::~CommunitySession()
{
}


CommunityLoginResult CommunitySession::login(const std::string& ipAddress, uint16_t port,
    const Certificate& certificate)
{
    if (ipAddress.empty()) {
        return CommunityLoginResult(ecClientConnectionFailed);
    }

    disconnect();

    certificate_ = certificate;
    loginResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    if (! connect(ipAddress, port)) {
        return CommunityLoginResult(ecClientConnectionFailed);
    }

    currentState_ = stLoggingIn;

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (loginResult_.isReponsed()) {
            return loginResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return CommunityLoginResult(ecClientTimeout);
}


void CommunitySession::disconnectFromServer()
{
    BaseSession::disconnectFromServer();

    currentState_ = stOffline;
}

// = rnf::ClientSessionCallback overriding

void CommunitySession::onValidated()
{
    BaseSession::onValidated();

    if (currentState_ == stLoggingIn) {
        c2m_login(certificate_);
    }
}


void CommunitySession::onDisconnected()
{
    BaseSession::onDisconnected();

    if (currentState_ == stLoggingIn) {
        loginResult_.errorCode_ = ecClientTimeout;
    }

    currentState_ = stOffline;

    sessionCallback_.communityServerDisconnected();
}

// = rpc::CommunitySessionRpc overriding

FORWARD_SRPC_METHOD_1(CommunitySession, c2m_login,
    Certificate, certificate);


RECEIVE_SRPC_METHOD_2(CommunitySession, c2m_onLogin,
    ErrorCode, errorCode, Certificate, reissuedCertificate)
{
    loginResult_.errorCode_ = errorCode;
    loginResult_.reissuedCertificate_ = reissuedCertificate;
    loginResult_.isResponsed_ = true;

    if (isFailed(errorCode)) {
        currentState_ = stOffline;
    }
    else {
        currentState_ = stLoggedIn;
    }
}

}} // namespace gideon { namespace clientnet {
