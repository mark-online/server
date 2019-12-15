#include "CommunityServerTestPCH.h"
#include "CommunityServiceTestFixture.h"
#include "MockCommunityLoginServerProxy.h"
#include "MockCommunityServerSideProxy.h"
#include "MockCommunityUser.h"
#include "CommunityServer/CommunityService.h"
#include "CommunityServer/user/CommunityUserManager.h"
#include "CommunityServer/user/CommunityUser.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <sne/database/DatabaseManager.h>

/**
* @class CommunityServiceTest
*
* Community 서비스 테스트
*/
class CommunityServiceTest : public CommunityServiceTestFixture
{ 
private:
    virtual void SetUp() {
        CommunityServiceTestFixture::SetUp();
    }
};


TEST_F(CommunityServiceTest, testInitialize)
{
    ASSERT_TRUE(COMMUNITY_SERVICE != 0);

    sne::database::Guard<servertest::MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("getProperties"));
}


TEST_F(CommunityServiceTest, testLoginUserWithoutCharacter)
{
    loginUser(accountId1_);

    ASSERT_EQ(1, getLoginServerProxy().getCallCount("m2l_loginCommunityUser"));
    ASSERT_EQ(1, sessionCallback_.getCallCount("loginResponsed"));
    ASSERT_EQ(ecOk, sessionCallback_.lastErrorCode_);

    ASSERT_EQ(1, COMMUNITYUSER_MANAGER->getUserCount());
}


TEST_F(CommunityServiceTest, testDuplicatedUserLoggedIn)
{
    loginUser(accountId1_);
    loginUser(accountId1_);

    ASSERT_EQ(2, getLoginServerProxy().getCallCount("m2l_loginCommunityUser"));
    ASSERT_EQ(2, sessionCallback_.getCallCount("loginResponsed"));
    ASSERT_EQ(ecLoginAlreadyLoggedIn, sessionCallback_.lastErrorCode_);

    ASSERT_EQ(0, sessionCallback_.getCallCount("expelledFromServer"));

    ASSERT_EQ(1, COMMUNITYUSER_MANAGER->getUserCount());
}


TEST_F(CommunityServiceTest, testLogout)
{
    loginUser(accountId1_);

    COMMUNITYUSER_MANAGER->logout(accountId1_);
    ASSERT_EQ(1, getLoginServerProxy().getCallCount("m2l_logoutUser"));
    ASSERT_EQ(0, COMMUNITYUSER_MANAGER->getUserCount());
}


TEST_F(CommunityServiceTest, testExpelledFromLoginServer)
{
    loginUser(accountId1_);

    getLoginServerProxy().m2l_evUserExpelled(accountId1_, erDuplicatedLogin);

    ASSERT_EQ(0, getLoginServerProxy().getCallCount("m2l_logoutUser"));
    ASSERT_EQ(0, COMMUNITYUSER_MANAGER->getUserCount());
}
