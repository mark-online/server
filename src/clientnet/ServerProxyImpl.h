#ifndef CLIENTNET_SERVERPROXYIMPL_H
#define CLIENTNET_SERVERPROXYIMPL_H

#pragma once

#include "session/SessionCallback.h"
#include <gideon/clientnet/ServerProxyConfig.h>
#include <gideon/clientnet/ServerProxy.h>
#include <sne/client/session/ClientSessionTick.h>

namespace gideon { namespace clientnet {

class ServerProxyCallback;
class LoginSession;
class CommunitySession;
class ZoneSession;

/**
 * @class ServerProxyImpl
 *
 * Gideon 클라이언트 세션 구현 클래스
 *
 * - 서버들과의 접속을 유지한다.
 * - Fascade pattern
 */
class ServerProxyImpl : public ServerProxy,
    private LoginSessionCallback,
    private CommunitySessionCallback,
    private ZoneSessionCallback
{
public:
    ServerProxyImpl(ServerProxyCallback& callback,
        const ServerProxyConfig& config);

    virtual ~ServerProxyImpl();

private:
    virtual void tick();

    virtual void handleMessages();

    virtual void disconnect();

private:
    virtual LoginResult logInLoginServer(const UserId& userId,
        const UserPassword& password);
    virtual void logOutLoginServer();

    virtual QueryShardInfoResult queryShardInfo();

    virtual SelectShardResult selectShard(ShardId shardId);

    virtual ReserveNicknameResult reserveNickname(const Nickname& nickname);

    virtual CreateCharacterResult createCharacter(const CreateCharacterInfo& createCharacterInfo);

    virtual DeleteCharacterResult deleteCharacter(ObjectId characterId);

    virtual EnterShardResult enterShard(ObjectId characterId);

private:
    virtual ErrorCode logInCommunityServer(const std::string& ip, uint16_t port);

private:
    virtual ErrorCode logInZoneServer(const std::string& ip, uint16_t port,
        RegionCode spawnRegionCode, ObjectId arenaId, MapCode preGlobalMapCode,
        const Position& position);
    virtual bool reserveZoneServerLogout();
    virtual bool cancelZoneServerLogout();

private:
    virtual void registerRpcForwarder(sne::srpc::RpcForwarder& forwarder);
    virtual void registerRpcReceiver(sne::srpc::RpcReceiver& receiver);

    virtual void unregisterRpcForwarder(sne::srpc::RpcForwarder& forwarder);
    virtual void unregisterRpcReceiver(sne::srpc::RpcReceiver& receiver);

private:
    virtual void registerRpcForwarderForCommunityServer(sne::srpc::RpcForwarder& forwarder);
    virtual void registerRpcReceiverForCommunityServer(sne::srpc::RpcReceiver& receiver);

    virtual void unregisterRpcForwarderForCommunityServer(sne::srpc::RpcForwarder& forwarder);
    virtual void unregisterRpcReceiverForCommunityServer(sne::srpc::RpcReceiver& receiver);

private:
    virtual bool isLoggingInCommunityServer() const;
    virtual bool isLoggedInCommunityServer() const;
    virtual bool isLoggingInZoneServer() const;
    virtual bool isLoggedInZoneServer() const;

private:
    void initSessions();
    void finiSessions();

    /**
     * 로그인 서버에 인증(재 로그인)한다
     * - 완료될 때까지 대기(block)한다
     */
    ErrorCode relogInLoginServer();

private:
    // = LoginSessionCallback implementing
    virtual void expelled(ExpelReason reason);

private:
    // = CommunitySessionCallback implementing
    virtual void communityServerDisconnected();

private:
    // = ZoneSessionCallback implementing
    virtual void zoneServerDisconnected();

private:
    ServerProxyCallback& callback_;
    const ServerProxyConfig config_;

    sne::client::ClientSessionTick sessionTick_;

    std::unique_ptr<LoginSession> loginSession_;
    std::unique_ptr<CommunitySession> communitySession_;
    std::unique_ptr<ZoneSession> zoneSession_;

    CertificateMap certificateMap_;
};

}} // namespace gideon { namespace clientnet {

#endif // CLIENTNET_SERVERPROXYIMPL_H
