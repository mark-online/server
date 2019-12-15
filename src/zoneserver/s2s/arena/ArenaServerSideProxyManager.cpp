#include "ZoneServerPCH.h"
#include "ArenaServerSideProxyManager.h"
#include "ArenaServerSideProxy.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

namespace
{

inline ArenaServerSideProxy* toProxy(sne::server::ServerSideProxy* proxy)
{
    return static_cast<ArenaServerSideProxy*>(proxy);
}

} // namespace


ArenaServerSideProxyManager::ArenaServerSideProxyManager() :
    sne::server::ServerSideProxyManager()
{
}



void ArenaServerSideProxyManager::matched(sne::server::ServerId serverId,
    ObjectId arenaId, ObjectId playerId, MapCode modeMapCode)
{
    ArenaServerSideProxy* proxy = getArenaServerSideProxy(serverId);
    if (proxy) {
        proxy->z2a_evArenaMatched(arenaId, playerId, modeMapCode, WORLD->getGlobalWorldMap().getMapCode());
    }
}


ArenaServerSideProxy* ArenaServerSideProxyManager::getArenaServerSideProxy(sne::server::ServerId serverId)
{
    Proxies proxies = getCopiedProxies();
    Proxies::iterator pos = proxies.find(serverId);
    if (pos != proxies.end()) {
        return toProxy((*pos).second);
    }
    return nullptr;
}


}} // namespace gideon { namespace zoneserver {
