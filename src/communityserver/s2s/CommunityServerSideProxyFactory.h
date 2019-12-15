#pragma once

#include <sne/server/session/AbstractServerSideSessionFactory.h>

namespace gideon { namespace communityserver {

/**
 * @class CommunityServerSideProxyFactory
 *
 * CommunityServer-side SessionFactory
 */
class CommunityServerSideProxyFactory :
    public sne::server::AbstractServerSideSessionFactory
{
public:
    CommunityServerSideProxyFactory(const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask) :
        sne::server::AbstractServerSideSessionFactory(serverSpec, ioContextTask) {}

private:
    virtual std::unique_ptr<sne::base::Session> create() const;
};

}} // namespace gideon { namespace communityserver {
