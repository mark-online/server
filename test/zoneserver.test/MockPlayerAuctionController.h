#pragma once

#include "ZoneServer/controller/player/PlayerAuctionController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerAnchorController
 *
 * 테스트 용 mock MockPlayerAnchorController
 */
class MockPlayerAuctionController :
    public zoneserver::gc::PlayerAuctionController,
    public sne::test::CallCounter
{
public:
    MockPlayerAuctionController(zoneserver::go::Entity* owner);

    OVERRIDE_SRPC_METHOD_2(onCreateAuction,
        ErrorCode, errorCode, GameMoney, currentMoney);
    OVERRIDE_SRPC_METHOD_2(onSearchEquipmentByEquipType,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onSearchItemsByCodeType,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onSellingItems,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onBiddingItems,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_4(onBid,
        ErrorCode, errorCode, AuctionId, auctionId, GameMoney, bidMoney, GameMoney, currentMoney);
    OVERRIDE_SRPC_METHOD_3(onBuyOut,
        ErrorCode, errorCode, AuctionId, auctionId, GameMoney, currentMoney);
    OVERRIDE_SRPC_METHOD_2(onCancelAuction,
        ErrorCode, errorCode, AuctionId, auctionId);
public:
    ErrorCode lastErrorCode_;
    AuctionInfos auctionInfos_;
};
