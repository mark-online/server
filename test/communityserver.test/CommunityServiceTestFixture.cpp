#include "CommunityServerTestPCH.h"
#include "CommunityServiceTestFixture.h"
#include "MockCommunityLoginServerProxy.h"
#include "MockCommunityServerSideProxy.h"
#include "MockCommunityUser.h"
#include "CommunityServer/s2s/CommunityServerSideProxyManager.h"
#include "CommunityServer/user/CommunityUserManager.h"
#include "CommunityServer/user/CommunityUserAllocator.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/MockPropertyTable.h>
#include <gideon/servertest/datatable/MockGuildLevelTable.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <sne/database/DatabaseManager.h>

using namespace communityserver;

namespace
{

servertest::MockPropertyTable s_mockPropertyTable;
servertest::MockGuildLevelTable s_mockGuildLevelTable;


/**
 * @class MockCommunityUserAllocator
 */
class MockCommunityUserAllocator : public CommunityUserAllocator
{
public:
    CommunityUser* malloc() {
        return new MockCommunityUser;
    }

    void free(CommunityUser* user) {
        boost::checked_delete(user);
    }
};


/**
 * @class MockCommunityService
 */
class MockCommunityService :
    public CommunityService
{
public:
    MockCommunityService() :
        mockLoginServerProxy_(nullptr) {}

    MockCommunityLoginServerProxy& getLoginServerProxy() {
        return *mockLoginServerProxy_;
    }

private:
    virtual const sne::server::S2sCertificate& getServerCertificate() const override {
        static const sne::server::S2sCertificate cert("gee");
        return cert;
    }

    virtual size_t getMaxUserCount() const override {
        return CommunityServiceTestFixture::maxCommunityServerUserCount;
    }

    virtual std::unique_ptr<CommunityUserAllocator>
        createCommunityUserAllocator() const override {
            return std::make_unique<MockCommunityUserAllocator>();
    }

    virtual std::unique_ptr<CommunityLoginServerProxy>
        createLoginServerProxy() const override {
        auto proxy = std::make_unique<MockCommunityLoginServerProxy>();
        (void)proxy->connect();
        mockLoginServerProxy_ = proxy.get();
        return proxy;
    }

private:
    mutable MockCommunityLoginServerProxy* mockLoginServerProxy_;
};

} // namespace

// = CommunityServiceTestFixture

CommunityServiceTestFixture::CommunityServiceTestFixture() :
    communityService_(nullptr)
{
}


CommunityServiceTestFixture::~CommunityServiceTestFixture()
{
}


void CommunityServiceTestFixture::SetUp()
{
    DatabaseTestFixture::SetUp();

    datatable::PropertyTable::instance(&s_mockPropertyTable);
    datatable::GuildLevelTable::instance(&s_mockGuildLevelTable);

    ASSERT_TRUE(getProperties());

    communityService_ = new MockCommunityService;
    ASSERT_EQ(true, communityService_->initialize("community"));
    CommunityService::instance(communityService_);

    zoneServer1_ = createCommunityServerSideProxy(sne::server::ServerId(1));
    zoneServer2_ = createCommunityServerSideProxy(sne::server::ServerId(2));

    accountId1_ = 1;
    setValidAccount(accountId1_);
    accountId2_ = 2;
    setValidAccount(accountId2_);
    accountId3_ = 3;
    setValidAccount(accountId3_);
}


void CommunityServiceTestFixture::TearDown()
{
    DatabaseTestFixture::TearDown();

    delete communityService_;
}


void CommunityServiceTestFixture::setValidAccount(AccountId accountId)
{
    sne::database::Guard<servertest::MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    db->setValidAccount(accountId, makeId(accountId), makePassword(accountId));
}


void CommunityServiceTestFixture::loginUser(AccountId accountId)
{
    const Certificate certificate = Certificate::issue(stCommunityServer, accountId, 1);
    COMMUNITY_SERVICE->login(certificate, sessionCallback_);
}


MockCommunityLoginServerProxy& CommunityServiceTestFixture::getLoginServerProxy()
{
    return static_cast<MockCommunityService*>(communityService_)->getLoginServerProxy();
}


MockCommunityUser* CommunityServiceTestFixture::getMockCommunityUser(AccountId accountId)
{
    return static_cast<MockCommunityUser*>(COMMUNITYUSER_MANAGER->getUser(accountId));
}


MockCommunityServerSideProxy*
CommunityServiceTestFixture::createCommunityServerSideProxy(sne::server::ServerId serverId)
{
    MockCommunityServerSideProxy* proxy = new MockCommunityServerSideProxy(serverId);

    COMMUNITYSERVERSIDEPROXY_MANAGER->add(proxy->getServerId(), proxy);

    const ZoneId zoneId = ZoneId(proxy->getServerId());
    proxy->z2m_rendezvous(zoneId);
    proxy->z2m_ready();

    return proxy;
}
