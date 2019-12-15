#ifndef CLIENTNET_SERVERPROXY_H
#define CLIENTNET_SERVERPROXY_H

#pragma once

#include <sne/Common.h>
#include "ServerProxyCallback.h"
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <boost/noncopyable.hpp>

namespace sne { namespace srpc {
class RpcForwarder;
class RpcReceiver;
}} // namespace sne { namespace srpc {

namespace gideon { namespace clientnet {

/**
 * @class ServerProxy
 *
 * Gideon 클라이언트 세션 인터페이스
 *
 * - 기본적으로 모든 요청은 비동기로 동작한다.
 */
class ServerProxy : public boost::noncopyable
{
public:
    virtual ~ServerProxy() {}

    /**
     * 네트웍 I/O를 처리한다.
     */
    virtual void tick() = 0;

    /**
     * 폴링을 사용할 경우 수신된 메세지만 처리한다.
     */
    virtual void handleMessages() = 0;

    /**
     * 모든 서버와의 접속을 해제한다
     * - 별도의 이벤트가 발생하지 않는다
     */
    virtual void disconnect() = 0;

    virtual void registerRpcForwarder(sne::srpc::RpcForwarder& forwarder) = 0;
    virtual void registerRpcReceiver(sne::srpc::RpcReceiver& receiver) = 0;

    virtual void unregisterRpcForwarder(sne::srpc::RpcForwarder& forwarder) = 0;
    virtual void unregisterRpcReceiver(sne::srpc::RpcReceiver& receiver) = 0;

    virtual void registerRpcForwarderForCommunityServer(sne::srpc::RpcForwarder& forwarder) = 0;
    virtual void registerRpcReceiverForCommunityServer(sne::srpc::RpcReceiver& receiver) = 0;

    virtual void unregisterRpcForwarderForCommunityServer(sne::srpc::RpcForwarder& forwarder) = 0;
    virtual void unregisterRpcReceiverForCommunityServer(sne::srpc::RpcReceiver& receiver) = 0;

public:
    /**
     * 로그인 서버에 로그인한다
     * - 완료될 때까지 대기(block)한다
     */
    virtual LoginResult logInLoginServer(const UserId& userId,
        const UserPassword& password) = 0;

    /// 로그인 서버로 부터 로그아웃한다
    virtual void logOutLoginServer() = 0;

    /**
     * 로그인 서버에 Shard 정보를 요청한다
     * - 완료될 때까지 대기(block)한다
     */
    virtual QueryShardInfoResult queryShardInfo() = 0;

    /**
     * Shard를 선택한다
     * - 완료될 때까지 대기(block)한다
     */
    virtual SelectShardResult selectShard(ShardId shardId) = 0;

    /**
     * 닉네임 예약을 한다
     * - 완료될 때까지 대기(block)한다
     */
    virtual ReserveNicknameResult reserveNickname(const Nickname& nickname) = 0;


    /**
     * 캐릭터를 생성한다
     * - 완료될 때까지 대기(block)한다
     */
    virtual CreateCharacterResult createCharacter(const CreateCharacterInfo& createCharacterInfo) = 0;

    /**
     * 캐릭터를 삭제한다
     * - 완료될 때까지 대기(block)한다
     */
    virtual DeleteCharacterResult deleteCharacter(ObjectId characterId) = 0;

    /**
     * Shard 입장을 요청한다
     * - 완료될 때까지 대기(block)한다
     */
    virtual EnterShardResult enterShard(ObjectId characterId) = 0;

public:
    /**
     * 커뮤니티 서버에 연결하고 로그인을 요청한다.
     * - 완료될 때까지 대기(block)한다
     * @param spawnRegionCode 커뮤니티 서버 이동의 경우 스폰될 지역 코드
     */
    virtual ErrorCode logInCommunityServer(const std::string& ip,
        uint16_t port) = 0;

public:
    /**
     * 존 서버에 연결하고 로그인을 요청한다.
     * - 완료될 때까지 대기(block)한다
     * @param spawnRegionCode 존 서버 이동의 경우 스폰될 지역 코드
	 * @param arenaId 투기장 서버로 들어갈때
     * @position 특정위치(글로벌 월드맵)로 갈때 (spawnRegionCode 유효하지 않을때 값이 넘어온다)
     */
    virtual ErrorCode logInZoneServer(const std::string& ip,
        uint16_t port, RegionCode spawnRegionCode, ObjectId arenaId,
        MapCode preGlobalMapCode, const Position& position) = 0;

    /**
     * 현재 접속 종인 존 서버에 로그아웃을 요청한다
     * - 서버는 일정 시간동안 대기 후 로그아웃 처리를 한다
     */
    virtual bool reserveZoneServerLogout() = 0;

    /**
     * 현재 접속 종인 존 서버에 로그아웃을 요청을 취소한다
     */
    virtual bool cancelZoneServerLogout() = 0;

public:
    /// 커뮤니티 서버에 로그인 중인가?
    virtual bool isLoggingInCommunityServer() const = 0;

    /// 커뮤니티 서버에 로그인되었는가?
    virtual bool isLoggedInCommunityServer() const = 0;

public:
    /// 존서버에 로그인 중인가?
    virtual bool isLoggingInZoneServer() const = 0;

    /// 존 서버에 로그인되었는가?
    virtual bool isLoggedInZoneServer() const = 0;
};

}} // namespace gideon { namespace clientnet {

#endif // CLIENTNET_SERVERPROXY_H
