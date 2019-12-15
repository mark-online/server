#pragma once

#include <sne/server/common/SocketAddress.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <sne/srpc/RpcInterface.h>

namespace gideon { namespace rpc {

/**
 * @class CommunityLoginRpc
 * Community Server <-> Login Server messages
 */
class CommunityLoginRpc : boost::noncopyable
{
public:
    virtual ~CommunityLoginRpc() {}

public:
    /**
     * 로그인 서버와 랑데뷰한다.
     * @param onlineUsers 커뮤니티 서버에 접속 중인 사용자 목록
     */
    DECLARE_SRPC_METHOD_1(CommunityLoginRpc, m2l_rendezvous,
        AccountIds, onlineUsers);

    /// 랑데뷰 요청에 대한 응답
    DECLARE_SRPC_METHOD_0(CommunityLoginRpc, m2l_onRendezvous);

public:
    /**
     * 로그인 서버에게 준비 완료를 알린다.
     */
    DECLARE_SRPC_METHOD_0(CommunityLoginRpc, m2l_ready);

public:
    /// 사용자가 커뮤니티 서버에 로그인 요청하였음을 알린다
    DECLARE_SRPC_METHOD_1(CommunityLoginRpc, m2l_loginCommunityUser,
        Certificate, certificate);

    /// 사용자 커뮤니티 로그인 요청에 대한 응답
    DECLARE_SRPC_METHOD_3(CommunityLoginRpc, m2l_onLoginCommunityUser,
        ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);

public:
    /// 사용자가 커뮤니티 서버에서 로그아웃 하였음을 알린다
    DECLARE_SRPC_METHOD_1(CommunityLoginRpc, m2l_logoutUser,
        AccountId, accountId);

public:
    /// 사용자가 강제 퇴장 되었음을 알린다
    DECLARE_SRPC_METHOD_2(CommunityLoginRpc, m2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);
};

}} // namespace gideon { namespace rpc {
