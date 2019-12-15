#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ArenaInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <sne/server/common/SocketAddress.h>
#include <sne/server/s2s/ServerSideProxyManager.h>
#include <sne/core/container/Containers.h>

namespace gideon { namespace zoneserver {

class ArenaServerSideProxy;

/**
 * @class ArenaServerSideProxyManager
 *
 * S2S(Zone) Server 관리자
 * - 목록 유지
 * - 이벤트 전파(broadcast)
 * - Composite Pattern
 */
class ZoneServer_Export ArenaServerSideProxyManager :
    public sne::server::ServerSideProxyManager
{
public:
    ArenaServerSideProxyManager();

	void matched(sne::server::ServerId serverId, ObjectId arenaId, ObjectId playerId, MapCode mapCode);

private:
    virtual void proxyRemoved(sne::server::ServerId /*proxyId*/) {}
    virtual void connected() {}
    
    ArenaServerSideProxy* getArenaServerSideProxy(sne::server::ServerId serverId);

};

}} // namespace gideon { namespace zoneserver {

#define ARENASERVERSIDEPROXY_MANAGER \
    static_cast<gideon::zoneserver::ArenaServerSideProxyManager*>( \
        SERVERSIDEPROXY_MANAGER)
