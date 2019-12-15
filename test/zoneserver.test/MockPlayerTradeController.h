#pragma once

#include "ZoneServer/controller/player/PlayerTradeController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerTradeController
 *
 * 테스트 용 mock MockPlayerTradeController
 */
class MockPlayerTradeController :
    public zoneserver::gc::PlayerTradeController,
    public sne::test::CallCounter
{
public:
    MockPlayerTradeController(zoneserver::go::Entity* owner);

    // = rpc::TradeRpc overriding
    OVERRIDE_SRPC_METHOD_2(onRequestTrade,
        ErrorCode, erroCode, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_3(onRespondTrade,
        ErrorCode, erroCode, ObjectId, playerId, bool, isAnwser);
    OVERRIDE_SRPC_METHOD_1(onCancelTrade,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_3(onToggleTradeReady,
        ErrorCode, errorCode, bool, isReady, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_2(onToggleTradeConform,
        ErrorCode, errorCode, bool, isConform);
    OVERRIDE_SRPC_METHOD_2(onAddTradeItem, 
        ErrorCode, errorCode, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(onRemoveTradeItem, 
        ErrorCode, errorCode, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_5(onBuyItem,
        ErrorCode, errorCode, ObjectId, npcId, BaseItemInfo, itemInfo,
        CostType, costType, uint64_t, currentCostValue);
    OVERRIDE_SRPC_METHOD_5(onSellItem,
        ErrorCode, errorCode, uint32_t, buyBackIndex, BuyBackItemInfo, buyBackInfo,
        CostType, costType, uint64_t, currentValue);

    OVERRIDE_SRPC_METHOD_1(evTradeRequested,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(evTradeReponded,
        ObjectId, playerId, bool, isAnwser);
    OVERRIDE_SRPC_METHOD_1(evTradeCancelled,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(evTradeReadyToggled,
        bool, isReady, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_1(evTradeConformToggled,
        bool, isConform);
    OVERRIDE_SRPC_METHOD_1(evTradeItemAdded,
        ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_1(evTradeItemRemoved,
        ObjectId, itemId);

public:
    ErrorCode lastErrorCode_;
    ObjectId lastTradePlayerId_;
};
