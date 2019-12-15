#include "LoginServerPCH.h"
#include "LoginClientSession.h"
#include "../LoginService.h"
#include "../shard/ShardManager.h"
#include "../user/LoginUserManager.h"
#include "../ban/BanManager.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace loginserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(LoginClientSession);

LoginClientSession::LoginClientSession(
    const sne::server::ServerSideSessionConfig& sessionConfig,
    const sne::server::ServerSpec& serverSpec,
    std::unique_ptr<sne::base::SessionImpl> impl) :
    sne::server::ServerSideSession(sessionConfig, serverSpec, std::move(impl)),
    accountId_(invalidAccountId)
{
    sne::sgp::RpcingExtension* extension =
        getImpl().getExtension<sne::sgp::RpcingExtension>();
    extension->registerRpcForwarder(*this);
    extension->registerRpcReceiver(*this);
}


LoginClientSession::~LoginClientSession()
{
}


void LoginClientSession::authenticatePended()
{
    extendAuthenticationTimeout();
}


void LoginClientSession::loginSucceeded(const AccountInfo& accountInfo,
    const CertificateMap& certificateMap)
{
    accountId_ = accountInfo.accountId_;

    c2l_onLogin(ecOk, accountInfo, LOGIN_SERVICE->getCommunityServerInfo(), certificateMap);
}


void LoginClientSession::loginFailed(ErrorCode errorCode)
{
    c2l_onLogin(errorCode, AccountInfo(), CommunityServerInfo(), CertificateMap());
}


// = sne::server::ServerSideSession overriding

bool LoginClientSession::onConnected()
{
    accountId_ = invalidAccountId;

    if (! sne::server::ServerSideSession::onConnected()) {
        return false;
    }

    return true;
}


void LoginClientSession::onDisconnected()
{
    sne::server::ServerSideSession::onDisconnected();

    LOGIN_SERVICE->clientDisconnected(accountId_, this);

    accountId_ = invalidAccountId;    
}


void LoginClientSession::onThrottling(size_t readBytes,
    size_t maxBytesPerSecond)
{
    SNE_LOG_INFO("Client(C%" PRIu64 ") is THROTTLED(%d > %d), delaying read.",
        getClientId(), readBytes, maxBytesPerSecond);
}

// = LoginClientSessionCallback overriding

void LoginClientSession::disconnectNow()
{
    disconnectGracefully();
}


void LoginClientSession::expelled(ExpelReason reason)
{
    c2l_evExpelled(reason);

    disconnectGracefully();
}


void LoginClientSession::shardSelected(ErrorCode errorCode, const FullUserInfo& userInfo)
{
    c2l_onSelectShard(errorCode, userInfo);
}


void LoginClientSession::characterCreated(ErrorCode errorCode,
    const FullCharacterInfo& characterInfo)
{    
    c2l_onCreateCharacter(errorCode, characterInfo);
}


void LoginClientSession::characterDeleted(ErrorCode errorCode, ObjectId characterId)
{
    c2l_onDeleteCharacter(errorCode, characterId);
}


void LoginClientSession::nicknameReserved(ErrorCode errorCode, const Nickname& nickname)
{
    if (isSucceeded(errorCode)) {
        reserveNickname_ = nickname;
    }
    c2l_onReserveNickname(errorCode, nickname);
}

// = sne::srpc::RpcForwarder overriding

