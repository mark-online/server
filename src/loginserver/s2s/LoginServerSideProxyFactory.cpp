#include "LoginServerPCH.h"
#include "LoginServerSideProxyFactory.h"
#include "LoginServerSideProxy.h"
#include <sne/server/s2s/ServerSideBridge.h>
#include <sne/sgp/session/SgpSessionImplFactory.h>

namespace gideon { namespace loginserver {

std::unique_ptr<sne::base::Session> LoginServerSideProxyFactory::create() const
{
    const sne::server::ServerSideSessionConfig sessionConfig =
        makeSessionConfig();

    auto bridge = std::make_unique<sne::server::ServerSideBridge>(
        sessionConfig,
        getServerSpec(),
        sne::sgp::SgpSessionImplFactory::create(sessionConfig));

    LoginServerSideProxy* proxy = new LoginServerSideProxy(bridge.get());
    proxy; // proxy의 메모리는 bridge가 해제함

    return bridge;
}

}} // namespace gideon { namespace loginserver {
