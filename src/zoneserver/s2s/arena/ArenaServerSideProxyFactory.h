#pragma once

#include <sne/server/session/AbstractServerSideSessionFactory.h>

namespace gideon { namespace zoneserver {

/**
 * @class ArenaServerSideProxyFactory
 *
 * ArenaServer-side SessionFactory
 */
class ArenaServerSideProxyFactory :
    public sne::server::AbstractServerSideSessionFactory
{
public:
    ArenaServerSideProxyFactory(const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask) :
        sne::server::AbstractServerSideSessionFactory(serverSpec, ioContextTask) {}

private:
    virtual std::unique_ptr<sne::base::Session> create() const override;
};

}} // namespace gideon { namespace zoneserver {
