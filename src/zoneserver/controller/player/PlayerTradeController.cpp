#include "ZoneServerPCH.h"
#include "PlayerTradeController.h"
#include "../callback/TradeCallback.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/ability/Storeable.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/Guildable.h"
#include "../../model/state/TradeState.h"
#include "../../model/state/ItemManageState.h"
#include "../../model/item/Inventory.h"
#include "../../service/trade/TradeManager.h"
#include "../../service/distance/DistanceChecker.h"
#include "../../service/item/ItemCostService.h"
#include <gideon/cs/datatable/NpcSellTable.h>
#include <gideon/cs/datatable/NpcBuyTable.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerTradeController);
	
PlayerTradeController::PlayerTradeController(go::Entity* owner) :
	Controller(owner)
{
}


void PlayerTradeController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerTradeController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


// = TradeCallback overriding
void PlayerTradeController::tradeGotRequest(ObjectId playerId)
{
	evTradeRequested(playerId);
}


void PlayerTradeController::tradeGotRespond(TradeId tradeId, ObjectId playerId, bool isAnswer)
{
	if (isAnswer) {
		getOwner().queryTradeState()->tradeAccepted(tradeId);		
	}
    else {
        getOwner().queryTradeState()->tradeCancelled(tradeId);
    }

	evTradeReponded(playerId, isAnswer);
}


void PlayerTradeController::tradeCancelled(TradeId tradeId, ObjectId playerId)
{
	getOwner().queryTradeState()->tradeCancelled(tradeId);
	evTradeCancelled(playerId);
}


void PlayerTradeController::tradeReadyToggled(bool isReady, GameMoney gameMoney)
{
	evTradeReadyToggled(isReady, gameMoney);
}


void PlayerTradeController::tradeConformToggled(bool isReady)
{
	evTradeConformToggled(isReady);
}


void PlayerTradeController::tradeItemAdded(const ItemInfo& itemInfo)
{
	evTradeItemAdded(itemInfo);
}


void PlayerTradeController::tradeItemRemoved(ObjectId itemId)
{
	evTradeItemRemoved(itemId);
}


void PlayerTradeController::tradeCompleted(TradeId tradeId)
{
	getOwner().queryTradeState()->tradeCompleted(tradeId);
	tradeConformToggled(true);
}



void PlayerTradeController::initialize()
{
}


void PlayerTradeController::finalize()
{
}


// = 
RECEIVE_SRPC_METHOD_1(PlayerTradeController, requestTrade,
    ObjectId, playerId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = ecOk;
    if (! owner.queryTradeState()->canRequestTrade()) {
        errorCode = ecTradeCannotSelfTradeState;
    }

    if (isSucceeded(errorCode)) {
        errorCode = TRADE_MANAGER->requestTrade(owner, playerId);
        if (isSucceeded(errorCode)) {
            owner.queryTradeState()->tradeRequest();
        }
    }

    onRequestTrade(errorCode, playerId);
}


RECEIVE_SRPC_METHOD_2(PlayerTradeController, respondTrade,
    ObjectId, playerId, bool, isAnswer)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    TradeId tradeId = invalidTradeId;
    ErrorCode errorCode = ecOk;
    if (! owner.queryTradeState()->canStartTrade()) {
        errorCode = ecTradeCannotSelfTradeState;
    }

    if (isSucceeded(errorCode)) {
       errorCode = TRADE_MANAGER->respondTrade(tradeId, owner, playerId, isAnswer);
       if (isSucceeded(errorCode)) {
           if (isAnswer) {
               owner.queryTradeState()->tradeAccepted(tradeId);		
           }
       }
    }

    onRespondTrade(errorCode, playerId, isAnswer);
}


RECEIVE_SRPC_METHOD_0(PlayerTradeController, cancelTrade)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

	TradeState* tradeState = getOwner().queryTradeState();
	if (! tradeState) {
		return;
	}

    const ErrorCode errorCode = 
        TRADE_MANAGER->cancelTrade(owner.getObjectId(), tradeState->getTradeId());

    if (isSucceeded(errorCode)) {
        tradeState->tradeCancelled(tradeState->getTradeId());
    }

    onCancelTrade(errorCode);
}


