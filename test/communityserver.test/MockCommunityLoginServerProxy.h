#pragma once

#include "CommunityServer/s2s/CommunityLoginServerProxy.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
* @class MockCommunityLoginServerProxy
*/
class MockCommunityLoginServerProxy :
    public communityserver::CommunityLoginServerProxy,
    public sne::test::CallCounter
{
public:
	typedef core::HashMap<AccountId, AccountInfo> AccountInfoMap;
    MockCommunityLoginServerProxy();

private:
    virtual bool isActivated() const {
        return true;
    }

private:
    // = communityserver::CommunityLoginServerProxy overriding
    OVERRIDE_SRPC_METHOD_1(m2l_rendezvous,
        AccountIds, onlineUsers);
    OVERRIDE_SRPC_METHOD_0(m2l_ready);
    OVERRIDE_SRPC_METHOD_1(m2l_loginCommunityUser,
        Certificate, certificate);
    OVERRIDE_SRPC_METHOD_1(m2l_logoutUser,
        AccountId, accountId);

private:
	AccountInfoMap accountInfoMap_;

};
