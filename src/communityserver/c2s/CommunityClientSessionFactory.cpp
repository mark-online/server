#include "CommunityServerPCH.h"
#include "CommunityClientSessionFactory.h"
#include "CommunityClientSession.h"
#include <sne/sgp/session/SgpSessionImplFactory.h>

namespace gideon { namespace communityserver {

std::unique_ptr<sne::base::Session> CommunityClientSessionFactory::create() const
{
    const sne::server::ServerSideSessionConfig sessionConfig =
        makeSessionConfig();

    return std::make_unique<CommunityClientSession>(
        sessionConfig, getServerSpec(),
        sne::sgp::SgpSessionImplFactory::create(sessionConfig));
}

}} // namespace gideon { namespace communityserver {
