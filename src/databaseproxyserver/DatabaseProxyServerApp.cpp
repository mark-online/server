#include "DatabaseProxyServerPCH.h"
#include "DatabaseProxyServerApp.h"
#include "s2s/DatabaseProxyServerSideSessionFactory.h"
#include "cache/account/AccountCacheManager.h"
#include <gideon/serverbase/database/GameDatabase.h>
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include <gideon/serverbase/datatable/DataTableLoader.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/app/Factory.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/io_context/IoContextTask.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/utility/Unicode.h>

namespace gideon { namespace databaseproxyserver {

DatabaseProxyServerApp::DatabaseProxyServerApp()
{
}


DatabaseProxyServerApp::~DatabaseProxyServerApp()
{
}


bool DatabaseProxyServerApp::initTaskScheduler()
{
    // TODO: config에서 읽을 것
    const uint16_t threadCount = 2;
    auto* ioContextTask = addIoContextTask("task_scheduler", threadCount);
    if (!ioContextTask) {
        return false;
    }
    taskScheduler_ = std::make_unique<sne::base::TaskScheduler>(*ioContextTask);
    sne::base::TaskScheduler::instance(taskScheduler_.get());
    return true;
}


bool DatabaseProxyServerApp::initDatabaseManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    sne::base::IoContextTask* nullIoContextTask = nullptr;
    serverbase::GideonDatabaseFactory dbPoolFactory(*nullIoContextTask, getServiceConfig().getServiceName());

    gameDatabaseManager_ = sne::database::DatabaseManagerFactory::create(
        getServiceConfig().getConfigReader(), dbPoolFactory);
    if (! gameDatabaseManager_.get()) {
        return false;
    }

    sne::database::DatabaseManager::instance(gameDatabaseManager_.get());
    return true;
}


bool DatabaseProxyServerApp::loadProperties()
{
    sne::server::Profiler profiler(__FUNCTION__);

    sne::server::Properties properties;

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->getProperties(properties)) {
        return false;
    }

    SNE_PROPERTIES::initialize(properties);
    return true;
}


bool DatabaseProxyServerApp::loadServerSpec()
{
    sne::database::Guard<serverbase::GameDatabase> db(*gameDatabaseManager_);
    (void)db->getServerSpec(c2sSpec_, getServiceConfig().getServiceName(), "");

    return c2sSpec_.isValid();
}


bool DatabaseProxyServerApp::loadDataTables()
{
    if (!serverbase::DataTableLoader::loadEquipTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadCharacterDefaultSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadBuildingTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadHarnessTable()) {
        return false;
    }
    return true;
}


bool DatabaseProxyServerApp::initMonitorService()
{
    return true;
}


bool DatabaseProxyServerApp::initAccountCacheManager()
{
    // TODO: 별도의 IoContextTask로 분리?
    accountCacheManager_ = std::make_unique<AccountCacheManager>(
        getIoContextTask().getIoContext());
    if (isFailed(accountCacheManager_->initialize())) {
        return false;
    }

    AccountCacheManager::instance(accountCacheManager_.get());
    return true;
}

// = sne::server::SessionServerApp overriding

bool DatabaseProxyServerApp::onInit()
{
    if (! initTaskScheduler()) {
        SNE_LOG_ERROR("DatabaseProxyServerApp::initTaskScheduler() FAILED!");
        return false;
    }

    if (! initDatabaseManager()) {
        SNE_LOG_ERROR("DatabaseProxyServerApp::initDatabaseManager() FAILED!");
        return false;
    }

    if (! loadProperties()) {
        SNE_LOG_ERROR("DatabaseProxyServerApp::loadProperties() FAILED!");
        return false;
    }

    if (! loadServerSpec()) {
        SNE_LOG_ERROR("DatabaseProxyServerApp::loadServerSpec() FAILED!");
        return false;
    }

    if (! initMonitorService()) {
        SNE_LOG_ERROR("DatabaseProxyServerApp::initMonitorService() FAILED!");
        return false;
    }

    if (! sne::database::proxy::BaseDatabaseProxyServerApp::onInit()) {
        return false;
    }

    if (!loadDataTables()) {
        return false;
    }

    if (! initAccountCacheManager()) {
        SNE_LOG_ERROR("DatabaseProxyServerApp::initAccountCacheManager() FAILED!");
        return false;
    }

    return true;
}

// = SessionFactoryHelper overriding

std::unique_ptr<sne::base::SessionFactory>
DatabaseProxyServerApp::createSessionFactory(
    const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask)
{
    return std::make_unique<DatabaseProxyServerSideSessionFactory>(
        serverSpec, ioContextTask);
}

}} // namespace gideon { namespace databaseproxyserver {

DEFINE_SNE_APP_FACTORY(gideon::databaseproxyserver::DatabaseProxyServerApp)
