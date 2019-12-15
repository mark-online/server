#pragma once

#include "zoneserver_export.h"
#include <gideon/serverbase/app/BaseSessionServerApp.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace sne { namespace server {
class ServerSideSessionManager;
}} // namespace sne { namespace server {


namespace sne { namespace base {
class TaskScheduler;
}} // namespace sne { namespace base {

namespace gideon { namespace zoneserver {

class ZoneService;
class GameTimer;

/**
 * @class ZoneServerApp
 *
 * Gideon Zone Server
 */
class ZoneServer_Export ZoneServerApp :
    public serverbase::BaseSessionServerApp
{
public:
    ZoneServerApp();
    virtual ~ZoneServerApp();

private: // = serverbase::BaseSessionServerApp overriding
    virtual bool onInit();
    virtual bool onStart();
    virtual void onFinish();

private: // = sne::server::SessionFactoryHelper overriding
    virtual std::unique_ptr<sne::base::SessionFactory> createSessionFactory(
        const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask);

private: // = sne::server::rcon::QuerySource overriding
    virtual std::string query(const std::string& name,
        const std::string& detail) const override;

private:
    bool loadServerSpec();
    bool loadDataTables();

    bool initZoneService();
    bool initMonitorService();
    void initGameTimer();
    bool initTaskScheduler();
	bool initSessionManagerForServers();

    void finishZoneService();

private:
    sne::server::ServerSpec z2lSpec_;
    sne::server::ServerSpec z2mSpec_;
	sne::server::ServerSpec z2aSpec_; // 일반 존서버 사용 격투장 사용 안함
	sne::server::ServerSpec s2sSpec_; // 일반 존서버 사용 안함, 격투장 사용

    std::unique_ptr<ZoneService> zoneService_;
    std::unique_ptr<GameTimer> gameTimer_;
    std::unique_ptr<sne::base::TaskScheduler> taskScheduler_;
    std::unique_ptr<sne::server::ServerSideSessionManager> sessionManagerForServers_;
};

}} // namespace gideon { namespace zoneserver {
