#pragma once

#include "ZoneServer/c2s/ZoneClientSessionCallback.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockZoneClientSessionCallback
 */
class MockZoneClientSessionCallback :
    public zoneserver::ZoneClientSessionCallback,
    public sne::test::CallCounter
{
public:
    MockZoneClientSessionCallback() :
        lastErrorCode_(ecWhatDidYouTest) {}

private:
    virtual void loginResponsed(ErrorCode errorCode,
        const Certificate& reissuedCertificate) {
        addCallCount("loginResponsed");
        lastErrorCode_ = errorCode;
        reissuedCertificate;
    }

    virtual void expelledFromServer() {
        addCallCount("expelledFromServer");
    }
    
    virtual void reserveMigration() {
        addCallCount("reserveMigration");
    }
public:
    virtual sne::base::Session* getCurrentSession() {
        return nullptr;
    }

    virtual sne::server::ClientId getCurrentClientId() const {
        return static_cast<sne::server::ClientId>(std::rand());
    }

    virtual bool isLogoutReserved() const {
        return false;
    }

public:
    ErrorCode lastErrorCode_;
};
