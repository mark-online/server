#include "ClientNetPCH.h"
#include "LoginSession.h"
#include <gideon/clientnet/ServerProxyConfig.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/utility/SystemUtil.h>

namespace gideon { namespace clientnet {

LoginSession::LoginSession(const ServerProxyConfig& config,
    LoginSessionCallback& sessionCallback,
    sne::client::ClientSessionTick& tick) :
    BaseSession(config, tick),
    sessionCallback_(sessionCallback),
    requestType_(rtNone)
{
}


LoginSession::~LoginSession()
{
}


LoginResult LoginSession::login(const UserId& userId,
    const UserPassword& password)
{
    if (isRequesting()) {
        return LoginResult(ecClientAlreadyRequested);
    }

    disconnect();

    loginResult_.reset();
    userId_ = userId;
    password_ = password;

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    if (! connectToLoginServer(rtLogin)) {
        return LoginResult(ecClientConnectionFailed);
    }

    for (;;) {
        if (sne::core::getTickCount() >= deadline) {
            break;
        }

        handleMessages();

        if (loginResult_.isReponsed()) {
            return loginResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return LoginResult(ecClientTimeout);
}


ReloginResult LoginSession::relogin(const Certificate& certificate)
{
    if (isRequesting()) {
        return ReloginResult(ecClientAlreadyRequested);
    }

    reloginResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    certificate_ = certificate;

    if (! connectToLoginServer(rtRelogin)) {
        return ReloginResult(ecClientConnectionFailed);
    }

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (reloginResult_.isReponsed()) {
            return reloginResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return ReloginResult(ecClientTimeout);
}


QueryShardInfoResult LoginSession::queryShardInfo()
{
    if (isRequesting()) {
        return QueryShardInfoResult(ecClientAlreadyRequested);
    }

    if (! isConnected()) {
        return QueryShardInfoResult(ecClientNotConnected);
    }

    requestType_ = rtQueryShardInfo;
    queryShardInfoResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    c2l_queryShardInfo();

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (queryShardInfoResult_.isReponsed()) {
            return queryShardInfoResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return QueryShardInfoResult(ecClientTimeout);
}


SelectShardResult LoginSession::selectShard(ShardId shardId)
{
    if (isRequesting()) {
        return SelectShardResult(ecClientAlreadyRequested);
    }

    if (! isConnected()) {
        return SelectShardResult(ecClientNotConnected);
    }

    requestType_ = rtSelectShard;
    selectShardResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    c2l_selectShard(shardId);

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (selectShardResult_.isReponsed()) {
            return selectShardResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return SelectShardResult(ecClientTimeout);
}


ReserveNicknameResult LoginSession::reserveNickname(const Nickname& nickname)
{
    if (isRequesting()) {
        return ReserveNicknameResult(ecClientAlreadyRequested);
    }

    if (! isConnected()) {
        return ReserveNicknameResult(ecClientNotConnected);
    }

    requestType_ = rtReserveNickname;
    reserveNicknameResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    c2l_reserveNickname(nickname);

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (reserveNicknameResult_.isReponsed()) {
            return reserveNicknameResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return ReserveNicknameResult(ecClientTimeout);
}



CreateCharacterResult LoginSession::createCharacter(const CreateCharacterInfo& createCharacterInfo)
{
    if (isRequesting()) {
        return CreateCharacterResult(ecClientAlreadyRequested);
    }

    if (! isConnected()) {
        return CreateCharacterResult(ecClientNotConnected);
    }

    requestType_ = rtCreateCharacter;
    createCharacterResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    c2l_createCharacter(createCharacterInfo);

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (createCharacterResult_.isReponsed()) {
            return createCharacterResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return CreateCharacterResult(ecClientTimeout);
}


DeleteCharacterResult LoginSession::deleteCharacter(ObjectId characterId)
{
    if (isRequesting()) {
        return DeleteCharacterResult(ecClientAlreadyRequested);
    }

    if (! isConnected()) {
        return DeleteCharacterResult(ecClientNotConnected);
    }

    requestType_ = rtDeleteCharacter;
    deleteCharacterResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    c2l_deleteCharacter(characterId);

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (deleteCharacterResult_.isReponsed()) {
            return deleteCharacterResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return DeleteCharacterResult(ecClientTimeout);
}


EnterShardResult LoginSession::enterShard(ObjectId characterId)
{
    if (isRequesting()) {
        return EnterShardResult(ecClientAlreadyRequested);
    }

    if (! isConnected()) {
        return EnterShardResult(ecClientNotConnected);
    }

    requestType_ = rtEnterShard;
    enterShardResult_.reset();

	const auto deadline = sne::core::getTickCount() + getConfig().connectionTimeout_;

    c2l_enterShard(characterId);

	while (sne::core::getTickCount() < deadline) {
        handleMessages();

        if (enterShardResult_.isReponsed()) {
            return enterShardResult_;
        }

        sne::base::pause(1);
    }

    disconnect();
    return EnterShardResult(ecClientTimeout);
}


void LoginSession::disconnectFromServer()
{
    BaseSession::disconnectFromServer();

    requestType_ = rtNone;
}


bool LoginSession::connectToLoginServer(RequestType rt)
{
    assert(! isRequesting());

    const bool isAlreadyConnected = isConnected();

    if (! isAlreadyConnected) {
        if (! connect(getConfig().loginServerAddress_, getConfig().loginServerPort_)) {
            return false;
        }
    }

    requestType_ = rt;

    if (isAlreadyConnected) {
        onValidated();
    }

    return true;
}

// = rnf::ClientSessionCallback overriding

void LoginSession::onValidated()
{
    BaseSession::onValidated();

    switch (requestType_) {
    case rtLogin:
        c2l_login(userId_, password_);
        password_.clear();
        break;
    case rtRelogin:
        c2l_relogin(certificate_);
        break;
    default:
        assert(false);
    }
}


void LoginSession::onDisconnected()
{
    BaseSession::onDisconnected();

    switch (requestType_) {
    case rtLogin:
        loginResult_.errorCode_ = ecClientTimeout;
        break;
    case rtQueryShardInfo:
        queryShardInfoResult_.errorCode_ = ecClientTimeout;
        break;
    case rtSelectShard:
        selectShardResult_.errorCode_ = ecClientTimeout;
        break;
    case rtCreateCharacter:
        createCharacterResult_.errorCode_ = ecClientTimeout;
        break;
    case rtDeleteCharacter:
        deleteCharacterResult_.errorCode_ = ecClientTimeout;
        break;
    }

    requestType_ = rtNone;
}

// = rpc::LoginSessionRpc overriding

FORWARD_SRPC_METHOD_2(LoginSession, c2l_login,
    UserId, userId, UserPassword, userPassword);


FORWARD_SRPC_METHOD_1(LoginSession, c2l_relogin,
    Certificate, certificate);


FORWARD_SRPC_METHOD_0(LoginSession, c2l_queryShardInfo);


FORWARD_SRPC_METHOD_1(LoginSession, c2l_selectShard,
    ShardId, shardId);


FORWARD_SRPC_METHOD_1(LoginSession, c2l_reserveNickname,
    Nickname, nickname);


FORWARD_SRPC_METHOD_1(LoginSession, c2l_createCharacter,
    CreateCharacterInfo, createCharacterInfo);


FORWARD_SRPC_METHOD_1(LoginSession, c2l_deleteCharacter,
    ObjectId, characterId);


FORWARD_SRPC_METHOD_1(LoginSession, c2l_enterShard,
    ObjectId, characterId);


RECEIVE_SRPC_METHOD_4(LoginSession, c2l_onLogin,
    ErrorCode, errorCode, AccountInfo, accountInfo,
    CommunityServerInfo, communityServerInfo, CertificateMap, certificateMap)
{
    requestType_ = rtNone;
	
    loginResult_.errorCode_ = errorCode;
	loginResult_.accountInfo_ = accountInfo;
    loginResult_.communityServerInfo_ = communityServerInfo;
    loginResult_.certificateMap_ = certificateMap;
}


RECEIVE_SRPC_METHOD_2(LoginSession, c2l_onRelogin,
    ErrorCode, errorCode, Certificate, reissuedCertificate)
{
    requestType_ = rtNone;

    reloginResult_.errorCode_ = errorCode;
    reloginResult_.reissuedCertificate_ = reissuedCertificate;
}


RECEIVE_SRPC_METHOD_4(LoginSession, c2l_onQueryShardInfo,
    ErrorCode, errorCode, ShardInfoList, shardInfos, ZoneMapCodeMap, zoneMapCodeMap,
    CharacterCountPerShardMap, characterCounts)
{
    requestType_ = rtNone;

    queryShardInfoResult_.isResponsed_ = true;
    queryShardInfoResult_.errorCode_ = errorCode;
    queryShardInfoResult_.shardInfos_ = shardInfos;
    queryShardInfoResult_.zoneMapCodeMap_ = zoneMapCodeMap;
    queryShardInfoResult_.characterCounts_ = characterCounts;
}


RECEIVE_SRPC_METHOD_2(LoginSession, c2l_onSelectShard,
    ErrorCode, errorCode, FullUserInfo, userInfo)
{
    requestType_ = rtNone;

    selectShardResult_.isResponsed_ = true;
    selectShardResult_.errorCode_ = errorCode;
    selectShardResult_.fullUserInfo_ = userInfo;
}


RECEIVE_SRPC_METHOD_2(LoginSession, c2l_onReserveNickname,
    ErrorCode, errorCode, Nickname, nickname)
{
    requestType_ = rtNone;

    
    reserveNicknameResult_.isResponsed_ = true;
    reserveNicknameResult_.errorCode_ = errorCode;
    reserveNicknameResult_.nickname_ = nickname;
}


RECEIVE_SRPC_METHOD_2(LoginSession, c2l_onCreateCharacter,
    ErrorCode, errorCode, FullCharacterInfo, characterInfo)
{
    requestType_ = rtNone;

    createCharacterResult_.isResponsed_ = true;
    createCharacterResult_.errorCode_ = errorCode;
    createCharacterResult_.fullCharacterInfo_ = characterInfo;
}


RECEIVE_SRPC_METHOD_2(LoginSession, c2l_onDeleteCharacter,
    ErrorCode, errorCode, ObjectId, characterId)
{
    requestType_ = rtNone;

    deleteCharacterResult_.isResponsed_ = true;
    deleteCharacterResult_.errorCode_ = errorCode;
    deleteCharacterResult_.characterId_ = characterId;
}


RECEIVE_SRPC_METHOD_2(LoginSession, c2l_onEnterShard,
    ErrorCode, errorCode, ZoneServerInfo, zoneServerInfo)
{
    requestType_ = rtNone;

    enterShardResult_.isResponsed_ = true;
    enterShardResult_.errorCode_ = errorCode;
    enterShardResult_.zoneServerInfo_ = zoneServerInfo;
}


RECEIVE_SRPC_METHOD_1(LoginSession, c2l_evExpelled,
    ExpelReason, reason)
{
    sessionCallback_.expelled(reason);
}


}} // namespace gideon { namespace clientnet {
