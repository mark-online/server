#include "CommunityServerPCH.h"
#include "CommunityServerSideProxyFactory.h"
#include "CommunityServerSideProxy.h"
#include <sne/server/s2s/ServerSideBridge.h>
#include <sne/sgp/session/SgpSessionImplFactory.h>

namespace gideon { namespace communityserver {

std::unique_ptr<sne::base::Session> CommunityServerSideProxyFactory::create() const
{
    const sne::server::ServerSideSessionConfig sessionConfig =
        makeSessionConfig();

    auto bridge =
        std::make_unique<sne::server::ServerSideBridge>(
            sessionConfig,
            getServerSpec(),
            sne::sgp::SgpSessionImplFactory::create(sessionConfig));

    CommunityServerSideProxy* proxy = new CommunityServerSideProxy(bridge.get());
    proxy; // proxy의 메모리는 bridge가 해제함

    return bridge;
}

}} // namespace gideon { namespace communityserver {
