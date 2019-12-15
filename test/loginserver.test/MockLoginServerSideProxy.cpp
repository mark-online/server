#include "LoginServerTestPCH.h"
#include "MockLoginServerSideProxy.h"

MockLoginServerSideProxy::MockLoginServerSideProxy(
    sne::server::ServerId serverId, ServerType serverType) :
    loginserver::LoginServerSideProxy(nullptr),
    lastErrorCode_(ecWhatDidYouTest),
    lastExpelReason_(erUnknown)
{
    setServerId(serverId);

    if (isZoneServer(serverType)) {
        z2l_rendezvous(1, 1, AccountIds(), 10);
    }
    else if (isCommunityServer(serverType)) {
        m2l_rendezvous(AccountIds());
    }
}


DEFINE_SRPC_METHOD_0(MockLoginServerSideProxy, z2l_onRendezvous)
{
    EXPECT_TRUE(isZoneServerProxy());

    addCallCount("z2l_onRendezvous");
}


DEFINE_SRPC_METHOD_3(MockLoginServerSideProxy, z2l_onLoginZoneUser,
    ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate)
{
    EXPECT_TRUE(isZoneServerProxy());

    accountInfo;
    addCallCount("z2l_onLoginZoneUser");
    lastErrorCode_ = errorCode;
    lastIssuedCertificate_ = reissuedCertificate;
}


DEFINE_SRPC_METHOD_1(MockLoginServerSideProxy, z2l_getFullUserInfo,
    AccountId, accountId)
{
    EXPECT_TRUE(isZoneServerProxy());

    accountId;
    addCallCount("z2l_getFullUserInfo");
}


DEFINE_SRPC_METHOD_1(MockLoginServerSideProxy, z2l_createCharacter,
    CreateCharacterInfo, createCharacterInfo)
{
    EXPECT_TRUE(isZoneServerProxy());

    createCharacterInfo;
    addCallCount("z2l_createCharacter");
}


DEFINE_SRPC_METHOD_2(MockLoginServerSideProxy, z2l_deleteCharacter,
    AccountId, accountId, ObjectId, characterId)
{
    EXPECT_TRUE(isZoneServerProxy());

    accountId, characterId;
    addCallCount("z2l_deleteCharacter");
}


DEFINE_SRPC_METHOD_2(MockLoginServerSideProxy, z2l_checkDuplicateNickname,
    AccountId, accountId, Nickname, nickname)
{
    EXPECT_TRUE(isZoneServerProxy());

    accountId, nickname;
    addCallCount("z2l_checkDuplicateNickname");
}


DEFINE_SRPC_METHOD_2(MockLoginServerSideProxy, z2l_evUserExpelled,
    AccountId, accountId, ExpelReason, expelReason)
{
    EXPECT_TRUE(isZoneServerProxy());

    accountId;
    addCallCount("z2l_evUserExpelled");
    lastExpelReason_ = expelReason;
}


DEFINE_SRPC_METHOD_3(MockLoginServerSideProxy, m2l_onLoginCommunityUser,
    ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate)
{
    EXPECT_TRUE(isCommunityServerProxy());

    accountInfo;
    addCallCount("m2l_onLoginCommunityUser");
    lastErrorCode_ = errorCode;
    lastIssuedCertificate_ = reissuedCertificate;
}


DEFINE_SRPC_METHOD_2(MockLoginServerSideProxy, m2l_evUserExpelled,
    AccountId, accountId, ExpelReason, expelReason)
{
    EXPECT_TRUE(isCommunityServerProxy());

    accountId;
    addCallCount("m2l_evUserExpelled");
    lastExpelReason_ = expelReason;
}
