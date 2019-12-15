#pragma once

#include <sne/server/session/AbstractServerSideSessionFactory.h>

namespace gideon { namespace communityserver {

/**
 * @class CommunityClientSessionFactory
 *
 * GameCommunity Server Client Session Factory
 */
class CommunityClientSessionFactory :
    public sne::server::AbstractServerSideSessionFactory
{
public:
    CommunityClientSessionFactory(const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask)  :
        sne::server::AbstractServerSideSessionFactory(serverSpec, ioContextTask) {}

private:
    virtual std::unique_ptr<sne::base::Session> create() const override;
};

}} // namespace gideon { namespace communityserver {
