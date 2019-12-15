#pragma once

#include "../zoneserver_export.h"
#include <gideon/server/rpc/ZoneLoginRpc.h>
#include <sne/server/s2s/ClientSideProxy.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace zoneserver {

/**
 * @class ZoneLoginServerProxy
 *
 * 로그인 서버 Proxy
 */
class ZoneServer_Export ZoneLoginServerProxy :
    public sne::server::ClientSideProxy,
    public rpc::ZoneLoginRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(ZoneLoginServerProxy);

public:
    ZoneLoginServerProxy(const sne::server::ServerInfo& serverInfo,
        const sne::server::ClientSideSessionConfig& config =
            sne::server::ClientSideSessionConfig());
    virtual ~ZoneLoginServerProxy();

    void ready();

public:
    virtual bool isActivated() const {
        return sne::server::ClientSideProxy::isActivated() &&
            isRendezvoused_;
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    // = rpc::ZoneLoginRpc overriding
    OVERRIDE_SRPC_METHOD_4(z2l_rendezvous,
        ShardId, shardId, ZoneId, zoneId, AccountIds, onlineUsers, uint16_t, maxUserCount);
    OVERRIDE_SRPC_METHOD_0(z2l_onRendezvous);

    OVERRIDE_SRPC_METHOD_0(z2l_ready);

    OVERRIDE_SRPC_METHOD_1(z2l_loginZoneUser,
        Certificate, certificate);
    OVERRIDE_SRPC_METHOD_3(z2l_onLoginZoneUser,
        ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);

    OVERRIDE_SRPC_METHOD_1(z2l_reserveMigration,
        AccountId, accountId);

    OVERRIDE_SRPC_METHOD_1(z2l_logoutUser,
        AccountId, accountId);

    OVERRIDE_SRPC_METHOD_2(z2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);

    OVERRIDE_SRPC_METHOD_1(z2l_getFullUserInfo,
        AccountId, accountId);
    OVERRIDE_SRPC_METHOD_2(z2l_onGetFullUserInfo,
        ErrorCode, errorCode, FullUserInfo, userInfo);

    OVERRIDE_SRPC_METHOD_1(z2l_createCharacter,
        CreateCharacterInfo, createCharacterInfo);
    OVERRIDE_SRPC_METHOD_3(z2l_onCreateCharacter,
        ErrorCode, errorCode, AccountId, accountId, FullCharacterInfo, characterInfo);

    OVERRIDE_SRPC_METHOD_2(z2l_deleteCharacter,
        AccountId, accountId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_3(z2l_onDeleteCharacter,
        ErrorCode, errorCode, AccountId, accountId, ObjectId, characterId);

    OVERRIDE_SRPC_METHOD_2(z2l_checkDuplicateNickname,
        AccountId, accountId, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_3(z2l_onCheckDuplicateNickname,
        ErrorCode, errorCode, AccountId, accountId, Nickname, nickname);

    OVERRIDE_SRPC_METHOD_1(z2l_evZoneServerActivated,
        ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_1(z2l_evZoneServerDeactivated,
        ZoneId, zoneId);

	OVERRIDE_SRPC_METHOD_1(z2l_evAddBanInfo,
		BanInfo, banInfo);
	OVERRIDE_SRPC_METHOD_4(z2l_evRemoveBanInfo,
		BanMode, banMode, AccountId, accountId, ObjectId, characterId, std::string, ipAddress);

private:
    // = sne::server::ClientSideProxy overriding
    virtual void onAuthenticated();
    virtual void onDisconnected();

private:
    bool isRendezvoused_;
    bool isReadied_;
};

}} // namespace gideon { namespace zoneserver {
