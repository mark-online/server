#pragma once

#include <sne/server/session/AbstractServerSideSessionFactory.h>

namespace gideon { namespace databaseproxyserver {

/**
 * @class DatabaseProxyServerSideSessionFactory
 *
 * GameDatabaseProxy Server Client Session Factory
 */
class DatabaseProxyServerSideSessionFactory :
    public sne::server::AbstractServerSideSessionFactory
{
public:
    DatabaseProxyServerSideSessionFactory(const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask)  :
        sne::server::AbstractServerSideSessionFactory(serverSpec, ioContextTask) {}

private:
    virtual std::unique_ptr<sne::base::Session> create() const;
};

}} // namespace gideon { namespace databaseproxyserver {
