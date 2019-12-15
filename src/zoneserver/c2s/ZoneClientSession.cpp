#include "ZoneServerPCH.h"
#include "ZoneClientSession.h"
#include "../ZoneService.h"
#include "../user/ZoneUserManager.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class LogoutTask
 */
class LogoutTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<LogoutTask>
{
public:
    LogoutTask(ZoneClientSession& session, bool shouldDisconnect) :
        session_(session),
        shouldDisconnect_(shouldDisconnect) {}

private:
    virtual void run() {
        session_.logout(shouldDisconnect_);
    }

private:
    ZoneClientSession& session_;
    const bool shouldDisconnect_;
};

} // namespace

// = ZoneClientSession

IMPLEMENT_SRPC_EVENT_DISPATCHER(ZoneClientSession);

ZoneClientSession::ZoneClientSession(
    const sne::server::ServerSideSessionConfig& sessionConfig,
    const sne::server::ServerSpec& serverSpec,
    std::unique_ptr<sne::base::SessionImpl> impl) :
    sne::server::ServerSideSession(sessionConfig, serverSpec, std::move(impl)),
    isLoggedIn_(false),
    isExpelled_(false),
    isLogoutReserved_(false),
    isMigrationReserved_(false)
{
    sne::sgp::RpcingExtension* extension =
        getImpl().getExtension<sne::sgp::RpcingExtension>();
    extension->registerRpcForwarder(*this);
    extension->registerRpcReceiver(*this);
}


ZoneClientSession::~ZoneClientSession()
{
}


void ZoneClientSession::logout(bool shouldDisconnect)
{
    if (isLoggedIn_ && (! isExpelled_)) {
        (void)ZONEUSER_MANAGER->logout(accountId_);
    }

    if (shouldDisconnect) {
        disconnectGracefully();
    }
}


void ZoneClientSession::reserveLogout(bool shouldDisconnect)
{
    if (isLogoutReserved_) {
        return;
    }
    isLogoutReserved_ = true;

    sne::base::Future::Ref logoutTask = logoutTask_.lock();
    if (logoutTask.get() != nullptr) {
        assert(false && "어떤 경우?");
        return;
    }

    // TODO: property table로 옮길 것
    const msec_t logoutReserveTime = 15 * 1000;
    logoutTask_ = TASK_SCHEDULER->schedule(
        std::make_unique<LogoutTask>(*this, shouldDisconnect), logoutReserveTime);
}


void ZoneClientSession::cancelLogout()
{
    if (! isLogoutReserved_) {
        return;
    }
    isLogoutReserved_ = false;

    sne::base::Future::Ref logoutTask = logoutTask_.lock();
    if (logoutTask.get() != nullptr) {
        logoutTask->cancel();
        logoutTask_.reset();
    }
}

// = sne::server::ServerSideSession overriding

bool ZoneClientSession::onConnected()
{
    accountId_ = invalidAccountId;
    isLoggedIn_ = false;
    isExpelled_ = false;
    isLogoutReserved_ = false;

    return sne::server::ServerSideSession::onConnected();
}


void ZoneClientSession::onDisconnected()
{
    sne::server::ServerSideSession::onDisconnected();

    if (isExpelled_) {
        return;
    }

    if (isMigrationReserved_) {
        logout(true);
    }
    else if (! isLogoutReserved_) {
        reserveLogout(false);
    }
}


void ZoneClientSession::onThrottling(size_t readBytes,
    size_t maxBytesPerSecond)
{
    SNE_LOG_INFO("Client(C%" PRIu64 ") is THROTTLED(%d > %d), delaying read.",
        getClientId(), readBytes, maxBytesPerSecond);
}

// = ZoneClientSessionCallback overrding

void ZoneClientSession::loginResponsed(ErrorCode errorCode,
    const Certificate& reissuedCertificate)
{
    c2z_onLogin(errorCode, reissuedCertificate);

    if (isSucceeded(errorCode)) {
        isLoggedIn_ = true;
        authenticated();
    }
    else {
        disconnectGracefully();
    }
}


void ZoneClientSession::expelledFromServer()
{
    isExpelled_ = true;

    if (! isLogoutReserved_) {
        disconnectGracefully();
    }
}


void ZoneClientSession::reserveMigration()
{
    isMigrationReserved_ = true;
}

// = sne::srpc::RpcForwarder overriding

void ZoneClientSession::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneClientSession::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void ZoneClientSession::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneClientSession::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::ZoneSessionRpc overriding

RECEIVE_SRPC_METHOD_5(ZoneClientSession, c2z_login,
    Certificate, certificate, RegionCode, spawnRegionCode, ObjectId, arenaId,
    MapCode, preGlobalMapCode, Position, position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    // 로그인 서버에 사용자 로그인을 요청하고, 그에 대한 응답이 올 경우
    // 클라이언트에게 로그인 응답을 전송한다

    const ErrorCode errorCode = ZONE_SERVICE->login(certificate, arenaId, spawnRegionCode,
        preGlobalMapCode, position, *this);
    if (isSucceeded(errorCode)) {
        accountId_ = certificate.accountId_;
        extendAuthenticationTimeout();
    }
    else {
        c2z_onLogin(errorCode, Certificate());
    }
}


RECEIVE_SRPC_METHOD_0(ZoneClientSession, c2z_reserveLogout)
{
    reserveLogout(true);
}


RECEIVE_SRPC_METHOD_0(ZoneClientSession, c2z_cancelLogout)
{
    cancelLogout();
}


FORWARD_SRPC_METHOD_2(ZoneClientSession, c2z_onLogin,
    ErrorCode, errorCode, Certificate, reissuedCertificate);

}} // namespace gideon { namespace zoneserver {
