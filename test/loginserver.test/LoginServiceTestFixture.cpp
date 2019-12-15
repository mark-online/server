#include "LoginServerTestPCH.h"
#include "LoginServiceTestFixture.h"
#include "MockLoginServerSideProxy.h"
#include "loginserver/s2s/LoginServerSideProxyManager.h"
#include "loginserver/user/LoginUserManager.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>

namespace
{

/**
 * @class MockLoginService
 */
class MockLoginService : public loginserver::LoginService
{
public:
    MockLoginService(const CommunityServerInfo& communityServerInfo) :
        loginserver::LoginService(communityServerInfo) {}

private:
    virtual const sne::server::S2sCertificate& getS2sCertificate() const {
        static const sne::server::S2sCertificate cert("gee");
        return cert;
    }

    virtual size_t getMaxUserCount() const {
        return 100;
    }
};

} // namespace

// = LoginServiceTestFixture

void LoginServiceTestFixture::SetUp()
{
    servertest::DatabaseTestFixture::setServiceName("login_server");
    servertest::DatabaseTestFixture::SetUp();

    ASSERT_TRUE(getProperties());

    const CommunityServerInfo communityServerInfo;
    loginService_ = new MockLoginService(communityServerInfo);
    ASSERT_EQ(true, loginService_->initialize());

    AccountInfo accountInfo;
    ASSERT_EQ(ecOk, LOGIN_SERVICE->login(accountInfo, verifiedCertificateMap_, L"test1", "test1"));
    verifiedAccountId_ = accountInfo.accountId_;
}


void LoginServiceTestFixture::TearDown()
{
    servertest::DatabaseTestFixture::TearDown();

    delete loginService_;
}


MockLoginServerSideProxy*
LoginServiceTestFixture::createLoginServerSideProxy(
    sne::server::ServerId serverId, ServerType serverType)
{
    MockLoginServerSideProxy* proxy =
        new MockLoginServerSideProxy(serverId, serverType);

    LOGINSERVERSIDEPROXY_MANAGER->add(proxy->getServerId(), proxy);

    proxy->bridge();
    return proxy;
}
