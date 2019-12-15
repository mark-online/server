#pragma once

#include "CommunityServer/c2s/CommunityClientSessionCallback.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockCommunityClientSessionCallback
 */
class MockCommunityClientSessionCallback :
    public communityserver::CommunityClientSessionCallback,
    public sne::test::CallCounter
{
public:
    MockCommunityClientSessionCallback() :
        lastErrorCode_(ecWhatDidYouTest) {}

private:
    virtual void loginResponsed(ErrorCode errorCode, const Certificate& reissuedCertificate) {
        addCallCount("loginResponsed");
        lastErrorCode_ = errorCode;
        reissuedCertificate;
    }

    virtual void expelledFromServer() {
        addCallCount("expelledFromServer");
    }

public:
    virtual sne::base::Session* getCurrentSession() {
        return nullptr;
    }

    virtual sne::server::ClientId getCurrentClientId() const {
        return static_cast<sne::server::ClientId>(std::rand());
    }

public:
    ErrorCode lastErrorCode_;
};
