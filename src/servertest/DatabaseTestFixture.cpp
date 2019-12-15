#include "ServerTestPCH.h"
#include <gideon/servertest/DatabaseTestFixture.h>
#include <gideon/servertest/MockProxyGameDatabase.h>
#include "MockAccountDatabase.h"
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>
#include <sne/database/DatabaseFactory.h>
#include <sne/server/config/ConfigReader.h>
#include <sne/server/common/Property.h>

namespace gideon { namespace servertest {

namespace
{

/**
 * @class MockDatabaseManager
 */
class MockDatabaseManager : public sne::database::DatabaseManager
{
public:
    MockDatabaseManager(const std::string& serviceName) :
        isAquire_(false)
    {
        if ("login_server" == serviceName) {
            database_ = std::make_unique<MockAccountDatabase>();
        }
        else {
            database_ = std::make_unique<MockProxyGameDatabase>();
        }
    }

private:
    virtual bool initialize(sne::server::ConfigReader& configReader,
        sne::database::DatabaseFactory& databaseFactory, msec_t /*pingInterval*/) {
        configReader, databaseFactory;
        return true;
    }

    virtual sne::database::Database* acquire() {
        if (isAquire_) {
            return nullptr;
        }
        isAquire_ = true;
        return database_.get();
    }

    virtual void release(sne::database::Database* /*db*/) {
        isAquire_ = false;
    }        

    virtual void enumerate(sne::database::DatabaseTask& task) {
        task.run(*database_);
    }

private:
    std::unique_ptr<sne::database::Database> database_;
    bool isAquire_;
};

} // namespace

void DatabaseTestFixture::SetUp()
{
    databaseManager_ = new MockDatabaseManager(serviceName_);
    sne::database::DatabaseManager::instance(databaseManager_);
}


void DatabaseTestFixture::TearDown()
{
    delete databaseManager_;
    databaseManager_ = nullptr;
}


void DatabaseTestFixture::setServiceName(const std::string& serviceName)
{
    serviceName_ = serviceName;
}


bool DatabaseTestFixture::getProperties()
{
    if ("login_server" == serviceName_) {		
        sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);
        server::Properties properties;
        db->getProperties(properties);

        SNE_PROPERTIES::initialize(properties);
        return true;
    }
    else {
        sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

        sne::base::Future::Ref future = db->getProperties();
        if ((! future) || (! future->waitForDone())) {
            return false;
        }

        const serverbase::GetPropertiesRequestFuture& requestFuture =
            static_cast<const serverbase::GetPropertiesRequestFuture&>(*future);
        if (isFailed(requestFuture.errorCode_)) {
            return false;
        }

        SNE_PROPERTIES::initialize(requestFuture.properties_);
    }
    return true;
}

}} // namespace gideon { namespace servertest {
