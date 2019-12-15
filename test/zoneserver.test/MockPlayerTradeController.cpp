#include "ZoneServerTestPCH.h"
#include "MockPlayerTradeController.h"

using namespace gideon::zoneserver;

MockPlayerTradeController::MockPlayerTradeController(zoneserver::go::Entity* owner) :
    PlayerTradeController(owner),
    lastErrorCode_(ecWhatDidYouTest),
    lastTradePlayerId_(invalidObjectId)
{
}



DEFINE_SRPC_METHOD_2(MockPlayerTradeController, onRequestTrade,
    ErrorCode, erroCode, ObjectId, playerId)
{
    addCallCount("onRequestTrade");
    lastErrorCode_ = erroCode;
    playerId;
}


DEFINE_SRPC_METHOD_3(MockPlayerTradeController, onRespondTrade,
    ErrorCode, erroCode, ObjectId, playerId, bool, isAnwser)
{
    addCallCount("onRespondTrade");
    lastErrorCode_ = erroCode;
    playerId, isAnwser;
}


DEFINE_SRPC_METHOD_1(MockPlayerTradeController, onCancelTrade,
    ErrorCode, errorCode)
{
    addCallCount("onCancelTrade");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_3(MockPlayerTradeController, onToggleTradeReady,
    ErrorCode, errorCode, bool, isReady, GameMoney, gameMoney)
{
    addCallCount("onToggleTradeReady");
    lastErrorCode_ = errorCode;
    isReady, gameMoney;
}


DEFINE_SRPC_METHOD_2(MockPlayerTradeController, onToggleTradeConform,
    ErrorCode, errorCode, bool, isConform)
{
    addCallCount("onToggleTradeConform");
    lastErrorCode_ = errorCode;
    isConform;
}


DEFINE_SRPC_METHOD_2(MockPlayerTradeController, onAddTradeItem, 
    ErrorCode, errorCode, ObjectId, itemId)
{
    addCallCount("onAddTradeItem");
    lastErrorCode_ = errorCode;
    itemId;
}


DEFINE_SRPC_METHOD_2(MockPlayerTradeController, onRemoveTradeItem, 
    ErrorCode, errorCode, ObjectId, itemId)
{
    addCallCount("onAddTradeItem");
    lastErrorCode_ = errorCode;
    itemId;
}


DEFINE_SRPC_METHOD_5(MockPlayerTradeController, onBuyItem,
    ErrorCode, errorCode, ObjectId, npcId, BaseItemInfo, itemInfo,
    CostType, costType, uint64_t, currentCostValue)
{
    addCallCount("onBuyItem");

    lastErrorCode_ = errorCode; 
    npcId, itemInfo, costType, currentCostValue;
}


DEFINE_SRPC_METHOD_5(MockPlayerTradeController, onSellItem,
    ErrorCode, errorCode, uint32_t, buyBackIndex, BuyBackItemInfo, buyBackInfo,
    CostType, costType, uint64_t, currentValue)
{
    addCallCount("onSellItem");

    lastErrorCode_ = errorCode; 
    buyBackIndex, buyBackInfo, costType, currentValue;
}



DEFINE_SRPC_METHOD_1(MockPlayerTradeController, evTradeRequested,
    ObjectId, playerId)
{
    addCallCount("evTradeRequested");
    lastTradePlayerId_ = playerId;
}


DEFINE_SRPC_METHOD_2(MockPlayerTradeController, evTradeReponded,
    ObjectId, playerId, bool, isAnwser)
{
    addCallCount("evTradeReponded");
    playerId, isAnwser;
}


DEFINE_SRPC_METHOD_1(MockPlayerTradeController, evTradeCancelled,
    ObjectId, playerId)
{
    addCallCount("evTradeCancelled");
    playerId;
}


DEFINE_SRPC_METHOD_2(MockPlayerTradeController, evTradeReadyToggled,
    bool, isReady, GameMoney, gameMoney)
{
    addCallCount("evTradeReadyToggled");
    isReady, gameMoney;
}


DEFINE_SRPC_METHOD_1(MockPlayerTradeController, evTradeConformToggled,
    bool, isConform)
{
    addCallCount("evTradeConformToggled");
    isConform;
}


DEFINE_SRPC_METHOD_1(MockPlayerTradeController, evTradeItemAdded,
    ItemInfo, itemInfo)
{
    addCallCount("evTradeItemAdded");
    itemInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerTradeController, evTradeItemRemoved,
    ObjectId, itemId)
{
    addCallCount("evTradeItemRemoved");
    itemId;
}
