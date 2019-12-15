#include "LoginServerTestPCH.h"
#include "LoginServiceTestFixture.h"
#include "MockLoginServerSideProxy.h"
#include "loginserver/LoginService.h"
#include "loginserver/user/LoginUserManager.h"
#include "loginserver/s2s/LoginServerSideProxy.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/s2s/ServerSideProxyManager.h>

using namespace sne;
using namespace gideon;

using servertest::MockProxyGameDatabase;

/**
* @class LoginServiceTest
*
* 로그인 서비스 테스트
*/
class LoginServiceTest : public LoginServiceTestFixture
{ 
private:
    virtual void SetUp() {
        LoginServiceTestFixture::SetUp();

        communityServer_ = createLoginServerSideProxy(server::ServerId(100), stCommunityServer);
        zoneServer1_ = createLoginServerSideProxy(server::ServerId(1), stZoneServer);
        zoneServer2_ = createLoginServerSideProxy(server::ServerId(2), stZoneServer);

        ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());
    }
    //virtual void TearDown();

protected:
    MockLoginServerSideProxy* communityServer_;
    MockLoginServerSideProxy* zoneServer1_;
    MockLoginServerSideProxy* zoneServer2_;
};


TEST_F(LoginServiceTest, testInitialize)
{
    ASSERT_TRUE(LOGIN_SERVICE != nullptr);

    {
        sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
        ASSERT_EQ(1, db->getCallCount("getProperties"));
    }
    ASSERT_EQ(3, SERVERSIDEPROXY_MANAGER->getProxyCount());
}


TEST_F(LoginServiceTest, testCommunityServerRedezvousWithUsers)
{
    const size_t userCount = 3;
    AccountIds onlineUsers;
    for (size_t i = 0; i < userCount; ++i) {
        onlineUsers.push_back(AccountId(i + 2));
    }
    communityServer_->m2l_rendezvous(onlineUsers);

    ASSERT_EQ(userCount + 1, LOGINUSER_MANAGER->getUserCount());
}


TEST_F(LoginServiceTest, testZoneServerRedezvousWithUsers)
{
    const size_t userCount = 3;
    AccountIds onlineUsers;
    for (size_t i = 0; i < userCount; ++i) {
        onlineUsers.push_back(AccountId(i + 2));
    }
    zoneServer1_->z2l_rendezvous(1, 1, onlineUsers, 10);

    ASSERT_EQ(userCount + 1, LOGINUSER_MANAGER->getUserCount());
}


TEST_F(LoginServiceTest, testLogin)
{
    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("authenticate"));
    ASSERT_EQ(1, db->getCallCount("getAccountInfo"));
    ASSERT_EQ(1, db->getCallCount("getCharacterCounts"));
}


TEST_F(LoginServiceTest, testDuplicatedLogin)
{
    AccountInfo accountInfo;
    CertificateMap certificateMap;
    ASSERT_EQ(ecLoginAlreadyLoggedIn, LOGIN_SERVICE->login(accountInfo, certificateMap, L"test1", "test1"));

    ASSERT_EQ(1, zoneServer1_->getCallCount("z2l_evUserExpelled"));

    ASSERT_EQ(1, communityServer_->getCallCount("m2l_evUserExpelled"));

    ASSERT_EQ(0, LOGINUSER_MANAGER->getUserCount());
}


TEST_F(LoginServiceTest, testLoginFromZoneServer)
{
    zoneServer1_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, zoneServer1_->getCallCount("z2l_onLoginZoneUser"));
    ASSERT_EQ(ecOk, zoneServer1_->lastErrorCode_);
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("updateLoginAt"));
}


TEST_F(LoginServiceTest, testDuplicatedLoginFromZoneServer)
{
    zoneServer1_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    zoneServer1_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    ASSERT_EQ(0, zoneServer1_->getCallCount("z2l_evUserExpelled"));
    ASSERT_EQ(ecLoginAlreadyLoggedIn, zoneServer1_->lastErrorCode_);
    ASSERT_EQ(0, zoneServer2_->getCallCount("z2l_evUserExpelled"));
    ASSERT_EQ(ecWhatDidYouTest, zoneServer2_->lastErrorCode_);
    ASSERT_EQ(0, communityServer_->getCallCount("m2l_evUserExpelled"));
    ASSERT_EQ(ecWhatDidYouTest, communityServer_->lastErrorCode_);
}


TEST_F(LoginServiceTest, testDuplicatedLoginInOtherZoneServer)
{
    zoneServer1_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    zoneServer2_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    ASSERT_EQ(0, zoneServer1_->getCallCount("z2l_evUserExpelled"));
    ASSERT_EQ(ecOk, zoneServer1_->lastErrorCode_);
    ASSERT_EQ(0, zoneServer2_->getCallCount("z2l_evUserExpelled"));
    ASSERT_EQ(ecLoginAlreadyLoggedIn, zoneServer2_->lastErrorCode_);
    ASSERT_EQ(0, communityServer_->getCallCount("m2l_evUserExpelled"));
    ASSERT_EQ(ecWhatDidYouTest, communityServer_->lastErrorCode_);
}


TEST_F(LoginServiceTest, testLogoutFromZoneServer)
{
    zoneServer1_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    zoneServer1_->z2l_logoutUser(verifiedAccountId_);
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(0, db->getCallCount("logLogout"));
}


TEST_F(LoginServiceTest, testLogout)
{
    zoneServer1_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    LOGIN_SERVICE->clientDisconnected(verifiedAccountId_);
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    zoneServer1_->z2l_logoutUser(verifiedAccountId_);
    ASSERT_EQ(0, LOGINUSER_MANAGER->getUserCount());

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("logLogout"));
}


TEST_F(LoginServiceTest, testZoneServerIsDown)
{
    zoneServer1_->z2l_loginZoneUser(*getCertificate(stZoneServer));
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    LOGIN_SERVICE->clientDisconnected(verifiedAccountId_);
    ASSERT_EQ(1, LOGINUSER_MANAGER->getUserCount());

    zoneServer1_->unbridge();
    ASSERT_EQ(0, LOGINUSER_MANAGER->getUserCount());
    ASSERT_FALSE(LOGINUSER_MANAGER->isOnline(verifiedAccountId_));

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("logLogout"));
}
