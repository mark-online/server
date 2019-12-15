#pragma once

#include <sne/server/session/AbstractServerSideSessionFactory.h>

namespace gideon { namespace loginserver {

/**
 * @class LoginServerSideProxyFactory
 *
 * LoginServer-side SessionFactory
 */
class LoginServerSideProxyFactory :
    public sne::server::AbstractServerSideSessionFactory
{
public:
    LoginServerSideProxyFactory(const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask) :
        sne::server::AbstractServerSideSessionFactory(serverSpec, ioContextTask) {}

private:
    virtual std::unique_ptr<sne::base::Session> create() const;
};

}} // namespace gideon { namespace loginserver {
