#pragma once

#include "communityserver_export.h"
#include <gideon/serverbase/app/BaseSessionServerApp.h>

namespace sne { namespace server {
class ServerSideSessionManager;
}} // namespace sne { namespace server {


namespace sne { namespace base {
class TaskScheduler;
}} // namespace sne { namespace base {

namespace gideon { namespace communityserver {

class CommunityService;

/**
 * @class CommunityServerApp
 *
 * Gideon Community Server
 */
class CommunityServer_Export CommunityServerApp :
    public serverbase::BaseSessionServerApp
{
public:
    CommunityServerApp();
    virtual ~CommunityServerApp();

private: // = serverbase::BaseSessionServerApp overriding
    virtual bool onInit();
    virtual bool onStart();
    virtual void onFinish();

private: // = sne::server::SessionFactoryHelper overriding
    virtual std::unique_ptr<sne::base::SessionFactory> createSessionFactory(
        const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask);

private: // = rcon::QuerySource overriding
    virtual std::string query(const std::string& name,
        const std::string& detail) const override;

private:
    bool loadServerSpec();
    bool loadDataTables();

    bool initTaskScheduler();
    bool initCommunityService();
    bool initSessionManagerForServers();
    bool initMonitorService();

    void finishCommunityService();

private:
    sne::server::ServerSpec m2lSpec_;
    sne::server::ServerSpec s2sSpec_;

    std::unique_ptr<CommunityService> communityService_;
    std::unique_ptr<sne::server::ServerSideSessionManager>
        sessionManagerForServers_;
    std::unique_ptr<sne::base::TaskScheduler> taskScheduler_;
};

}} // namespace gideon { namespace communityserver {
