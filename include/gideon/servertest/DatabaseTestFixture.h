#pragma once

#include <gideon/Common.h>
#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4127 4512)
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#  pragma warning (pop)
#endif

namespace sne { namespace database {
class DatabaseManager;
}} // namespace sne { namespace database {

namespace gideon { namespace servertest {

/**
 * @class DatabaseTestFixture
 *
 * 데이터베이스 관련 테스트를 위한 TestFixture
 */
class GIDEON_SERVER_API DatabaseTestFixture : public testing::Test
{
protected:
    virtual void SetUp();
    virtual void TearDown();

	void setServiceName(const std::string& serviceName = "zone_server");

protected:
    bool getProperties();

private:
    sne::database::DatabaseManager* databaseManager_;
	std::string serviceName_;
};

}} // namespace gideon { namespace servertest {
