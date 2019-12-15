#include "ZoneServerTestPCH.h"
#include "MockPlayerAuctionController.h"

using namespace gideon::zoneserver;

MockPlayerAuctionController::MockPlayerAuctionController(zoneserver::go::Entity* owner) :
    PlayerAuctionController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}


DEFINE_SRPC_METHOD_2(MockPlayerAuctionController, onCreateAuction,
    ErrorCode, errorCode, GameMoney, currentMoney)
{
    addCallCount("onCreateAuction");
    lastErrorCode_ = errorCode;
    currentMoney;
}


DEFINE_SRPC_METHOD_2(MockPlayerAuctionController, onSearchEquipmentByEquipType,
    ErrorCode, errorCode, AuctionInfos, infos)
{
    addCallCount("onSearchEquipmentByEquipType");
    lastErrorCode_ = errorCode;
    auctionInfos_ = infos;
}


DEFINE_SRPC_METHOD_2(MockPlayerAuctionController, onSearchItemsByCodeType,
    ErrorCode, errorCode, AuctionInfos, infos)
{
    addCallCount("onSearchItemsByCodeType");
    lastErrorCode_ = errorCode;
    auctionInfos_ = infos;
}


DEFINE_SRPC_METHOD_2(MockPlayerAuctionController, onSellingItems,
    ErrorCode, errorCode, AuctionInfos, infos)
{
    addCallCount("onSellingItems");
    lastErrorCode_ = errorCode;
    auctionInfos_ = infos;
}


DEFINE_SRPC_METHOD_2(MockPlayerAuctionController, onBiddingItems,
    ErrorCode, errorCode, AuctionInfos, infos)
{
    addCallCount("onBiddingItems");
    lastErrorCode_ = errorCode;
    auctionInfos_ = infos;
}


DEFINE_SRPC_METHOD_4(MockPlayerAuctionController, onBid,
    ErrorCode, errorCode, AuctionId, auctionId, GameMoney, bidMoney, GameMoney, currentMoney)
{
    addCallCount("onBid");
    lastErrorCode_ = errorCode;
    auctionId, bidMoney, currentMoney;
}


DEFINE_SRPC_METHOD_3(MockPlayerAuctionController, onBuyOut,
    ErrorCode, errorCode, AuctionId, auctionId, GameMoney, currentMoney)
{
    addCallCount("onBuyOut");
    lastErrorCode_ = errorCode;
    auctionId, currentMoney;
}


DEFINE_SRPC_METHOD_2(MockPlayerAuctionController, onCancelAuction,
    ErrorCode, errorCode, AuctionId, auctionId)
{
    addCallCount("onCancelAuction");
    lastErrorCode_ = errorCode;
    auctionId;
}