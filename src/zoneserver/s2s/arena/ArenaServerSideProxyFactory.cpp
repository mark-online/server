#include "ZoneServerPCH.h"
#include "ArenaServerSideProxyFactory.h"
#include "ArenaServerSideProxy.h"
#include <sne/server/s2s/ServerSideBridge.h>
#include <sne/sgp/session/SgpSessionImplFactory.h>

namespace gideon { namespace zoneserver {

std::unique_ptr<sne::base::Session> ArenaServerSideProxyFactory::create() const
{
    const sne::server::ServerSideSessionConfig sessionConfig =
        makeSessionConfig();

    auto bridge = std::make_unique<sne::server::ServerSideBridge>(
            sessionConfig,
            getServerSpec(),
            sne::sgp::SgpSessionImplFactory::create(sessionConfig));

    ArenaServerSideProxy* proxy = new ArenaServerSideProxy(bridge.get());
    proxy; // proxy의 메모리는 bridge가 해제함

    return bridge;
}

}} // namespace gideon { namespace zoneserver {
