#include "AccountDatabaseTestPCH.h"
#include <gideon/serverbase/database/AccountDatabase.h>
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/database/ado/AdoDatabase.h>
#include <sne/server/config/ConfigReader.h>
#include <sne/core/utility/Win32Util.h>
#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4127 4512)
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#  pragma warning (pop)
#endif

using namespace sne;
using namespace gideon;
using namespace gideon::serverbase;

const char connectionString[] =
    "Provider=sqloledb;"
    //"Data Source=gideon_db_server;"
    "Data Source=localhost\\SQLEXPRESS;"
    "Initial Catalog=Gideon_account_test;"
    "User Id=Gideon_account_test;"
    "Password=Gideon_account_test;" 
    "Trusted_Connection=False;";

/**
 * @class MockConfigReader
 */
class MockConfigReader : public server::ConfigReader
{
private:
    virtual bool open(const std::string& filename) {
        filename;
        return true;
    }

    virtual std::string getString(const char* sectionName,
        const char* keyName) {
        if (std::string("database") == sectionName) {
            if (std::string("connection-type") == keyName) {
                return "direct";
            }
            if (std::string("connection-count") == keyName) {
                return "1";
            }
            if (std::string("connection-timeout") == keyName) {
                return "3";
            }
            if (std::string("connection-string") == keyName) {
                return connectionString;
            }
        }
        return "";
    }
};

/**
* @class SqlServerAccountDatabaseTest
*
* SqlServer GameDatabase 테스트
* - database/migrate/sql/test/create_database_and_user.sql을 실행해야 한다
*/
class SqlServerAccountDatabaseTest : public testing::Test
{ 
public:
    SqlServerAccountDatabaseTest() :
        db_(0) {}

    virtual void SetUp();
    virtual void TearDown();

    void addItem(ObjectId itemId, SlotId slotId);
private:
    void resetTables();

    
protected:
    core::InitOle* initOle_;

    database::DatabaseManager* accountDatabaseManager_;
    database::AdoDatabase* db_;
public:
    ObjectId accountId_;
    ObjectId characterId_;
};


void SqlServerAccountDatabaseTest::SetUp()
{
    initOle_ = new core::InitOle();

    MockConfigReader configReader;
    base::IoContextTask* nullIoContextTask = nullptr;
    GideonDatabaseFactory databaseFactory(*nullIoContextTask, "login_server");

    accountDatabaseManager_ =
        database::DatabaseManagerFactory::create(
            configReader, databaseFactory).release();

    ASSERT_TRUE(0 != accountDatabaseManager_);

    db_ = new database::AdoDatabase;
    ASSERT_EQ(true,
        db_->open(connectionString));

    resetTables();
}


void SqlServerAccountDatabaseTest::TearDown()
{
    if (accountDatabaseManager_ != nullptr) {
        delete accountDatabaseManager_;
    }

    if (db_ != nullptr) {
        db_->close();
        delete db_;
    }

    delete initOle_;
}


void SqlServerAccountDatabaseTest::resetTables()
{
    try {
        db_->execute("DELETE FROM Zones");
        db_->execute("DELETE FROM Accounts");
        db_->execute("DELETE FROM Shards");
        db_->execute("DELETE FROM Server_Specs");
        db_->execute("DELETE FROM Hosts");        

        db_->execute("INSERT INTO Hosts (name) "
            "VALUES ('host')");

        db_->execute("INSERT INTO Server_Specs "
            "(name, host_name, listening_address,"
            "listening_port, monitor_port,"
            "worker_thread_count,session_pool_size, max_user_count,"
            "packet_cipher,cipher_key_time_limit,"
            "authentication_timeout, heartbeat_timeout,"
            "max_bytes_per_second, seconds_for_throttling,"
            "max_pendable_packet_count, certificate) VALUES "
            "('test', 'host', 'localhost',"
            "0, 0, 0, 0, 0, 'cip', 0, 0, 0, 0, 0, 0, 'cert')");

        db_->execute("INSERT INTO Shards"
            "(id, name) VALUES "
            "(1, N'test')");

        db_->execute("INSERT INTO Zones "
            "(id, name, global_world_map_code, is_first_zone, enabled, shard_id) VALUES "
            "(1, 'test', 1, 1, 1, 1)");

        //database::Guard<AccountDatabase> db(accountDatabaseManager_);
        //AccountId accountId;
        //ASSERT_EQ(ecOk, db->createAccount(accountId, L"test1", L"test1", "test1.email.com"));
        //ASSERT_EQ(ecOk, db->createAccount(accountId, L"test2", L"test2", "test2.email.com"));
        //ASSERT_EQ(ecOk, db->createAccount(accountId, L"test3", L"test3", "test3.email.com"));
    }
    catch (const database::AdoException& /*e*/) {
        ASSERT_TRUE(false) << db_->getLastError();
    }
}


TEST_F(SqlServerAccountDatabaseTest, testGetProperties)
{
    server::Properties props;
    database::Guard<AccountDatabase> db(accountDatabaseManager_);

    ASSERT_EQ(true, db->getProperties(props));
    ASSERT_TRUE(0 < props.size());
}


TEST_F(SqlServerAccountDatabaseTest, testGetServerSpec)
{
    database::Guard<AccountDatabase> db(accountDatabaseManager_);

    server::ServerSpec spec;
    ASSERT_EQ(true, db->getServerSpec(spec, "test", ""));
    ASSERT_TRUE(! spec.listeningAddresses_.empty());
    ASSERT_EQ("localhost", spec.listeningAddresses_[0]);
}


//TEST_F(SqlServerAccountDatabaseTest, testGetAccountInfo)
//{
//    database::Guard<AccountDatabase> db(accountDatabaseManager_);
//
//    AccountInfo accountInfo;
//	AccountId accountId = invalidAccountId;
//    ASSERT_TRUE(db->authenticate(accountId, L"test1", L"test1"));
//
//	ASSERT_TRUE(db->getAccountInfo(accountInfo, accountId));
//
//	ASSERT_TRUE(accountInfo.isValid());
//}
//
//
//TEST_F(SqlServerAccountDatabaseTest, testGetCharacterCounts)
//{
//    database::Guard<AccountDatabase> db(accountDatabaseManager_);
//
//    AccountId accountId = invalidAccountId;
//    ASSERT_TRUE(db->authenticate(accountId, L"test1", L"test1"));
//
//    CharacterCountPerShardMap characterCounts;
//    ASSERT_TRUE(db->getCharacterCounts(characterCounts, accountId));
//}
//
//
//TEST_F(SqlServerAccountDatabaseTest, testUpdateCharacterCount)
//{
//    database::Guard<AccountDatabase> db(accountDatabaseManager_);
//
//    AccountId accountId = invalidAccountId;
//    ASSERT_TRUE(db->authenticate(accountId, L"test1", L"test1"));
//
//    ASSERT_TRUE(db->updateCharacterCount(accountId, 1, 1));
//}
