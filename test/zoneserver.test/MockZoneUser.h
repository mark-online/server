#pragma once

#include "ZoneServer/user/detail/ZoneUserImpl.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockZoneUser
 *
 * 테스트 용 mock ZoneUser
 */
class MockZoneUser :
    public zoneserver::ZoneUserImpl,
    public sne::test::CallCounter
{
public:
    MockZoneUser(zoneserver::go::EntityAllocator& playerAllocator);

private:
    // = rpc::AccountRpc overriding
    //OVERRIDE_SRPC_METHOD_1(onQueryUserInfo,
    //    FullUserInfo, userInfo);
    //OVERRIDE_SRPC_METHOD_2(onCreateCharacter,
    //    ErrorCode, errorCode, FullCharacterInfo, characterInfo);
    //OVERRIDE_SRPC_METHOD_2(onDeleteCharacter,
    //    ErrorCode, errorCode, ObjectId, characterId);
    //OVERRIDE_SRPC_METHOD_4(onEnterWorld,
    //    ErrorCode, errorCode, FullCharacterInfo, characterInfo, MapCode, mapCode,
    //    WorldTime, worldTime);

public:
    ErrorCode lastErrorCode_;
    FullCharacterInfo lastCharacterInfo_;
};