RECEIVE_SRPC_METHOD_1(PlayerTradeController, toggleTradeReady,
    GameMoney, gameMoney)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

	TradeState* tradeState = getOwner().queryTradeState();
	if (! tradeState) {
		return;
	}

    bool isCancel = false;
    bool isReady = false;
    const ErrorCode errorCode = TRADE_MANAGER->toggleReady(isCancel, isReady, 
        tradeState->getTradeId(), owner.getObjectId(), gameMoney);	
    
    if (isCancel) {
        tradeState->tradeCancelled(tradeState->getTradeId());
        evTradeCancelled(owner.getObjectId());
    }
    else {
        onToggleTradeReady(errorCode, isReady, gameMoney);
    }
	
}


RECEIVE_SRPC_METHOD_0(PlayerTradeController, toggleTradeConform)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }


	TradeState* tradeState = getOwner().queryTradeState();
	if (! tradeState) {
		return;
	}


    bool isComplete = false;
    bool isConform = false;

    const ErrorCode errorCode= TRADE_MANAGER->toggleConform(isComplete, isConform, 
        tradeState->getTradeId(), owner.getObjectId());
    if (isComplete) {
        tradeState->tradeCompleted(tradeState->getTradeId());
    }


    onToggleTradeConform(errorCode, isConform);
}


RECEIVE_SRPC_METHOD_1(PlayerTradeController, addTradeItem, 
    ObjectId, itemId)
    {
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = TRADE_MANAGER->addItem(itemId, 
        getOwner().queryTradeState()->getTradeId(), owner.getObjectId());

    onAddTradeItem(errorCode , itemId);
}


RECEIVE_SRPC_METHOD_1(PlayerTradeController, removeTradeItem, 
    ObjectId, itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = TRADE_MANAGER->removeItem(itemId, 
        getOwner().queryTradeState()->getTradeId(), owner.getObjectId());

    onRemoveTradeItem(errorCode, itemId);
}


RECEIVE_SRPC_METHOD_2(PlayerTradeController, switchTradeItem, 
    ObjectId, invenItemId, ObjectId, tradeItemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = TRADE_MANAGER->swapItem(invenItemId, tradeItemId,
        getOwner().queryTradeState()->getTradeId(), owner.getObjectId());

    onSwitchTradeItem(errorCode, invenItemId, tradeItemId);
}


RECEIVE_SRPC_METHOD_2(PlayerTradeController, buyItem,
    ObjectId, npcId, BaseItemInfo, itemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    GameMoney totalPrice = 0;
    CostType costType = ctGameMoney;
    ErrorCode errorCode = checkBuyItem(totalPrice, costType, npcId, itemInfo);
    if (isFailed(errorCode)) {
        onBuyItem(errorCode, npcId, itemInfo, costType, 0);
        return;
    }

    // 돈 또는 명점 등등.
    uint64_t currentValue = 0;
    errorCode = owner.queryStoreable()->buyItem(itemInfo, currentValue, costType, uint32_t(totalPrice));    

    onBuyItem(errorCode, npcId, itemInfo, costType, currentValue);

	SNE_LOG_INFO("GameLog PlayerTradeController::buyItem(Ec:%u, PID%" PRIu64 ", itemCode:%u, ItemCount:%d",
		errorCode, owner.getObjectId(), itemInfo.itemCode_, itemInfo.count_);
}


RECEIVE_SRPC_METHOD_3(PlayerTradeController, sellItem,
    ObjectId, npcId, ObjectId, itemId, uint8_t, itemCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    uint32_t sellCost = 0;
	CostType costType = ctNone;
	DataCode itemCode = invalidDataCode;
    const ErrorCode errorCode = checkSellItem(costType , sellCost, itemCode, npcId, itemId, itemCount);
    if (isFailed(errorCode)) {
        onSellItem(errorCode, 0, BuyBackItemInfo(), ctNone, 0);
        return;
    }

	uint64_t currentValue = 0;
    BuyBackItemInfo buyBackItemInfo;
    uint32_t buyBackIndex = 0;
    owner.queryStoreable()->sellItem(currentValue, buyBackIndex, buyBackItemInfo, itemId, itemCount, costType, sellCost);

    onSellItem(ecOk, buyBackIndex, buyBackItemInfo, costType, currentValue);

	SNE_LOG_INFO("GameLog PlayerTradeController::buyItem(Ec:%u, PID%" PRIu64 ", itemCode:%u, ItemCount:%d",
		errorCode, owner.getObjectId(), itemCode, itemCount);
}


