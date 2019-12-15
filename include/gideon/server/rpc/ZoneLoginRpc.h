#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <gideon/server/data/BanInfo.h>
#include <sne/srpc/RpcInterface.h>

namespace gideon { namespace rpc {

/**
 * @class ZoneLoginRpc
 * Zone Server <-> Login Server messages
 */
class ZoneLoginRpc : boost::noncopyable
{
public:
    virtual ~ZoneLoginRpc() {}

public:
    /**
     * 로그인 서버와 랑데뷰한다.
     * @param onlineUsers 존 서버에 접속 중인 사용자 목록
     */
    DECLARE_SRPC_METHOD_4(ZoneLoginRpc, z2l_rendezvous,
        ShardId, shardId, ZoneId, zoneId, AccountIds, onlineUsers, uint16_t, maxUserCount);

    /// 랑데뷰 요청에 대한 응답
    DECLARE_SRPC_METHOD_0(ZoneLoginRpc, z2l_onRendezvous);

public:
    /// 로그인 서버에게 준비 완료를 알린다.
    DECLARE_SRPC_METHOD_0(ZoneLoginRpc, z2l_ready);

public:
    /// 사용자가 존 서버에 로그인 요청하였음을 알린다
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_loginZoneUser,
        Certificate, certificate);

    /// 사용자 존 로그인 요청에 대한 응답
    DECLARE_SRPC_METHOD_3(ZoneLoginRpc, z2l_onLoginZoneUser,
        ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);

public:
    /// 존 서버 이동을 예약한다
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_reserveMigration,
        AccountId, accountId);

public:
    /// 사용자가 존 서버에서 로그아웃 하였음을 알린다
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_logoutUser,
        AccountId, accountId);

    // = Login -> Zone
public:
    /// 존서버에게 사용자 정보를 요청한다
    /// - response: z2l_onGetFullUserInfo
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_getFullUserInfo,
        AccountId, accountId);

    /// 정보를 요청에 대한 응답
    DECLARE_SRPC_METHOD_2(ZoneLoginRpc, z2l_onGetFullUserInfo,
        ErrorCode, errorCode, FullUserInfo, userInfo);

public:
    /// 존서버에게 캐릭터 생성을 요청한다
    /// - response: z2l_onCreateCharacter
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_createCharacter,
        CreateCharacterInfo, createCharacterInfo);

    /// 캐릭터 생성 요청에 대한 응답
    DECLARE_SRPC_METHOD_3(ZoneLoginRpc, z2l_onCreateCharacter,
        ErrorCode, errorCode, AccountId, accountId, FullCharacterInfo, characterInfo);

public:
    /// 존서버에게 캐릭터 삭제를 요청한다
    /// - response: z2l_onDeleteCharacter
    DECLARE_SRPC_METHOD_2(ZoneLoginRpc, z2l_deleteCharacter,
        AccountId, accountId, ObjectId, characterId);

    /// 캐릭터 삭제 요청에 대한 응답
    DECLARE_SRPC_METHOD_3(ZoneLoginRpc, z2l_onDeleteCharacter,
        ErrorCode, errorCode, AccountId, accountId, ObjectId, characterId);

public:
    /// 존서버에게 닉네임 예약을 요청한다
    /// - response: z2l_onCheckDuplicateNickname
    DECLARE_SRPC_METHOD_2(ZoneLoginRpc, z2l_checkDuplicateNickname,
        AccountId, accountId, Nickname, nickname);

    /// 닉네임 예약 요청에 대한 응답
    DECLARE_SRPC_METHOD_3(ZoneLoginRpc, z2l_onCheckDuplicateNickname,
        ErrorCode, errorCode, AccountId, accountId, Nickname, nickname);

public:
    /// 사용자가 강제 퇴장 되었음을 알린다
    DECLARE_SRPC_METHOD_2(ZoneLoginRpc, z2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);

public:
    /// 존서버가 활성화되었음을 알린다
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_evZoneServerActivated,
        ZoneId, zoneId);

    /// 존서버가 비활성화되었음을 알린다
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_evZoneServerDeactivated,
        ZoneId, zoneId);

public:
	DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2l_evAddBanInfo,
		BanInfo, banInfo);
	DECLARE_SRPC_METHOD_4(ZoneLoginRpc, z2l_evRemoveBanInfo,
		BanMode, banMode, AccountId, accountId, ObjectId, characterId, std::string, ipAddress);
};

}} // namespace gideon { namespace rpc {
