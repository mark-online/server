#pragma once

#include "LoginClientSessionCallback.h"
#include <gideon/cs/shared/rpc/detail/LoginRpc.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <sne/server/session/ServerSideSession.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace loginserver {

/**
 * @class LoginClientSession
 *
 * Login server-side client session
 */
class LoginClientSession :
    public sne::server::ServerSideSession,
    public rpc::LoginSessionRpc,
    private LoginClientSessionCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(LoginClientSession);

public:
    LoginClientSession(const sne::server::ServerSideSessionConfig& sessionConfig,
        const sne::server::ServerSpec& serverSpec,
        std::unique_ptr<sne::base::SessionImpl> impl);
    virtual ~LoginClientSession();

private:
    // = sne::server::ServerSideSession overriding
    bool onConnected() override;
    void onDisconnected() override;
    void onThrottling(size_t readBytes, size_t maxBytesPerSecond) override;

private:
    void authenticatePended();
    void loginSucceeded(const AccountInfo& accountInfo, const CertificateMap& certificateMap);
    void loginFailed(ErrorCode errorCode);

private:
    bool isLoggedIn() const {
        return isValidAccountId(accountId_);
    }

private:
    // = LoginClientSessionCallback overriding
    void disconnectNow() override;
    void expelled(ExpelReason reason) override;
    void shardSelected(ErrorCode errorCode, const FullUserInfo& userInfo) override;
    void characterCreated(ErrorCode errorCode,
        const FullCharacterInfo& characterInfo) override;
    void characterDeleted(ErrorCode errorCode, ObjectId characterId) override;
    void nicknameReserved(ErrorCode errorCode, const Nickname& nickname) override;

private:
    // = sne::srpc::RpcForwarder overriding
    void onForwarding(const sne::srpc::RRpcId& rpcId) override;

    // = sne::srpc::RpcReceiver overriding
    void onReceiving(const sne::srpc::RRpcId& rpcId) override;

public:
    // = rpc::LoginSessionRpc overriding
    OVERRIDE_SRPC_METHOD_2(c2l_login,
        UserId, userId, UserPassword, userPassword);
    OVERRIDE_SRPC_METHOD_1(c2l_relogin,
        Certificate, certificate);
    OVERRIDE_SRPC_METHOD_0(c2l_queryShardInfo);
    OVERRIDE_SRPC_METHOD_1(c2l_selectShard,
        ShardId, shardId);
    OVERRIDE_SRPC_METHOD_1(c2l_reserveNickname,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_1(c2l_createCharacter,
        CreateCharacterInfo, createCharacterInfo);
    OVERRIDE_SRPC_METHOD_1(c2l_deleteCharacter,
        ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_1(c2l_enterShard,
        ObjectId, characterId);

    OVERRIDE_SRPC_METHOD_4(c2l_onLogin,
        ErrorCode, errorCode, AccountInfo, accountInfo,
        CommunityServerInfo, communityServerInfo, CertificateMap, certificateMap);
    OVERRIDE_SRPC_METHOD_2(c2l_onRelogin,
        ErrorCode, errorCode, Certificate, reissuedCertificate);
    OVERRIDE_SRPC_METHOD_4(c2l_onQueryShardInfo,
        ErrorCode, errorCode, ShardInfoList, shardInfos, ZoneMapCodeMap, zoneMapCodeMap,
        CharacterCountPerShardMap, characterCounts);
    OVERRIDE_SRPC_METHOD_2(c2l_onSelectShard,
        ErrorCode, errorCode, FullUserInfo, userInfo);
    OVERRIDE_SRPC_METHOD_2(c2l_onReserveNickname,
        ErrorCode, errorCode, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(c2l_onCreateCharacter,
        ErrorCode, errorCode, FullCharacterInfo, characterInfo);
    OVERRIDE_SRPC_METHOD_2(c2l_onDeleteCharacter,
        ErrorCode, errorCode, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_2(c2l_onEnterShard,
        ErrorCode, errorCode, ZoneServerInfo, zoneServerInfo);
    OVERRIDE_SRPC_METHOD_1(c2l_evExpelled,
        ExpelReason, reason);

private:
	AccountId accountId_;
    Nickname reserveNickname_;
};

}} // namespace gideon { namespace loginserver {