void LoginClientSession::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("LoginClientSession::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void LoginClientSession::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("LoginClientSession::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::ZoneLoginRpc overriding

RECEIVE_SRPC_METHOD_2(LoginClientSession, c2l_login,
    UserId, userId, UserPassword, userPassword)
{
    sne::server::Profiler profiler(__FUNCTION__);

    AccountInfo accountInfo;
    CertificateMap certificateMap;
    ErrorCode errorCode =
        LOGIN_SERVICE->login(accountInfo, certificateMap, userId, userPassword, this);

    if (isSucceeded(errorCode)) {
        if (BAN_MANAGER->isBanAccount(accountInfo.accountId_)) {
            loginFailed(ecLoginBanUser);
            disconnectGracefully();
            return;
        }
    }

    if (isSucceeded(errorCode)) {
        authenticated();
        loginSucceeded(accountInfo, certificateMap);
    }
    else {
        loginFailed(errorCode);
        disconnectGracefully();
    }
}


RECEIVE_SRPC_METHOD_1(LoginClientSession, c2l_relogin,
    Certificate, certificate)
{
    sne::server::Profiler profiler(__FUNCTION__);

    ErrorCode errorCode = ecOk;
    Certificate reinssuedCertificate;
    if (isValidAccountId(accountId_)) {
        errorCode = LOGIN_SERVICE->authenticate(reinssuedCertificate, certificate);
        if (isSucceeded(errorCode)) {
            authenticated();
        }
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    c2l_onRelogin(errorCode, reinssuedCertificate);

    if (isFailed(errorCode)) {
        disconnectGracefully();
    }
}


RECEIVE_SRPC_METHOD_0(LoginClientSession, c2l_queryShardInfo)
{
    ErrorCode errorCode = ecOk;
    ShardInfoList shardInfos;
    ZoneMapCodeMap zoneMapCodeMap;
    CharacterCountPerShardMap characterCounts;

    if (isValidAccountId(accountId_)) {
        shardInfos = SHARD_MANAGER->getShardInfoList();
        zoneMapCodeMap = SHARD_MANAGER->getZoneMapCodeMap();
        characterCounts = LOGINUSER_MANAGER->getCharacterCounts(accountId_);
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    c2l_onQueryShardInfo(errorCode, shardInfos, zoneMapCodeMap, characterCounts);
}


RECEIVE_SRPC_METHOD_1(LoginClientSession, c2l_selectShard,
    ShardId, shardId)
{
    ErrorCode errorCode = ecOk;

    if (isValidAccountId(accountId_)) {
        errorCode = SHARD_MANAGER->selectShard(shardId, accountId_, *this);
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    if (isFailed(errorCode)) {
        c2l_onSelectShard(errorCode, FullUserInfo());
    }
}


RECEIVE_SRPC_METHOD_1(LoginClientSession, c2l_reserveNickname,
    Nickname, nickname)
{
    ErrorCode errorCode = ecOk;    
    reserveNickname_.clear();
    if (isValidAccountId(accountId_)) {
        LOGINUSER_MANAGER->deleteReservedNickname(accountId_);
        
        errorCode = LOGINUSER_MANAGER->reserveNickname(accountId_, nickname);
        if (isSucceeded(errorCode )) {            
            const ShardId shardId = LOGINUSER_MANAGER->getSelectedShardId(accountId_);
            if (isValidShardId(shardId)) {
                errorCode = SHARD_MANAGER->reserveNickname(shardId, accountId_, nickname, *this);
            }
            else {
                errorCode = ecShardNotSelected;
            }
        }
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    if (isFailed(errorCode)) {
        c2l_onReserveNickname(errorCode, nickname);
    }
}


RECEIVE_SRPC_METHOD_1(LoginClientSession, c2l_createCharacter,
    CreateCharacterInfo, createCharacterInfo)
{
    ErrorCode errorCode = ecOk;
    if (createCharacterInfo.accountId_ != createCharacterInfo.accountId_) {
        errorCode = ecCharacterInvalidAccountId;
    }

    if (isSucceeded(errorCode)) {
        if (reserveNickname_.empty()) {
            errorCode = ecCharacterCheckDuplicateNickname;
        }
        else if (isValidAccountId(accountId_)) {
            const ShardId shardId = LOGINUSER_MANAGER->getSelectedShardId(accountId_);
            if (isValidShardId(shardId)) {
                errorCode = SHARD_MANAGER->createCharacter(shardId, createCharacterInfo, *this);
            }
            else {
                errorCode = ecShardNotSelected;
            }
        }
        else {
            errorCode = ecLoginIsNotLoginUser;
        }
    }

    if (isFailed(errorCode)) {
        LOGINUSER_MANAGER->deleteReservedNickname(accountId_);        
        c2l_onCreateCharacter(errorCode, FullCharacterInfo());
    }
}


RECEIVE_SRPC_METHOD_1(LoginClientSession, c2l_deleteCharacter,
    ObjectId, characterId)
{
    ErrorCode errorCode = ecOk;

    if (isValidAccountId(accountId_)) {
        const ShardId shardId = LOGINUSER_MANAGER->getSelectedShardId(accountId_);
        if (isValidShardId(shardId)) {
            errorCode =
                SHARD_MANAGER->deleteCharacter(shardId, accountId_, characterId, *this);
        }
        else {
            errorCode = ecShardNotSelected;
        }
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    if (isFailed(errorCode)) {
        c2l_onDeleteCharacter(errorCode, characterId);
    }
}


RECEIVE_SRPC_METHOD_1(LoginClientSession, c2l_enterShard,
    ObjectId, characterId)
{
    ErrorCode errorCode = ecOk;
    ZoneServerInfo zoneServerInfo;

    if (isValidAccountId(accountId_)) {
        ShardId shardId = invalidShardId;
        ZoneId zoneId = invalidZoneId;
        if (LOGINUSER_MANAGER->queryZoneServerInfo(shardId, zoneId, accountId_, characterId)) {
            zoneServerInfo = SHARD_MANAGER->getZoneServerInfo(shardId, zoneId);
            if ((! zoneServerInfo.isValid()) || (! zoneServerInfo.isEnabled_)) {
                errorCode = ecZoneServerIsDown;
            }
        }
        else {
            errorCode = ecShardInvalid;
        }
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    c2l_onEnterShard(errorCode, zoneServerInfo);
}


FORWARD_SRPC_METHOD_4(LoginClientSession, c2l_onLogin,
    ErrorCode, errorCode, AccountInfo, accountInfo,
    CommunityServerInfo, communityServerInfo, CertificateMap, certificateMap);


FORWARD_SRPC_METHOD_2(LoginClientSession, c2l_onRelogin,
    ErrorCode, errorCode, Certificate, reissuedCertificate);


FORWARD_SRPC_METHOD_4(LoginClientSession, c2l_onQueryShardInfo,
    ErrorCode, errorCode, ShardInfoList, shardInfos, ZoneMapCodeMap, zoneMapCodeMap,
    CharacterCountPerShardMap, characterCounts);


FORWARD_SRPC_METHOD_2(LoginClientSession, c2l_onSelectShard,
    ErrorCode, errorCode, FullUserInfo, userInfo);


FORWARD_SRPC_METHOD_2(LoginClientSession, c2l_onReserveNickname,
    ErrorCode, errorCode, Nickname, nickname);


FORWARD_SRPC_METHOD_2(LoginClientSession, c2l_onCreateCharacter,
    ErrorCode, errorCode, FullCharacterInfo, characterInfo);


FORWARD_SRPC_METHOD_2(LoginClientSession, c2l_onDeleteCharacter,
    ErrorCode, errorCode, ObjectId, characterId);


FORWARD_SRPC_METHOD_2(LoginClientSession, c2l_onEnterShard,
    ErrorCode, errorCode, ZoneServerInfo, zoneServerInfo);


FORWARD_SRPC_METHOD_1(LoginClientSession, c2l_evExpelled,
    ExpelReason, reason);

}} // namespace gideon { namespace loginserver {
