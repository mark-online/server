#include "LoginServerPCH.h"
#include "LoginServerSideProxy.h"
#include "../LoginService.h"
#include "../user/LoginUserManager.h"
#include "../shard/ShardManager.h"
#include "../ban/BanManager.h"
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace loginserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(LoginServerSideProxy);

LoginServerSideProxy::LoginServerSideProxy(
    sne::server::ServerSideBridge* bridge) :
    sne::server::ManagedServerSideProxy(bridge),
    serverType_(stUnknown),
    shardId_(invalidShardId),
    zoneId_(invalidZoneId)
{
    if (hasSession()) {
        assert(getSession().hasImpl());
        sne::sgp::RpcingExtension* extension =
            getSession().getImpl().getExtension<sne::sgp::RpcingExtension>();
        extension->registerRpcForwarder(*this);
        extension->registerRpcReceiver(*this);
    }
}


LoginServerSideProxy::~LoginServerSideProxy()
{
}


void LoginServerSideProxy::expelUser(AccountId accountId, ExpelReason reason)
{
    if (isZoneServerProxy()) {
        z2l_evUserExpelled(accountId, reason);
    }
    else if (isCommunityServerProxy()) {
        m2l_evUserExpelled(accountId, reason);
    }
}


void LoginServerSideProxy::zoneServerConnected(ShardId shardId, ZoneId zoneId)
{
    if (isZoneServerProxy()) {
        if (shardId != shardId_) {
            return;
        }
        if (zoneId == zoneId_) {
            return;
        }
        z2l_evZoneServerActivated(zoneId);
    }
}


void LoginServerSideProxy::zoneServerDisconnected(ShardId shardId, ZoneId zoneId)
{
    if (isZoneServerProxy()) {
        if (shardId != shardId_) {
            return;
        }
        if (zoneId == zoneId_) {
            return;
        }
        z2l_evZoneServerDeactivated(zoneId);
    }
}

// = sne::server::ManagedServerSideProxy overriding

void LoginServerSideProxy::bridged()
{
    sne::server::ManagedServerSideProxy::bridged();

    SNE_LOG_WARNING("LoginServerSideProxy::bridged(S%u).",
        getServerId());
}


void LoginServerSideProxy::unbridged()
{
    SNE_LOG_WARNING("LoginServerSideProxy::unbridged(S%u,%d).",
        getServerId(), serverType_);

    if (! sne::server::isValid(getServerId())) {
        return;
    }

    if (isZoneServerProxy()) {
        LOGINUSER_MANAGER->zoneServerDisconnected(getServerId());
        SHARD_MANAGER->zoneServerDisconnected(shardId_, zoneId_, getServerId());

        SNE_LOG_INFO("ZoneServer(S%u) disconnected", getServerId());
    }
    else if (isCommunityServerProxy()) {
        LOGINUSER_MANAGER->communityServerDisconnected();
    
        SNE_LOG_INFO("CommunityServer disconnected");
    }

    sne::server::ManagedServerSideProxy::unbridged();

    serverType_ = stUnknown;
    shardId_ = invalidShardId;
    zoneId_ = invalidZoneId;
}

// = sne::server::ServerSideProxy overriding

bool LoginServerSideProxy::verifyAuthentication(
    const sne::server::S2sCertificate& certificate) const
{
    return isSucceeded(LOGIN_SERVICE->authenticate(certificate));
}

// = sne::srpc::RpcForwarder overriding

