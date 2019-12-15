#pragma once

#include "loginserver/s2s/LoginServerSideProxy.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
* @class MockLoginServerSideProxy
*/
class MockLoginServerSideProxy :
    public loginserver::LoginServerSideProxy,
    public sne::test::CallCounter
{
public:
    MockLoginServerSideProxy(sne::server::ServerId serverId,
        ServerType serverType);

    void bridge() {
        bridged();
    }

    void unbridge() {
        unbridged();
    }

public:
    OVERRIDE_SRPC_METHOD_0(z2l_onRendezvous);

    OVERRIDE_SRPC_METHOD_3(z2l_onLoginZoneUser,
        ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);

    OVERRIDE_SRPC_METHOD_1(z2l_getFullUserInfo,
        AccountId, accountId);

    OVERRIDE_SRPC_METHOD_1(z2l_createCharacter,
        CreateCharacterInfo, createCharacterInfo);

    OVERRIDE_SRPC_METHOD_2(z2l_deleteCharacter,
        AccountId, accountId, ObjectId, characterId);

    OVERRIDE_SRPC_METHOD_2(z2l_checkDuplicateNickname,
        AccountId, accountId, Nickname, nickname);

    OVERRIDE_SRPC_METHOD_2(z2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);

public:
    OVERRIDE_SRPC_METHOD_3(m2l_onLoginCommunityUser,
        ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate);

    OVERRIDE_SRPC_METHOD_2(m2l_evUserExpelled,
        AccountId, accountId, ExpelReason, expelReason);

public:
    ErrorCode lastErrorCode_;
    ExpelReason lastExpelReason_;
    Certificate lastIssuedCertificate_;
};
