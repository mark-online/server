#include "ClientNetPCH.h"
#include "ZoneSession.h"
#include "SessionCallback.h"
#include <gideon/clientnet/ServerProxyConfig.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/utility/SystemUtil.h>

namespace gideon { namespace clientnet {

ZoneSession::ZoneSession(const ServerProxyConfig& config,
    ZoneSessionCallback& sessionCallback,
    sne::client::ClientSessionTick& tick) :
    BaseSession(config, tick),
    sessionCallback_(sessionCallback),
    spawnRegionCode_(invalidRegionCode),
	arenaId_(invalidObjectId),
    preGlobalMapCode_(invalidMapCode),
    currentState_(stOffline)
{
}


ZoneSession::~ZoneSession()
{
}


ZoneLoginResult ZoneSession::login(const std::string& ipAddress, uint16_t port,
    const Certificate& certificate, RegionCode spawnRegionCode, ObjectId arenaId,
    MapCode preGlobalMapCode, const Position& position)
{
    if (ipAddress.empty()) {
        return ZoneLoginResult(ecClientConnectionFailed);
    }

    disconnect();

    certificate_ = certificate;
    spawnRegionCode_ = spawnRegionCode;
	arenaId_ = arenaId;
    preGlobalMapCode_ = preGlobalMapCode;
    position_ = position;
    loginResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    if (! connect(ipAddress, port)) {
        return ZoneLoginResult(ecClientConnectionFailed);
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
    return ZoneLoginResult(ecClientTimeout);
}


void ZoneSession::reserveLogout()
{
    c2z_reserveLogout();
}


void ZoneSession::cancelLogout()
{
    c2z_cancelLogout();
}


void ZoneSession::disconnectFromServer()
{
    BaseSession::disconnectFromServer();

    currentState_ = stOffline;
}

// = rnf::ClientSessionCallback overriding

void ZoneSession::onValidated()
{
    BaseSession::onValidated();

    if (currentState_ == stLoggingIn) {
        c2z_login(certificate_, spawnRegionCode_, arenaId_, preGlobalMapCode_, position_);
    }
}


void ZoneSession::onDisconnected()
{
    BaseSession::onDisconnected();

    if (currentState_ == stLoggingIn) {
        loginResult_.errorCode_ = ecClientTimeout;
    }

    currentState_ = stOffline;

    sessionCallback_.zoneServerDisconnected();
}

// = rpc::ZoneSessionRpc overriding

FORWARD_SRPC_METHOD_5(ZoneSession, c2z_login,
    Certificate, certificate, RegionCode, spawnRegionCode, ObjectId, arenaId, 
    MapCode, preGlobalMapCode, Position, position);


FORWARD_SRPC_METHOD_0(ZoneSession, c2z_reserveLogout);


FORWARD_SRPC_METHOD_0(ZoneSession, c2z_cancelLogout);


RECEIVE_SRPC_METHOD_2(ZoneSession, c2z_onLogin,
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
