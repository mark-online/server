#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/TradeCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/TradeRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerTradeController
 * 거래 담당
 */
class ZoneServer_Export PlayerTradeController : public Controller,
	public TradeCallback,
    public rpc::TradeRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerTradeController);
public:
    PlayerTradeController(go::Entity* owner);

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

public:
    virtual void initialize();
    virtual void finalize();

private:
	// = TradeCallback overriding
	virtual void tradeGotRequest(ObjectId playerId);
	virtual void tradeGotRespond(TradeId tradeId, ObjectId playerId, bool isAnswer);
	virtual void tradeCancelled(TradeId tradeId, ObjectId playerId);
	virtual void tradeReadyToggled(bool isReady, GameMoney gameMoney);
	virtual void tradeConformToggled(bool isConform);
	virtual void tradeItemAdded(const ItemInfo& itemInfo);
	virtual void tradeItemRemoved(ObjectId itemId);
	virtual void tradeCompleted(TradeId tradeId);

public:
    // = rpc::TradeRpc overriding
    OVERRIDE_SRPC_METHOD_1(requestTrade,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(respondTrade,
        ObjectId, playerId, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_0(cancelTrade);
    OVERRIDE_SRPC_METHOD_1(toggleTradeReady,
        GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_0(toggleTradeConform);
    OVERRIDE_SRPC_METHOD_1(addTradeItem, 
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_1(removeTradeItem, 
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(switchTradeItem, 
        ObjectId, invenItemId, ObjectId, tradeItemId);
    OVERRIDE_SRPC_METHOD_2(buyItem,
        ObjectId, npcId, BaseItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_3(sellItem,
        ObjectId, npcId, ObjectId, itemId, uint8_t, itemCount);
    OVERRIDE_SRPC_METHOD_1(queryBuyBackItemInfos,
        ObjectId, npcId);
    OVERRIDE_SRPC_METHOD_2(buyBackItem,
        ObjectId, npcId, uint32_t, index);


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
    OVERRIDE_SRPC_METHOD_3(onSwitchTradeItem, 
        ErrorCode, errorCode, ObjectId, invenItemId, ObjectId, tradeItemId);
    OVERRIDE_SRPC_METHOD_5(onBuyItem,
        ErrorCode, errorCode, ObjectId, npcId, BaseItemInfo, itemInfo,
        CostType, type, uint64_t, currentCostValue);
    OVERRIDE_SRPC_METHOD_5(onSellItem,
        ErrorCode, errorCode, uint32_t, buyBackIndex, BuyBackItemInfo, buyBackInfo, CostType, costType, uint64_t, currentValue);
    OVERRIDE_SRPC_METHOD_2(onBuyBackItemInfos,
        ErrorCode, errorCode, BuyBackItemInfoMap, itemMap);
    OVERRIDE_SRPC_METHOD_4(onBuyBackItem,
        ErrorCode, errorCode, uint32_t, index, CostType, ct, uint64_t, currentValue);

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

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    ErrorCode checkBuyItem(GameMoney& totalPrice, CostType& costType, ObjectId npcId, 
        const BaseItemInfo& itemInfo);
	ErrorCode checkSellItem(CostType& costType, uint32_t& sellPrice,
		DataCode& itemCode, ObjectId npcId, ObjectId itemId, uint8_t itemCount);
    ErrorCode checkBuyBack(ObjectId npcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
