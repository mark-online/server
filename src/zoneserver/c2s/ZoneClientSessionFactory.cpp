#include "ZoneServerPCH.h"
#include "ZoneClientSessionFactory.h"
#include "ZoneClientSession.h"
#include <sne/sgp/session/SgpSessionImplFactory.h>

namespace gideon { namespace zoneserver {

std::unique_ptr<sne::base::Session> ZoneClientSessionFactory::create() const
{
    const sne::server::ServerSideSessionConfig sessionConfig =
        makeSessionConfig();

    return std::make_unique<ZoneClientSession>(sessionConfig,
        getServerSpec(),
        sne::sgp::SgpSessionImplFactory::create(sessionConfig));
}

}} // namespace gideon { namespace zoneserver {
