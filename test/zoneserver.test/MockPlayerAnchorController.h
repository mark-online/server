#pragma once

#include "ZoneServer/controller/player/PlayerAnchorController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerAnchorController
 *
 * 테스트 용 mock MockPlayerAnchorController
 */
class MockPlayerAnchorController :
    public zoneserver::gc::PlayerAnchorController,
    public sne::test::CallCounter
{
public:
    MockPlayerAnchorController(zoneserver::go::Entity* owner);

    OVERRIDE_SRPC_METHOD_1(onCreateAnchor,
        ErrorCode, errorCode);
    //OVERRIDE_SRPC_METHOD_2(onBuildAnchor,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo);
    //OVERRIDE_SRPC_METHOD_3(onPutfuelItem,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemInfo, itemInfo);
    //OVERRIDE_SRPC_METHOD_3(onPopfuelItem,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ObjectId, itemId);
    //OVERRIDE_SRPC_METHOD_3(onQueryInsideAnchorInfo,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemMap, itemMap);
    //OVERRIDE_SRPC_METHOD_4(onExcuteAnchorEffectByPosition,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, Position, position);
    //OVERRIDE_SRPC_METHOD_4(onExcuteAnchorEffectByTarget,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo);

    //OVERRIDE_SRPC_METHOD_3(evAnchorStartBuilt,
    //    GameObjectInfo, anchorInfo, ObjectId, ownerId, sec_t, startBuildTime);
    //OVERRIDE_SRPC_METHOD_1(evAnchorBroken,
    //    GameObjectInfo, anchorInfo);
    //OVERRIDE_SRPC_METHOD_1(evAnchorReconnaissanced,
    //    Positions, positions);
    //OVERRIDE_SRPC_METHOD_3(evAnchorPositionFired,
    //    GameObjectInfo, anchorInfo, uint8_t, index, Position, position);
    //OVERRIDE_SRPC_METHOD_3(evAnchorTargetFired,
    //    GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo);
public:
    ErrorCode lastErrorCode_;
    ObjectId lastAnchorId_;
    ObjectId lastOwnerId_;
};
