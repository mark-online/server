#include "ZoneServerTestPCH.h"
#include "MockZoneUser.h"
#include "MockPlayer.h"

MockZoneUser::MockZoneUser(zoneserver::go::EntityAllocator& playerAllocator) :
    ZoneUserImpl(playerAllocator),
    lastErrorCode_(ecWhatDidYouTest)
{
}

// = rpc::AccountRpc overriding

//DEFINE_SRPC_METHOD_1(MockZoneUser, onQueryUserInfo,
//    FullUserInfo, userInfo)
//{
//    addCallCount("onQueryUserInfo");
//
//    userInfo;
//}
//
//
//DEFINE_SRPC_METHOD_2(MockZoneUser, onCreateCharacter,
//    ErrorCode, errorCode, FullCharacterInfo, characterInfo)
//{
//    addCallCount("onCreateCharacter");
//
//    lastErrorCode_ = errorCode;
//    lastCharacterInfo_ = characterInfo;
//}
//
//
//DEFINE_SRPC_METHOD_2(MockZoneUser, onDeleteCharacter,
//    ErrorCode, errorCode, ObjectId, characterId)
//{
//    addCallCount("onDeleteCharacter");
//
//    lastErrorCode_ = errorCode;
//    characterId;
//}
//
//
//DEFINE_SRPC_METHOD_4(MockZoneUser, onEnterWorld,
//    ErrorCode, errorCode, FullCharacterInfo, characterInfo, MapCode, mapCode,
//    WorldTime, worldTime)
//{
//    errorCode, characterInfo, mapCode, worldTime;
//
//    addCallCount("onEnterWorld");
//}