RECEIVE_SRPC_METHOD_1(PlayerTradeController, queryBuyBackItemInfos,
    ObjectId, npcId)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = checkBuyBack(npcId);
    if (isFailed(errorCode)) {
        onBuyBackItemInfos(errorCode, BuyBackItemInfoMap());
        return;
    }

    onBuyBackItemInfos(ecOk, owner.queryStoreable()->queryBuyBackItemInfoMap());
}


RECEIVE_SRPC_METHOD_2(PlayerTradeController, buyBackItem,
    ObjectId, npcId, uint32_t, index)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    ErrorCode errorCode = checkBuyBack(npcId);
    if (isFailed(errorCode)) {
        onBuyBackItem(errorCode, index, ctNone, 0);
        return;
    }
    uint64_t value = 0;
    CostType ct = ctNone;
    errorCode = owner.queryStoreable()->buyBackItem(ct, value, index);
    onBuyBackItem(errorCode, index, ct, value);
}


FORWARD_SRPC_METHOD_2(PlayerTradeController, onRequestTrade,
    ErrorCode, erroCode, ObjectId, playerId);


FORWARD_SRPC_METHOD_3(PlayerTradeController, onRespondTrade,
    ErrorCode, erroCode, ObjectId, playerId, bool, isAnwser);


FORWARD_SRPC_METHOD_1(PlayerTradeController, onCancelTrade,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_3(PlayerTradeController, onToggleTradeReady,
    ErrorCode, errorCode, bool, isReady, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_2(PlayerTradeController, onToggleTradeConform,
    ErrorCode, errorCode, bool, isConform);


FORWARD_SRPC_METHOD_2(PlayerTradeController, onAddTradeItem, 
    ErrorCode, errorCode, ObjectId, itemId);


FORWARD_SRPC_METHOD_2(PlayerTradeController, onRemoveTradeItem,
    ErrorCode, errorCode, ObjectId, itemId);


FORWARD_SRPC_METHOD_3(PlayerTradeController, onSwitchTradeItem, 
    ErrorCode, errorCode, ObjectId, invenItemId, ObjectId, tradeItemId);


FORWARD_SRPC_METHOD_5(PlayerTradeController, onBuyItem,
    ErrorCode, errorCode, ObjectId, npcId, BaseItemInfo, itemInfo,
    CostType, type, uint64_t, currentCostValue);


FORWARD_SRPC_METHOD_5(PlayerTradeController, onSellItem,
    ErrorCode, errorCode, uint32_t, buyBackIndex, BuyBackItemInfo, buyBackInfo,
    CostType, costType, uint64_t, currentValue);


FORWARD_SRPC_METHOD_2(PlayerTradeController, onBuyBackItemInfos,
    ErrorCode, errorCode, BuyBackItemInfoMap, itemMap);


FORWARD_SRPC_METHOD_4(PlayerTradeController, onBuyBackItem,
    ErrorCode, errorCode, uint32_t, index, CostType, ct, uint64_t, currentValue);


FORWARD_SRPC_METHOD_1(PlayerTradeController, evTradeRequested,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_2(PlayerTradeController, evTradeReponded,
    ObjectId, playerId, bool, isAnwser);


FORWARD_SRPC_METHOD_1(PlayerTradeController, evTradeCancelled,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_2(PlayerTradeController, evTradeReadyToggled,
    bool, isReady, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_1(PlayerTradeController, evTradeConformToggled,
    bool, isConform);


FORWARD_SRPC_METHOD_1(PlayerTradeController, evTradeItemAdded,
    ItemInfo, itemInfo);

FORWARD_SRPC_METHOD_1(PlayerTradeController, evTradeItemRemoved,
    ObjectId, itemId);


// = sne::srpc::RpcForwarder overriding

void PlayerTradeController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTradeController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerTradeController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerTradeController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


ErrorCode PlayerTradeController::checkBuyItem(GameMoney& totalPrice, CostType& costType, ObjectId npcId,
    const BaseItemInfo& itemInfo)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return ecServerInternalError;
    }

    if (! owner.queryItemManageState()->canItemUseableState()) {
        return ecItemCannotUseableState;
    }

    go::Entity* target = 
        owner.queryKnowable()->getEntity(GameObjectInfo(otNpc, npcId));
    if (! target) {
        return ecStoreNotFindNpc;
    }

    go::Npc& npc = static_cast<go::Npc&>(*target);
    if (! npc.canSell()) {
        return ecStoreNotSellNpc;
    }

	if (! DISTANCE_CHECKER->checkNpcDistance(npc.getPosition(), owner.getPosition())) {
		return ecStoreNpcFarDistance;
	}

    const datatable::SellList* sellList =  NPC_SELL_TABLE->getSellList(target->getEntityCode()); 
    if (sellList == nullptr) {
        return ecStoreNotSellNpc;
    }

    datatable::SellList::const_iterator pos = sellList->find(itemInfo.itemCode_);
    if (pos == sellList->end()) {
        return ecStoreNotFindSellItemList;
    }

	uint32_t cost = 0;
	const ErrorCode ec = ItemCostService::getSellItemCost(costType, cost, itemInfo.itemCode_);
	if (isFailed(ec)) {
		return ec;
	}
    totalPrice = cost * itemInfo.count_;
    
    permil_t discountPer = owner.queryGuildable()->getGuildEffectValue(estStoreItemSale);
    if (0 < discountPer) {
        totalPrice -= (totalPrice * discountPer / 1000);
    }

    return ecOk;
}


ErrorCode PlayerTradeController::checkSellItem(CostType& costType, uint32_t& sellPrice,
    DataCode& itemCode, ObjectId npcId, ObjectId itemId, uint8_t itemCount)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return ecServerInternalError;
    }

    if (! owner.queryItemManageState()->canItemUseableState()) {
        return ecItemCannotUseableState;
    }

    go::Entity* target = 
        owner.queryKnowable()->getEntity(GameObjectInfo(otNpc, npcId));
    if (! target) {
        return ecStoreNotBuyNpc;
    }

    go::Npc& npc = static_cast<go::Npc&>(*target);
    if (! npc.canBuy()) {
        return ecStoreNotBuyNpc;
    }

	if (! DISTANCE_CHECKER->checkNpcDistance(npc.getPosition(), owner.getPosition())) {
		return ecStoreNpcFarDistance;
	}

    Inventory& inventory = owner.queryInventoryable()->getInventory();
    ItemInfo sellItemInfo = inventory.getItemInfo(itemId);
	itemCode = sellItemInfo.itemCode_;
    if (sellItemInfo.isEquipped()) {
        return ecStoreNotSellEquipped;
    }

    if (! sellItemInfo.isValid()) {
        return ecInventoryItemNotFound;
    }

    if (sellItemInfo.count_ < itemCount) {
        return ecStoreSellNotEnoughItemCount;
    }

	bool shouldUseDefalutValue = true;
	const gdt::npc_buy_t* npcBuy = NPC_BUY_TABLE->getNpcBuyTemplate(target->getEntityCode());
	if (npcBuy) {
		CodeType codeType = getCodeType(sellItemInfo.itemCode_);
		if (codeType == static_cast<CodeType>(static_cast<uint8_t>(npcBuy->item_code_type()))) {
			shouldUseDefalutValue = false;
		}
	}
	
	ErrorCode errorCode = ecOk; 
	uint32_t cost = 0;
	if (shouldUseDefalutValue) {
		errorCode = ItemCostService::getDefaultBuyItemCost(costType, cost, sellItemInfo.itemCode_);
	}
	else {
		errorCode = ItemCostService::getBuyItemCost(costType, cost, sellItemInfo.itemCode_);
	}

	if (isFailed(errorCode)) {
		return errorCode;
	}

    sellPrice = cost * itemCount;
    return ecOk;

}


ErrorCode PlayerTradeController::checkBuyBack(ObjectId npcId)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return ecServerInternalError;
    }

    if (! owner.queryItemManageState()->canItemUseableState()) {
        return ecItemCannotUseableState;
    }

    go::Entity* target = 
        owner.queryKnowable()->getEntity(GameObjectInfo(otNpc, npcId));
    if (! target) {
        return ecStoreNotBuyNpc;
    }

    go::Npc& npc = static_cast<go::Npc&>(*target);
    if (! (npc.canBuy() || npc.canSell())) {
        return ecStoreNotBuyNpc;
    }

    if (! DISTANCE_CHECKER->checkNpcDistance(npc.getPosition(), owner.getPosition())) {
        return ecStoreNpcFarDistance;
    }

    return ecOk;
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
