#include "LoginServerPCH.h"
#include "LoginClientSessionFactory.h"
#include "LoginClientSession.h"
#include <sne/sgp/session/SgpSessionImplFactory.h>

namespace gideon { namespace loginserver {

std::unique_ptr<sne::base::Session> LoginClientSessionFactory::create() const
{
    const sne::server::ServerSideSessionConfig sessionConfig =
        makeSessionConfig();

    return std::make_unique<LoginClientSession>(
        sessionConfig,
        getServerSpec(),
        sne::sgp::SgpSessionImplFactory::create(sessionConfig));
}

}} // namespace gideon { namespace loginserver {