void LoginServerSideProxy::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("LoginServerSideProxy::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void LoginServerSideProxy::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("LoginServerSideProxy::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::ZoneLoginRpc overriding

RECEIVE_SRPC_METHOD_4(LoginServerSideProxy, z2l_rendezvous,
    ShardId, shardId, ZoneId, zoneId, AccountIds, onlineUsers, uint16_t, maxUserCount)
{
    serverType_ = stZoneServer;
    shardId_ = shardId;
    zoneId_ = zoneId;

    SHARD_MANAGER->zoneServerConnected(getServerId(), shardId_, zoneId_,
        uint16_t(onlineUsers.size()), maxUserCount);
    LOGINUSER_MANAGER->zoneServerConnected(getServerId(), shardId_, onlineUsers);

    z2l_onRendezvous();

    SNE_LOG_INFO("ZoneServer(S%u,H%u,Z%u) connected",
        getServerId(), shardId_, zoneId_);
}


FORWARD_SRPC_METHOD_0(LoginServerSideProxy, z2l_onRendezvous);


RECEIVE_SRPC_METHOD_0(LoginServerSideProxy, z2l_ready)
{
    SHARD_MANAGER->zoneServerReadied(shardId_, zoneId_);

    SNE_LOG_INFO("ZoneServer(S%u,H%u,Z%u) readied.",
        getServerId(), shardId_, zoneId_);
}


RECEIVE_SRPC_METHOD_1(LoginServerSideProxy, z2l_loginZoneUser,
    Certificate, certificate)
{
	AccountInfo accountInfo;
    Certificate reissuedCertificate;
    const ErrorCode errorCode =
        LOGINUSER_MANAGER->loginZoneUser(accountInfo, reissuedCertificate,
            getServerId(), shardId_, certificate);

    if (isSucceeded(errorCode)) {
        SHARD_MANAGER->userLoggedIn(shardId_, zoneId_);
    }

     z2l_onLoginZoneUser(errorCode, accountInfo, reissuedCertificate);
}


FORWARD_SRPC_METHOD_3(LoginServerSideProxy, z2l_onLoginZoneUser,
    ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);


RECEIVE_SRPC_METHOD_1(LoginServerSideProxy, z2l_reserveMigration,
    AccountId, accountId)
{
    LOGINUSER_MANAGER->reserveMigration(accountId);
}


RECEIVE_SRPC_METHOD_1(LoginServerSideProxy, z2l_logoutUser,
    AccountId, accountId)
{
    if (! LOGINUSER_MANAGER->logout(accountId, stZoneServer)) {
        return;
    }

    SHARD_MANAGER->userLoggedOut(shardId_, zoneId_);
}


FORWARD_SRPC_METHOD_1(LoginServerSideProxy, z2l_getFullUserInfo,
    AccountId, accountId);


RECEIVE_SRPC_METHOD_2(LoginServerSideProxy, z2l_onGetFullUserInfo,
    ErrorCode, errorCode, FullUserInfo, userInfo)
{
    SHARD_MANAGER->shardSelected(errorCode, userInfo);
}


FORWARD_SRPC_METHOD_1(LoginServerSideProxy, z2l_createCharacter,
    CreateCharacterInfo, createCharacterInfo);


RECEIVE_SRPC_METHOD_3(LoginServerSideProxy, z2l_onCreateCharacter,
    ErrorCode, errorCode, AccountId, accountId, FullCharacterInfo, characterInfo)
{
    SHARD_MANAGER->characterCreated(errorCode, accountId, characterInfo);
}


FORWARD_SRPC_METHOD_2(LoginServerSideProxy, z2l_deleteCharacter,
    AccountId, accountId, ObjectId, characterId);


RECEIVE_SRPC_METHOD_3(LoginServerSideProxy, z2l_onDeleteCharacter,
    ErrorCode, errorCode, AccountId, accountId, ObjectId, characterId)
{
    SHARD_MANAGER->characterDeleted(errorCode, accountId, characterId);
}


FORWARD_SRPC_METHOD_2(LoginServerSideProxy, z2l_checkDuplicateNickname,
    AccountId, accountId, Nickname, nickname);


RECEIVE_SRPC_METHOD_3(LoginServerSideProxy, z2l_onCheckDuplicateNickname,
    ErrorCode, errorCode, AccountId, accountId, Nickname, nickname)
{
    SHARD_MANAGER->nicknameReserved(errorCode, accountId, nickname);
}


RECEIVE_SRPC_METHOD_1(LoginServerSideProxy, z2l_evAddBanInfo,
	BanInfo, banInfo)
{
	BAN_MANAGER->addBan(banInfo);
}


RECEIVE_SRPC_METHOD_4(LoginServerSideProxy, z2l_evRemoveBanInfo,
	BanMode, banMode, AccountId, accountId, ObjectId, characterId, std::string, ipAddress)
{
	BAN_MANAGER->removeBan(banMode, accountId, characterId, ipAddress);
}


FORWARD_SRPC_METHOD_2(LoginServerSideProxy, z2l_evUserExpelled,
    AccountId, accountId, ExpelReason, expelReason);


FORWARD_SRPC_METHOD_1(LoginServerSideProxy, z2l_evZoneServerActivated,
    ZoneId, zoneId);


FORWARD_SRPC_METHOD_1(LoginServerSideProxy, z2l_evZoneServerDeactivated,
    ZoneId, zoneId);




// = rpc::CommunityLoginRpc overriding

RECEIVE_SRPC_METHOD_1(LoginServerSideProxy, m2l_rendezvous,
    AccountIds, onlineUsers)
{
    serverType_ = stCommunityServer;

    LOGINUSER_MANAGER->communityServerConnected(getServerId(), onlineUsers);

    m2l_onRendezvous();

    SNE_LOG_INFO("CommunityServer connected");
}


FORWARD_SRPC_METHOD_0(LoginServerSideProxy, m2l_onRendezvous);


RECEIVE_SRPC_METHOD_0(LoginServerSideProxy, m2l_ready)
{
    SNE_LOG_INFO("CommunityServer(S%u) readied.",
        getServerId());
}


RECEIVE_SRPC_METHOD_1(LoginServerSideProxy, m2l_loginCommunityUser,
    Certificate, certificate)
{
	AccountInfo accountInfo;
    Certificate reissuedCertificate;
    const ErrorCode errorCode =
        LOGINUSER_MANAGER->loginCommunityUser(accountInfo, reissuedCertificate,
            getServerId(), certificate);

     m2l_onLoginCommunityUser(errorCode, accountInfo, reissuedCertificate);
}


FORWARD_SRPC_METHOD_3(LoginServerSideProxy, m2l_onLoginCommunityUser,
    ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);


RECEIVE_SRPC_METHOD_1(LoginServerSideProxy, m2l_logoutUser,
    AccountId, accountId)
{
    (void)LOGINUSER_MANAGER->logout(accountId, stCommunityServer);
}


FORWARD_SRPC_METHOD_2(LoginServerSideProxy, m2l_evUserExpelled,
    AccountId, accountId, ExpelReason, expelReason);

}} // namespace gideon { namespace loginserver {
