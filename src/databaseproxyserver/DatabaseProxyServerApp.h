#pragma once

#include "databaseproxyserver_export.h"
#include <sne/database/proxy/server/BaseDatabaseProxyServerApp.h>
#include <sne/server/common/ServerSpec.h>

namespace sne { namespace base {
class TaskScheduler;
}} // namespace sne { namespace base {

namespace sne { namespace database {
class DatabaseManager;
}} // namespace sne { namespace database {

namespace gideon { namespace databaseproxyserver {

class AccountCacheManager;

/**
 * @class DatabaseProxyServerApp
 *
 * Gideon DatabaseClientSideProxy Server
 */
class DatabaseProxyServer_Export DatabaseProxyServerApp :
    public sne::database::proxy::BaseDatabaseProxyServerApp
{
public:
    DatabaseProxyServerApp();
    virtual ~DatabaseProxyServerApp();

private:
    // = sne::server::SessionServerApp overriding
    virtual bool onInit();

    virtual const sne::server::ServerSpec& getServerSpec() const {
        return c2sSpec_;
    }

    // = sne::server::SessionFactoryHelper overriding
    virtual std::unique_ptr<sne::base::SessionFactory> createSessionFactory(
        const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask);

private:
    bool loadProperties();
    bool loadServerSpec();
    bool loadDataTables();

    bool initTaskScheduler();
    bool initDatabaseManager();
    bool initMonitorService();
    bool initAccountCacheManager();

private:
    std::unique_ptr<sne::database::DatabaseManager> gameDatabaseManager_;
    std::unique_ptr<AccountCacheManager> accountCacheManager_;

    sne::server::ServerSpec c2sSpec_;

    std::unique_ptr<sne::base::TaskScheduler> taskScheduler_;
};

}} // namespace gideon { namespace databaseproxyserver {
