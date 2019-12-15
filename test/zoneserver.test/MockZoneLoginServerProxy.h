#pragma once

#include "ZoneServer/s2s/ZoneLoginServerProxy.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
* @class MockZoneLoginServerProxy
*/
class MockZoneLoginServerProxy :
    public zoneserver::ZoneLoginServerProxy,
    public sne::test::CallCounter
{
public:
	typedef core::HashMap<AccountId, AccountInfo> AccountInfoMap;
    MockZoneLoginServerProxy();

private:
    virtual bool isActivated() const {
        return true;
    }

private:
    // = zoneserver::ZoneLoginServerProxy overriding
    OVERRIDE_SRPC_METHOD_4(z2l_rendezvous,
        ShardId, shardId, ZoneId, zoneId, AccountIds, onlineUsers, uint16_t, maxUserCount);
    OVERRIDE_SRPC_METHOD_0(z2l_ready);
    OVERRIDE_SRPC_METHOD_1(z2l_loginZoneUser,
        Certificate, certificate);
	OVERRIDE_SRPC_METHOD_3(z2l_onLoginZoneUser,
		ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);
    OVERRIDE_SRPC_METHOD_1(z2l_reserveMigration,
        AccountId, accountId);
    OVERRIDE_SRPC_METHOD_1(z2l_logoutUser,
        AccountId, accountId);
    OVERRIDE_SRPC_METHOD_2(z2l_onGetFullUserInfo,
        ErrorCode, errorCode, FullUserInfo, userInfo);

private:
	AccountInfoMap accountInfoMap_;
};
