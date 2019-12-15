#include "DatabaseProxyServerPCH.h"
#include "DatabaseProxyServerSideSessionFactory.h"
#include "DatabaseProxyServerSideRpcExtension.h"
#include <sne/database/proxy/server/s2s/DatabaseProxyServerSideSession.h>
#include <sne/sgp/session/SgpSessionImplFactory.h>
#include <sne/base/session/impl/SessionImpl.h>

namespace gideon { namespace databaseproxyserver {

std::unique_ptr<sne::base::Session> DatabaseProxyServerSideSessionFactory::create() const
{
    const sne::server::ServerSideSessionConfig sessionConfig = makeSessionConfig();

    auto sessionImpl =
        sne::sgp::SgpSessionImplFactory::create(sessionConfig);
    sessionImpl->registerExtension<DatabaseProxyServerSideRpcExtension>();

    return std::make_unique<sne::database::proxy::DatabaseProxyServerSideSession>(
        sessionConfig, getServerSpec(), std::move(sessionImpl));
}

}} // namespace gideon { namespace databaseproxyserver {
