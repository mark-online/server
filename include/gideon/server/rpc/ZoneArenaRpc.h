#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/cs/shared/data/ArenaInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <sne/srpc/RpcInterface.h>

namespace gideon { namespace rpc {

/**
 * @class ZoneArenaRpc
 * Zone Server <-> Arena Server messages
 */
class ZoneArenaRpc : boost::noncopyable
{
public:
    virtual ~ZoneArenaRpc() {}

public:
    /**
     * 투기장 서버와 랑데뷰한다.
     * @param onlineUsers 존 서버에 접속 중인 사용자 목록
     */
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2a_rendezvous,
        ZoneId, zoneId);

    /// 랑데뷰 요청에 대한 응답
    DECLARE_SRPC_METHOD_0(ZoneLoginRpc, z2a_onRendezvous);

public:
    /// 투기장 서버에게 준비 완료를 알린다.
    DECLARE_SRPC_METHOD_0(ZoneLoginRpc, z2a_ready);

public:
    /// 투기장 서버에게 탈영병을 지우라고 알린다..(치트키)
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2a_releaseDeserter,
        ObjectId, playerId);

public:
	/// 투기장 참가 신청을 한다.
	DECLARE_SRPC_METHOD_2(ZoneLoginRpc, z2a_challengeSolo,
		ObjectId, playerId, MapCode, modeMapCode);

	/// 투기장 참가 신청 응답.
	DECLARE_SRPC_METHOD_3(ZoneLoginRpc, z2a_onChallengeSolo,
		ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode);

public:
	DECLARE_SRPC_METHOD_2(ZoneLoginRpc, z2a_cancelChallenge,
		ObjectId, playerId, MapCode, modeMapCode);

	DECLARE_SRPC_METHOD_3(ZoneLoginRpc, z2a_onCancelChallenge,
		ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode);

public:
    DECLARE_SRPC_METHOD_1(ZoneLoginRpc, z2a_logoutChallenger,
        ObjectId, playerId);

public:
	DECLARE_SRPC_METHOD_4(ZoneLoginRpc, z2a_evArenaMatched,
		ObjectId, arenaId, ObjectId, playerId, MapCode, modeMapCode, MapCode, arenaWorldMapCode);

};

}} // namespace gideon { namespace rpc {
