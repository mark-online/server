#include "ZoneServerTestPCH.h"
#include "ZoneServiceTestFixture.h"
#include "MockZoneLoginServerProxy.h"
#include "MockZoneUser.h"
#include "MockPlayerController.h"
#include "ZoneServer/ZoneService.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include "ZoneServer/user/ZoneUser.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <sne/database/DatabaseManager.h>

/**
* @class ZoneServiceTest
*
* Zone 서비스 테스트
*/
class ZoneServiceTest : public ZoneServiceTestFixture
{ 
private:
    virtual void SetUp() {
        ZoneServiceTestFixture::SetUp();
    }
};


TEST_F(ZoneServiceTest, testInitialize)
{
    ASSERT_TRUE(ZONE_SERVICE != 0);

    sne::database::Guard<servertest::MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("getProperties"));
    ASSERT_EQ(1, db->getCallCount("getWorldTime"));
}


TEST_F(ZoneServiceTest, testLoginUserWithoutCharacter)
{
    const AccountId accountId2 = accountId1_ + 1;
    setValidAccount(accountId2);

    loginUser(accountId2);

    ASSERT_EQ(1, getLoginServerProxy().getCallCount("z2l_loginZoneUser"));
    ASSERT_EQ(1, sessionCallback_.getCallCount("loginResponsed"));
    ASSERT_EQ(ecCharacterNotFound, sessionCallback_.lastErrorCode_);

    ASSERT_EQ(0, ZONEUSER_MANAGER->getUserCount());

    zoneserver::ZoneUser* user2 = ZONEUSER_MANAGER->getUser(accountId2);
    ASSERT_TRUE(! user2);
}


TEST_F(ZoneServiceTest, testLoginUserWithCharacter)
{
    const AccountId accountId2 = accountId1_ + 1;
    setValidAccount(accountId2);

    createCharacter(accountId2);
    loginUser(accountId2);

    ASSERT_EQ(1, getLoginServerProxy().getCallCount("z2l_loginZoneUser"));
    ASSERT_EQ(1, sessionCallback_.getCallCount("loginResponsed"));
    ASSERT_EQ(ecOk, sessionCallback_.lastErrorCode_);

    ASSERT_EQ(1, ZONEUSER_MANAGER->getUserCount());

    zoneserver::ZoneUser* user2 = ZONEUSER_MANAGER->getUser(accountId2);
    ASSERT_TRUE(user2->hasCharacter());
}


TEST_F(ZoneServiceTest, testDuplicatedUserLoggedIn)
{
    createCharacter(accountId1_);
    loginUser(accountId1_);
    loginUser(accountId1_);

    ASSERT_EQ(2, getLoginServerProxy().getCallCount("z2l_loginZoneUser"));
    ASSERT_EQ(2, sessionCallback_.getCallCount("loginResponsed"));
    ASSERT_EQ(ecLoginAlreadyLoggedIn, sessionCallback_.lastErrorCode_);

    ASSERT_EQ(0, sessionCallback_.getCallCount("expelledFromServer"));

    ASSERT_EQ(1, ZONEUSER_MANAGER->getUserCount());
}


TEST_F(ZoneServiceTest, testLogout)
{
    createCharacter(accountId1_);
    loginUser(accountId1_);

    ZONEUSER_MANAGER->logout(accountId1_);
    ASSERT_EQ(1, getLoginServerProxy().getCallCount("z2l_logoutUser"));
    ASSERT_EQ(0, ZONEUSER_MANAGER->getUserCount());
}


TEST_F(ZoneServiceTest, testExpelledFromLoginServer)
{
    createCharacter(accountId1_);
    loginUser(accountId1_);

    getLoginServerProxy().z2l_evUserExpelled(accountId1_, erDuplicatedLogin);

    ASSERT_EQ(0, getLoginServerProxy().getCallCount("z2l_logoutUser"));
    ASSERT_EQ(0, ZONEUSER_MANAGER->getUserCount());
}
