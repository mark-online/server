#include "CommunityServerTestPCH.h"
#include "MockCommunityLoginServerProxy.h"
#include "CommunityServer/user/CommunityUserManager.h"

// = communityserver::CommunityLoginServerProxy overriding

MockCommunityLoginServerProxy::MockCommunityLoginServerProxy() :
	communityserver::CommunityLoginServerProxy(sne::server::ServerInfo())
{
	accountInfoMap_.emplace(1, AccountInfo(1, L"test1", agAdmin));
	accountInfoMap_.emplace(2, AccountInfo(2, L"test2", agPlayer));
	accountInfoMap_.emplace(3, AccountInfo(3, L"test3", agPlayer));
	accountInfoMap_.emplace(4, AccountInfo(4, L"test4", agPlayer));
}


DEFINE_SRPC_METHOD_1(MockCommunityLoginServerProxy, m2l_rendezvous,
    AccountIds, onlineUsers)
{
    addCallCount("m2l_rendezvous");
    onlineUsers;
}


DEFINE_SRPC_METHOD_0(MockCommunityLoginServerProxy, m2l_ready)
{
    addCallCount("m2l_ready");
}


DEFINE_SRPC_METHOD_1(MockCommunityLoginServerProxy, m2l_loginCommunityUser,
    Certificate, certificate)
{
    addCallCount("m2l_loginCommunityUser");

    AccountInfo accountInfo;
    ErrorCode errorCode = ecOk;

    const AccountInfoMap::const_iterator pos = accountInfoMap_.find(certificate.accountId_);
    if (pos != accountInfoMap_.end()) {
        accountInfo = (*pos).second;
    }
    else {
        errorCode = ecLoginIsNotLoginUser;
    }

    m2l_onLoginCommunityUser(errorCode, accountInfo, certificate);
}


DEFINE_SRPC_METHOD_1(MockCommunityLoginServerProxy, m2l_logoutUser,
    AccountId, accountId)
{
    addCallCount("m2l_logoutUser");
    accountId;
    // TODO: 로그아웃
}
