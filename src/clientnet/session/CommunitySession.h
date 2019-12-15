#pragma once

#include "BaseSession.h"
#include <gideon/clientnet/ServerProxyCallback.h>
#include <gideon/cs/shared/rpc/detail/LoginRpc.h>

namespace gideon { namespace clientnet {

class CommunitySessionCallback;

/**
 * @struct CommunityLoginResult
 * 커뮤니티 서버 로그인 요청에 대한 결과
 */
struct CommunityLoginResult
{
    ErrorCode errorCode_;
    bool isResponsed_;
    Certificate reissuedCertificate_;

    explicit CommunityLoginResult(ErrorCode errorCode = ecOk) :
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
 * @class CommunitySession
 */
class CommunitySession :
    public BaseSession,
    public rpc::CommunitySessionRpc
{
    enum State {
        stOffline,
        stLoggingIn,
        stLoggedIn
    };
public:
    CommunitySession(const ServerProxyConfig& config,
        CommunitySessionCallback& sessionCallback,
        sne::client::ClientSessionTick& tick);
    virtual ~CommunitySession();

    CommunityLoginResult login(const std::string& ipAddress, uint16_t port,
        const Certificate& certificate);

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
    // = rpc::CommunitySessionRpc overriding
    OVERRIDE_SRPC_METHOD_1(c2m_login,
        Certificate, certificate);
    OVERRIDE_SRPC_METHOD_2(c2m_onLogin,
        ErrorCode, errorCode, Certificate, reissuedCertificate);

private:
    CommunitySessionCallback& sessionCallback_;

    Certificate certificate_;

    CommunityLoginResult loginResult_;

    State currentState_;
};

}} // namespace gideon { namespace clientnet {
