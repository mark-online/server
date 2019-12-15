#pragma once

#include "ZoneClientSessionCallback.h"
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/rpc/detail/LoginRpc.h>
#include <sne/server/session/ServerSideSession.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver {

/**
 * @class ZoneClientSession
 *
 * Server-side client session
 */
class ZoneClientSession :
    public sne::server::ServerSideSession,
    public ZoneClientSessionCallback,
    public rpc::ZoneSessionRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(ZoneClientSession);

public:
    ZoneClientSession(const sne::server::ServerSideSessionConfig& sessionConfig,
        const sne::server::ServerSpec& serverSpec,
        std::unique_ptr<sne::base::SessionImpl> impl);
    virtual ~ZoneClientSession();

public:
    void logout(bool shouldDisconnect);

private:
    /// 로그아웃을 예약한다
    void reserveLogout(bool shouldDisconnect);

    /// 예약된 로그아웃을 취소한다
    void cancelLogout();

private:
    // = sne::server::ServerSideSession overriding
    virtual bool onConnected();
    virtual void onDisconnected();
    virtual void onThrottling(size_t readBytes, size_t maxBytesPerSecond);

private:
    // = ZoneClientSessionCallback overrding
    virtual void loginResponsed(ErrorCode errorCode, const Certificate& reissuedCertificate);
    virtual void expelledFromServer();
    virtual void reserveMigration();
    virtual sne::base::Session* getCurrentSession() {
        return this;
    }
    virtual sne::server::ClientId getCurrentClientId() const {
        return getClientId();
    }
    virtual bool isLogoutReserved() const {
        return isLogoutReserved_;
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    // = rpc::ZoneSessionRpc overriding
    OVERRIDE_SRPC_METHOD_5(c2z_login,
        Certificate, certificate, RegionCode, spawnRegionCode, ObjectId, arenaId,
        MapCode, preGlobalMapCode, Position, position);
    OVERRIDE_SRPC_METHOD_0(c2z_reserveLogout);
    OVERRIDE_SRPC_METHOD_0(c2z_cancelLogout);
    OVERRIDE_SRPC_METHOD_2(c2z_onLogin,
        ErrorCode, errorCode, Certificate, reissuedCertificate);

private:
    AccountId accountId_;

    bool isLoggedIn_;
    bool isExpelled_;
    bool isLogoutReserved_;
    bool isMigrationReserved_;

    sne::base::Future::WeakRef logoutTask_;
};

}} // namespace gideon { namespace zoneserver {
