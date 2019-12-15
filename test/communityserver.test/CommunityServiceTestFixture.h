#pragma once

#include "MockCommunityClientSessionCallback.h"
#include "CommunityServer/CommunityService.h"
#include <gideon/servertest/DatabaseTestFixture.h>

using namespace sne;
using namespace gideon;

class MockCommunityLoginServerProxy;
class MockCommunityServerSideProxy;
class MockCommunityUser;

/**
* @class CommunityServiceTestFixture
*
* 커뮤니티 서비스 테스트 Fixture
*/
class CommunityServiceTestFixture : public servertest::DatabaseTestFixture
{
public:
    enum {
        /// 커뮤니티 서버에 입장 가능한 최대 인원 수
        maxCommunityServerUserCount = 10
    };

    CommunityServiceTestFixture();
    virtual ~CommunityServiceTestFixture();

protected:
    virtual void SetUp();
    virtual void TearDown();

protected:
    void setValidAccount(AccountId accountId);

    void loginUser(AccountId accountId);

    std::wstring makeId(AccountId accountId) const {
        return core::formatString(L"test%u", accountId);
    }

    std::wstring makeNickname(AccountId accountId) const {
        return makeId(accountId);
    }

    std::string makePassword(AccountId accountId) const {
        return core::formatString("test%u", accountId);
    }

    MockCommunityLoginServerProxy& getLoginServerProxy();

protected:
    MockCommunityUser* getMockCommunityUser(AccountId accountId);

private:
    MockCommunityServerSideProxy* createCommunityServerSideProxy(
        sne::server::ServerId serverId);

protected:
    communityserver::CommunityService* communityService_;

    MockCommunityServerSideProxy* zoneServer1_;
    MockCommunityServerSideProxy* zoneServer2_;

    MockCommunityClientSessionCallback sessionCallback_;
    AccountId accountId1_;
    AccountId accountId2_;
    AccountId accountId3_;
};
