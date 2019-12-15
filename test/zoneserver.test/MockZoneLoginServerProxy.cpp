#include "ZoneServerTestPCH.h"
#include "MockZoneLoginServerProxy.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>

// = zoneserver::ZoneLoginServerProxy overriding

MockZoneLoginServerProxy::MockZoneLoginServerProxy() :
	zoneserver::ZoneLoginServerProxy(sne::server::ServerInfo())
{
	accountInfoMap_.emplace(1, AccountInfo(1, L"test1", agAdmin));
	accountInfoMap_.emplace(2, AccountInfo(2, L"test2", agPlayer));
	accountInfoMap_.emplace(3, AccountInfo(3, L"test3", agPlayer));
	accountInfoMap_.emplace(4, AccountInfo(4, L"test4", agPlayer));
}


DEFINE_SRPC_METHOD_4(MockZoneLoginServerProxy, z2l_rendezvous,
    ShardId, shardId, ZoneId, zoneId, AccountIds, onlineUsers, uint16_t, maxUserCount)
{
    addCallCount("z2l_rendezvous");
    shardId, zoneId, onlineUsers, maxUserCount;
}


DEFINE_SRPC_METHOD_0(MockZoneLoginServerProxy, z2l_ready)
{
    addCallCount("z2l_ready");
}


DEFINE_SRPC_METHOD_1(MockZoneLoginServerProxy, z2l_loginZoneUser,
    Certificate, certificate)
{
    addCallCount("z2l_loginZoneUser");

    // TODO: 수정
    AccountInfo accountInfo;
    ErrorCode errorCode = ecOk;

    const AccountInfoMap::const_iterator pos = accountInfoMap_.find(certificate.accountId_);
    if (pos != accountInfoMap_.end()) {
        accountInfo = (*pos).second;
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    z2l_onLoginZoneUser(errorCode, accountInfo, certificate);
}


DEFINE_SRPC_METHOD_3(MockZoneLoginServerProxy, z2l_onLoginZoneUser,
    ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate)
{
    if (isFailed(errorCode)) {
        ZONEUSER_MANAGER->loginResponsed(errorCode, accountInfo, reissuedCertificate);
        return;
    }

    FullUserInfo userInfo;
    {
        sne::database::Guard<servertest::MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);	
	    userInfo = db->getFullUserInfo(accountInfo.accountId_);
    }

    ZONEUSER_MANAGER->userInfoResponsed(errorCode, accountInfo, userInfo);
}


DEFINE_SRPC_METHOD_1(MockZoneLoginServerProxy, z2l_reserveMigration,
    AccountId, accountId)
{
    addCallCount("z2l_reserveMigration");

    accountId;
}


DEFINE_SRPC_METHOD_1(MockZoneLoginServerProxy, z2l_logoutUser,
    AccountId, accountId)
{
    addCallCount("z2l_logoutUser");
    accountId;
    // TODO: 로그아웃
}


DEFINE_SRPC_METHOD_2(MockZoneLoginServerProxy, z2l_onGetFullUserInfo,
    ErrorCode, errorCode, FullUserInfo, userInfo)
{
    addCallCount("z2l_onGetFullUserInfo");
    errorCode, userInfo;
}
