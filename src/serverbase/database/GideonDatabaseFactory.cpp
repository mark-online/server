#include "ServerBasePCH.h"
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include "mssql/SqlServerGameDatabase.h"
#include "mssql/SqlServerAccountDatabase.h"
#include "proxy/DeferrableProxyGameDatabase.h"
#include <sne/base/utility/Assert.h>

namespace gideon { namespace serverbase {

std::unique_ptr<sne::database::Database> GideonDatabaseFactory::create(ConnectionType ct)
{
    if ("login_server" == serviceName_) {
        return std::make_unique<SqlServerAccountDatabase>();
    }

    switch (ct) {
    case sne::database::DatabaseFactory::ctDirect:
        return std::make_unique<SqlServerGameDatabase>();
    case sne::database::DatabaseFactory::ctProxy:
        return std::make_unique<DeferrableProxyGameDatabase>(ioContextTask_);
    }

    SNE_ASSERT(false && "Invalid connection type");
    return nullptr;
}

}} // namespace gideon { namespace serverbase {
