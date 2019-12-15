#pragma once

#include "BaseSession.h"
#include <gideon/clientnet/ServerProxyCallback.h>
#include <gideon/cs/shared/rpc/detail/LoginRpc.h>

namespace gideon { namespace clientnet {

class ZoneSessionCallback;


/**
 * @struct ZoneLoginResult
 * 존 서버 로그인 요청에 대한 결과
 */
struct ZoneLoginResult
{
    ErrorCode errorCode_;
    bool isResponsed_;
    Certificate reissuedCertificate_;

    explicit ZoneLoginResult(const ErrorCode errorCode = ecOk) :
        errorCode_(errorCode),
        isResponsed_(false) {}

    void reset() {
        errorCode_ = ecOk;
        isResponsed_ = false;
        reissuedCertificate_.reset();
    }

    bool isReponsed() const {
        return isFailed(errorCode_) || isResponsed_;
    }
};


/**
 * @class ZoneSession
 */
class ZoneSession :
    public BaseSession,
    public rpc::ZoneSessionRpc
{
    enum State {
        stOffline,
        stLoggingIn,
        stLoggedIn
    };
public:
    ZoneSession(const ServerProxyConfig& config,
        ZoneSessionCallback& sessionCallback,
        sne::client::ClientSessionTick& tick);
    virtual ~ZoneSession();

    ZoneLoginResult login(const std::string& ipAddress, uint16_t port,
        const Certificate& certificate, RegionCode spawnRegionCode, ObjectId arenaId,
        MapCode preGlobalMapCode, const Position& position);

    void reserveLogout();

    void cancelLogout();

public:
    bool isLoggingIn() const {
        return currentState_ == stLoggingIn;
    }

private:
    virtual void disconnectFromServer();
    virtual bool isLoggedInServer() const {
        return currentState_ == stLoggedIn;
    }

private:
    // = rnf::ClientSessionCallback overriding
    virtual void onValidated();
    virtual void onDisconnected();

public:
    // = rpc::ZoneSessionRpc overriding
    OVERRIDE_SRPC_METHOD_5(c2z_login,
        Certificate, certificate, RegionCode, spawnRegionCode, ObjectId, arenaId,
        MapCode, preMapCode, Position, position);
    OVERRIDE_SRPC_METHOD_0(c2z_reserveLogout);
    OVERRIDE_SRPC_METHOD_0(c2z_cancelLogout);
    OVERRIDE_SRPC_METHOD_2(c2z_onLogin,
        ErrorCode, errorCode, Certificate, reissuedCertificate);

private:
    ZoneSessionCallback& sessionCallback_;

    Certificate certificate_;
    RegionCode spawnRegionCode_;
	ObjectId arenaId_;
    MapCode preGlobalMapCode_;
    ZoneLoginResult loginResult_;
    Position position_;

    State currentState_;
};

}} // namespace gideon { namespace clientnet {
