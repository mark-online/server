#pragma once

#include "loginserver_export.h"
#include <sne/server/app/SessionServerApp.h>
#include <sne/server/common/ServerSpec.h>

namespace sne { namespace base {
class TaskScheduler;
}} // namespace sne { namespace base {

namespace sne { namespace database {
class DatabaseManager;
}} // namespace sne { namespace database {

namespace sne { namespace server {
class DatabaseManager;
class ServerSideSessionManager;
}} // namespace sne { namespace server {

namespace gideon { namespace loginserver {

class LoginService;

/**
 * @class LoginServerApp
 *
 * Gideon Login Server
 */
class LoginServer_Export LoginServerApp :
    public sne::server::SessionServerApp
{
public:
    LoginServerApp();
    virtual ~LoginServerApp();

private: // = serverbase::BaseSessionServerApp overriding
    virtual bool onInit();
    virtual void onFinish();

private: // = sne::server::SessionFactoryHelper overriding
    virtual std::unique_ptr<sne::base::SessionFactory> createSessionFactory(
        const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask);

private: // = sne::base::SessionValidator overriding
    virtual bool isValidated() const;

private: // = sne::server::rcon::QuerySource overriding
    virtual std::string query(const std::string& name,
        const std::string& detail) const override;

private:
    virtual const sne::server::ServerSpec& getServerSpec() const {
        return c2sSpec_;
    }

private:
    bool initDatabaseManager();
    bool loadProperties();
    bool loadServerSpec();
    bool initMonitorService();
    bool initTaskScheduler();
    bool initSessionManagerForServers();

    bool initLoginService();
    void finishLoginService();

private:
    sne::server::ServerSpec s2sSpec_;
    sne::server::ServerSpec c2sSpec_;
    sne::server::ServerSpec c2mSpec_;

    std::unique_ptr<sne::database::DatabaseManager> databaseManager_;
    std::unique_ptr<LoginService> loginService_;
    std::unique_ptr<sne::server::ServerSideSessionManager> sessionManagerForServers_;

    std::unique_ptr<sne::base::TaskScheduler> taskScheduler_;
};

}} // namespace gideon { namespace loginserver {
