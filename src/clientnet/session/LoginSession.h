#pragma once

#include "BaseSession.h"
#include "SessionCallback.h"
#include <gideon/clientnet/ServerProxyCallback.h>
#include <gideon/cs/shared/rpc/detail/LoginRpc.h>

namespace gideon { namespace clientnet {

/**
 * @struct ReloginResult
 * 재 로그인 요청에 대한 결과
 */
struct ReloginResult
{
	ErrorCode errorCode_;
    Certificate reissuedCertificate_;
	
	explicit ReloginResult(ErrorCode errorCode = ecOk) :
		errorCode_(errorCode) {}

    void reset() {
        errorCode_ = ecOk;
        reissuedCertificate_.reset();
    }

	bool isReponsed() const {
		return isFailed(errorCode_) || reissuedCertificate_.isValid();
	}
};


/**
 * @class LoginSession
 */
class LoginSession :
    public BaseSession,
    public rpc::LoginSessionRpc
{
    enum RequestType {
        rtNone = 0,
		rtLogin,
        rtRelogin,
        rtReserveNickname,
        rtQueryShardInfo,
        rtSelectShard,
        rtCreateCharacter,
        rtDeleteCharacter,
        rtEnterShard
    };
public:
    LoginSession(const ServerProxyConfig& config,
        LoginSessionCallback& sessionCallback,
        sne::client::ClientSessionTick& tick);
    virtual ~LoginSession();

    /// 로그인 서버에 로그인한다
    /// - 완료할 때까지 대기(blocking)한다
    LoginResult login(const UserId& userId, const UserPassword& password);

    /// 로그인 서버에 재로그인한다
    /// - 완료할 때까지 대기(blocking)한다
    ReloginResult relogin(const Certificate& certificate);

    /// 로그인 서버에 Shard 정보를 요청한다.
    /// - 완료할 때까지 대기(blocking)한다
    QueryShardInfoResult queryShardInfo();

    /// Shard 선택을 요청한다.
    /// - 완료할 때까지 대기(blocking)한다
    SelectShardResult selectShard(ShardId shardId);

    /// 닉네임 예약을 요청한다.
    /// - 완료할 때까지 대기(blocking)한다
    ReserveNicknameResult reserveNickname(const Nickname& nickname);

    /// 캐릭터 생성을 요청한다.
    /// - 완료할 때까지 대기(blocking)한다
    CreateCharacterResult createCharacter(const CreateCharacterInfo& createCharacterInfo);

    /// 캐릭터 삭제를 요청한다.
    /// - 완료할 때까지 대기(blocking)한다
    DeleteCharacterResult deleteCharacter(ObjectId characterId);

    /// Shard 입장을 요청한다.
    /// - 완료할 때까지 대기(blocking)한다
    EnterShardResult enterShard(ObjectId characterId);

private:
    bool connectToLoginServer(RequestType rt);

    bool isRequesting() const {
        return rtNone != requestType_;
    }

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
    virtual void disconnectFromServer();

private:
    // = rnf::ClientSessionCallback overriding
    virtual void onValidated();
    virtual void onDisconnected();

private:
    LoginSessionCallback& sessionCallback_;
    RequestType requestType_;

    UserId userId_;
    UserPassword password_;
    std::string userEmail_;
    Certificate certificate_;

    LoginResult loginResult_;
    ReloginResult reloginResult_;
    QueryShardInfoResult queryShardInfoResult_;
    SelectShardResult selectShardResult_;
    ReserveNicknameResult reserveNicknameResult_;
    CreateCharacterResult createCharacterResult_;
    DeleteCharacterResult deleteCharacterResult_;
    EnterShardResult enterShardResult_;
};

}} // namespace gideon { namespace clientnet {
