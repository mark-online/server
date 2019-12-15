#pragma once

#include "loginserver/LoginService.h"
#include <gideon/servertest/DatabaseTestFixture.h>
#include <gideon/cs/shared/data/Certificate.h>

class MockLoginServerSideProxy;

using namespace sne;
using namespace gideon;


/**
* @class LoginServiceTestFixture
*
* 로그인 서비스 테스트 Fixture
*/
class LoginServiceTestFixture : public servertest::DatabaseTestFixture
{
protected:
    virtual void SetUp();
    virtual void TearDown();

protected:
    MockLoginServerSideProxy* createLoginServerSideProxy(
        sne::server::ServerId serverId, ServerType serverType);

protected:
    const Certificate* getCertificate(ServerType st) const {
        return gideon::getCertificate(verifiedCertificateMap_, st);
    }

protected:
    loginserver::LoginService* loginService_;
    AccountId verifiedAccountId_;
    CertificateMap verifiedCertificateMap_;
};
