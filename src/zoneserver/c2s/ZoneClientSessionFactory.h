#pragma once

#include <sne/server/session/AbstractServerSideSessionFactory.h>

namespace gideon { namespace zoneserver {

/**
 * @class ZoneClientSessionFactory
 *
 * GameZone Server Client Session Factory
 */
class ZoneClientSessionFactory :
    public sne::server::AbstractServerSideSessionFactory
{
public:
    ZoneClientSessionFactory(const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask)  :
        sne::server::AbstractServerSideSessionFactory(serverSpec, ioContextTask) {}

private:
    virtual std::unique_ptr<sne::base::Session> create() const;
};

}} // namespace gideon { namespace zoneserver {
