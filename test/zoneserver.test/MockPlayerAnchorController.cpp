#include "ZoneServerTestPCH.h"
#include "MockPlayerAnchorController.h"

using namespace gideon::zoneserver;

MockPlayerAnchorController::MockPlayerAnchorController(zoneserver::go::Entity* owner) :
    PlayerAnchorController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}




DEFINE_SRPC_METHOD_1(MockPlayerAnchorController, onCreateAnchor,
    ErrorCode, errorCode)
{
    addCallCount("onCreateAnchor");

    lastErrorCode_ = errorCode;
}

//
//DEFINE_SRPC_METHOD_2(MockPlayerAnchorController, onBuildAnchor,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo)
//{
//    addCallCount("onBuildAnchor");
//
//    lastErrorCode_ = errorCode;
//    lastAnchorId_ = anchorInfo.objectId_;
//}
//
//
//DEFINE_SRPC_METHOD_3(MockPlayerAnchorController, onPutfuelItem,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemInfo, itemInfo)
//{
//    addCallCount("onPutfuelItem");
//
//    lastErrorCode_ = errorCode;
//    lastAnchorId_ = anchorInfo.objectId_;
//    itemInfo;
//}
//
//
//DEFINE_SRPC_METHOD_3(MockPlayerAnchorController, onPopfuelItem,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ObjectId, itemId)
//{
//    addCallCount("onPutfuelItem");
//
//    lastErrorCode_ = errorCode;
//    lastAnchorId_ = anchorInfo.objectId_;
//    itemId;
//}
//
//
//DEFINE_SRPC_METHOD_3(MockPlayerAnchorController, onQueryInsideAnchorInfo,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemMap, itemMap)
//{
//    addCallCount("onPutfuelItem");
//
//    lastErrorCode_ = errorCode;
//    lastAnchorId_ = anchorInfo.objectId_;
//    itemMap;
//}
//
//
//DEFINE_SRPC_METHOD_4(MockPlayerAnchorController, onExcuteAnchorEffectByPosition,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, Position, position)
//{
//    addCallCount("onExcuteAnchorEffectByPosition");
//
//    lastErrorCode_ = errorCode;
//    lastAnchorId_ = anchorInfo.objectId_;
//    index, position;
//}
//
//
//DEFINE_SRPC_METHOD_4(MockPlayerAnchorController, onExcuteAnchorEffectByTarget,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo)
//{
//    addCallCount("onExcuteAnchorEffectByTarget");
//
//    lastErrorCode_ = errorCode;
//    lastAnchorId_ = anchorInfo.objectId_;
//    index, targetInfo;
//}
//
//
//DEFINE_SRPC_METHOD_3(MockPlayerAnchorController, evAnchorStartBuilt,
//    GameObjectInfo, anchorInfo, ObjectId, ownerId, sec_t, startBuildTime)
//{
//    addCallCount("evAnchorStartBuilt");
//    lastAnchorId_ = anchorInfo.objectId_;
//    lastOwnerId_ = ownerId;
//    startBuildTime;
//}
//
//
//
//DEFINE_SRPC_METHOD_1(MockPlayerAnchorController, evAnchorBroken,
//    GameObjectInfo, anchorInfo)
//{
//    addCallCount("evAnchorBroken");
//    lastAnchorId_ = anchorInfo.objectId_;
//}
//
//
//DEFINE_SRPC_METHOD_1(MockPlayerAnchorController, evAnchorReconnaissanced,
//    Positions, positions)
//{
//    addCallCount("evAnchorReconnaissanced");
//    positions;
//}
//
//
//DEFINE_SRPC_METHOD_3(MockPlayerAnchorController, evAnchorPositionFired,
//    GameObjectInfo, anchorInfo, uint8_t, index, Position, position)
//{
//    addCallCount("evAnchorPositionFired");
//    lastAnchorId_ = anchorInfo.objectId_;
//    index, position;
//}
//
//
//DEFINE_SRPC_METHOD_3(MockPlayerAnchorController, evAnchorTargetFired,
//    GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo)
//{
//    addCallCount("evAnchorTargetFired");
//    lastAnchorId_ = anchorInfo.objectId_;
//    index, targetInfo;
//}