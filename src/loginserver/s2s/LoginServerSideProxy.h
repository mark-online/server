#pragma once

#include "../loginserver_export.h"
#include <gideon/server/rpc/ZoneLoginRpc.h>
#include <gideon/server/rpc/CommunityLoginRpc.h>
#include <gideon/cs/shared/data/ServerType.h>
#include <sne/server/s2s/ManagedServerSideProxy.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace loginserver {

/**
 * @class LoginServerSideProxy
 *
 * LoginServer-side proxy
 */
class LoginServer_Export LoginServerSideProxy :
    public sne::server::ManagedServerSideProxy,
    public rpc::ZoneLoginRpc,
    public rpc::CommunityLoginRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(LoginServerSideProxy);

public:
    LoginServerSideProxy(sne::server::ServerSideBridge* bridge);
    virtual ~LoginServerSideProxy();

    void expelUser(AccountId accountId, ExpelReason reason);

public:
    void zoneServerConnected(ShardId shardId, ZoneId zoneId);
    void zoneServerDisconnected(ShardId shardId, ZoneId zoneId);

public:
    bool isZoneServerProxy() const {
        return isZoneServer(serverType_);
    }

    bool isCommunityServerProxy() const {
        return isCommunityServer(serverType_);
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

    OVERRIDE_SRPC_METHOD_2(z2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);

    OVERRIDE_SRPC_METHOD_1(z2l_evZoneServerActivated,
        ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_1(z2l_evZoneServerDeactivated,
        ZoneId, zoneId);

	OVERRIDE_SRPC_METHOD_1(z2l_evAddBanInfo,
		BanInfo, banInfo);
	OVERRIDE_SRPC_METHOD_4(z2l_evRemoveBanInfo,
		BanMode, banMode, AccountId, accountId, ObjectId, characterId, std::string, ipAddress);

public:
    // = rpc::CommunityLoginRpc overriding
    OVERRIDE_SRPC_METHOD_1(m2l_rendezvous,
        AccountIds, onlineUsers);
    OVERRIDE_SRPC_METHOD_0(m2l_onRendezvous);

    OVERRIDE_SRPC_METHOD_0(m2l_ready);

    OVERRIDE_SRPC_METHOD_1(m2l_loginCommunityUser,
        Certificate, certificate);
    OVERRIDE_SRPC_METHOD_3(m2l_onLoginCommunityUser,
        ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);

    OVERRIDE_SRPC_METHOD_1(m2l_logoutUser,
        AccountId, accountId);

    OVERRIDE_SRPC_METHOD_2(m2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);

protected: // for Test
    // = sne::server::ManagedServerSideProxy overriding
    virtual void bridged();
    virtual void unbridged();

private:
    // = sne::server::ServerSideProxy overriding
    virtual bool verifyAuthentication(
        const sne::server::S2sCertificate& certificate) const;

private:
    ServerType serverType_;
    ShardId shardId_;
    ZoneId zoneId_;
};

}} // namespace gideon { namespace loginserver {
