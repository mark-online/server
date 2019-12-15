#pragma once

#include "AccountDatabase.h"
#include "GameDatabase.h"
#include <gideon/Common.h>
#include <sne/database/DatabaseFactory.h>

namespace sne { namespace base {
class IoContextTask;
}} // namespace sne { namespace base {

namespace gideon { namespace serverbase {

/**
 * @struct DatabaseFactory
 *
 * AccountDatabase 객체를 만든다.
 */
class GIDEON_SERVER_API GideonDatabaseFactory : public sne::database::DatabaseFactory
{
public:
    GideonDatabaseFactory(sne::base::IoContextTask& ioContextTask, const std::string& serviceName) :
        ioContextTask_(ioContextTask),
		serviceName_(serviceName) {}

    virtual std::unique_ptr<sne::database::Database> create(ConnectionType ct);

private:
    sne::base::IoContextTask& ioContextTask_;
	const std::string serviceName_;
};

}} // namespace gideon { namespace serverbase {
