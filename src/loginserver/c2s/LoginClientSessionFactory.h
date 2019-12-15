#pragma once

#include <sne/server/session/AbstractServerSideSessionFactory.h>

namespace gideon { namespace loginserver {

/**
 * @class LoginClientSessionFactory
 *
 * LoginServer Client Session Factory
 */
class LoginClientSessionFactory :
    public sne::server::AbstractServerSideSessionFactory
{
public:
    LoginClientSessionFactory(const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask)  :
        sne::server::AbstractServerSideSessionFactory(serverSpec, ioContextTask) {}

private:
    std::unique_ptr<sne::base::Session> create() const override;
};

}} // namespace gideon { namespace loginserver {
