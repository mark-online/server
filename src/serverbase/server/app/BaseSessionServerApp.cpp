#include "ServerBasePCH.h"
#include <gideon/serverbase/app/BaseSessionServerApp.h>
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/server/data/ServerAddress.h>
#include <gideon/cs/datatable/DataTableFactory.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/server/common/Property.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace serverbase {

BaseSessionServerApp::BaseSessionServerApp()
{
}


BaseSessionServerApp::~BaseSessionServerApp()
{
}


bool BaseSessionServerApp::initDatabaseManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    const size_t s2dWorkerThreadCount = 2;
    if (! addIoContextTask(getS2dIoServiceName(),
        s2dWorkerThreadCount)) {
        SNE_LOG_ERROR("BaseSessionServerApp.addIoContextTask(%s) FAILED!",
            getS2dIoServiceName().c_str());
        return false;
    }

    GideonDatabaseFactory databaseFactory(
        getIoContextTask(getS2dIoServiceName()), getS2dIoServiceName());

    databaseManager_.reset(
        sne::database::DatabaseManagerFactory::create(
            getServiceConfig().getConfigReader(), databaseFactory).release());

    if (! databaseManager_.get()) {
        return false;
    }

    sne::database::DatabaseManager::instance(databaseManager_.get());
    return true;
}


bool BaseSessionServerApp::loadServerSpec()
{
    sne::server::Profiler profiler(__FUNCTION__);

    c2sSpec_ = getServerSpec(getServiceConfig().getServiceName(), "");
    //s2sSpec_ = getServerSpec(getServiceConfig().getServiceName(), ".s2s");
    return true;
}


bool BaseSessionServerApp::loadProperties()
{
    sne::server::Profiler profiler(__FUNCTION__);

    sne::database::Guard<ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getProperties();
    if ((! future) || (! future->waitForDone())) {
        return false;
    }

    const GetPropertiesRequestFuture& requestFuture =
        static_cast<const GetPropertiesRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        return false;
    }

    SNE_PROPERTIES::initialize(requestFuture.properties_);
    return true;
}


sne::server::ServerSpec BaseSessionServerApp::getServerSpec(
    const std::string& serverName, const std::string& suffix) const
{
    sne::database::Guard<ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->getServerSpec(serverName, suffix);
    if ((! future) || (! future->waitForDone())) {
        return sne::server::ServerSpec();
    }

    const GetServerSpecRequestFuture& requestFuture =
        static_cast<const GetServerSpecRequestFuture&>(*future);
    if (isFailed(requestFuture.errorCode_)) {
        return sne::server::ServerSpec();
    }

    return requestFuture.serverSpec_;
}


// = sne::server::SessionServerApp overriding

bool BaseSessionServerApp::onInit()
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! initDatabaseManager()) {
        SNE_LOG_ERROR("BaseSessionServerApp::initDatabaseManager() FAILED!");
        return false;
    }

    if (! loadProperties()) {
        SNE_LOG_ERROR("BaseSessionServerApp::loadProperties() FAILED!");
        return false;
    }

    if (! loadServerSpec()) {
        SNE_LOG_ERROR("BaseSessionServerApp::loadServerSpec() FAILED!");
        return false;
    }

    if (! sne::server::SessionServerApp::onInit()) {
        return false;
    }

    return true;
}

}} // namespace gideon { namespace serverbase {
