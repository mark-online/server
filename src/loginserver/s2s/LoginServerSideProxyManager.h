#pragma once

#include "../loginserver_export.h"
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <sne/server/common/SocketAddress.h>
#include <sne/server/s2s/ServerSideProxyManager.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/core/container/Containers.h>

namespace gideon { namespace loginserver {

class LoginServerSideProxy;

/**
 * @class LoginServerSideProxyManager
 *
 * S2S(Zone) Server 관리자
 * - 목록 유지
 * - 이벤트 전파(broadcast)
 * - Composite Pattern
 */
class LoginServer_Export LoginServerSideProxyManager :
    public sne::server::ServerSideProxyManager
{
public:
    LoginServerSideProxyManager();

    /// 해당 사용자를 해당 서버에서 접속 해제 시킨다
    void expelUser(AccountId accountId, ExpelReason reason, sne::server::ServerId proxyId);

    /// 모든 서버로 부터 접속을 해제한다
    void expelUser(AccountId accountId, ExpelReason reason);

    /// Shard 입장 요청 (유저 상세 정보를 얻기 위함)
    bool requestFullUserInfo(sne::server::ServerId proxyId, AccountId accountId);

    /// 캐릭터 생성 요청
    bool requestCreateCharacter(sne::server::ServerId proxyId, 
        const CreateCharacterInfo& createCharacterInfo);

    /// 캐릭터 삭제 요청
    bool requestDeleteCharacter(sne::server::ServerId proxyId, AccountId accountId,
        ObjectId characterId);

    /// 닉네임 예약 요청
    bool requestReserveNickname(sne::server::ServerId proxyId, AccountId accountId,
        const Nickname& nickname);

public:
    void zoneServerConnected(ShardId shardId, ZoneId zoneId);
    void zoneServerDisconnected(ShardId shardId, ZoneId zoneId);

private:
    virtual void proxyRemoved(sne::server::ServerId /*proxyId*/) {}
    virtual void connected() {}
};

}} // namespace gideon { namespace loginserver {

#define LOGINSERVERSIDEPROXY_MANAGER \
    static_cast<gideon::loginserver::LoginServerSideProxyManager*>( \
        SERVERSIDEPROXY_MANAGER)
