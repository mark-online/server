#include "ZoneServerPCH.h"
#include "TradeManager.h"
#include "Trade.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/state/TradeState.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/TradeCallback.h"
#include "../../world/WorldMap.h"
#include "../distance/DistanceChecker.h"

namespace gideon { namespace zoneserver {

SNE_DEFINE_SINGLETON(TradeManager);

TradeManager::TradeManager()
{
}


TradeManager::~TradeManager()
{
}


ErrorCode TradeManager::requestTrade(go::Entity& owner, ObjectId playerId)
{
	if (isWaited(owner.getObjectId())) {
		return ecTradeWaitSelfTradeState;
	}

	if (isWaited(playerId)) {
		return ecTradeWaitOrderTradeState;
	}
	
	go::Entity* order = nullptr;
	const ErrorCode errorCode = checkCanRespondTrade(order, owner, playerId);
	if (isFailed(errorCode)) {
		return errorCode;
	}
	
	auto trade = std::make_shared<Trade>(&owner, order);
	{
        std::lock_guard<LockType> lock(lock_);

        waitPlayerMap_.emplace(owner.getObjectId(), trade);
	}

	gc::TradeCallback* tradeCallback = order->getController().queryTradeCallback();
	if (tradeCallback) {
		tradeCallback->tradeGotRequest(owner.getObjectId());
	}

	return ecOk;
}


ErrorCode TradeManager::respondTrade(TradeId& tradeId, go::Entity& owner, ObjectId playerId, bool isAnswer)
{
	TradePtr waitTrade = getWaitTrade(playerId);
	if (waitTrade.get() == nullptr) {
		return ecTradeCancelRequestTrade;
	}

	if (isWaited(owner.getObjectId())) {
		return ecTradeWaitSelfTradeState;
	}

	if (! waitTrade->checkWaitTrade(playerId, owner.getObjectId())) {
		return ecTradeInvalidTradeInfo;
	}

	go::Entity* order = nullptr;
	const ErrorCode errorCode = checkCanTrade(order, owner, playerId);
	if (isFailed(errorCode)) {
		return errorCode;
	}
	
	tradeId = invalidTradeId;
	
    {
        std::lock_guard<LockType> lock(lock_);

        waitPlayerMap_.erase(playerId);
	}

	if (isAnswer) {
		tradeId = ++tradeIdGenerator_;
		waitTrade->initialize(tradeId);
		{
            std::lock_guard<LockType> lock(lock_);

            tradePtrMap_.emplace(tradeId, waitTrade);
		}
	}

	gc::TradeCallback* tradeCallback = order->getController().queryTradeCallback();
	if (tradeCallback) {
		tradeCallback->tradeGotRespond(tradeId, owner.getObjectId(), isAnswer);
	}
	
	return ecOk;
}


ErrorCode TradeManager::cancelTrade(ObjectId playerId, TradeId tradeId)
{
	TradePtr trade;
	if (isValidTradeId(tradeId)) {
		trade = getTrade(tradeId);
		{
            std::lock_guard<LockType> lock(lock_);

            tradePtrMap_.erase(tradeId);
		}
	}
	else {
		trade = getWaitTrade(playerId);		
		{
            std::lock_guard<LockType> lock(lock_);

            waitPlayerMap_.erase(playerId);
		}
	}

	if (trade.get() == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	trade->cancel(playerId);

	return ecOk;
}


ErrorCode TradeManager::toggleReady(bool& isCancel, bool& isReady, TradeId tradeId, ObjectId playerId,
    GameMoney gameMoney)
{
	TradePtr trade = getTrade(tradeId);

	if (trade.get() == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	const ErrorCode errorCode = trade->toggleReady(isCancel, isReady, playerId, gameMoney);
    if (isCancel) {
        cancelTrade(playerId, tradeId);
    }

    return errorCode;
}


ErrorCode TradeManager::toggleConform(bool& isCompleted, bool& isConform, TradeId tradeId, ObjectId playerId)
{
	TradePtr trade = getTrade(tradeId);

	if (trade.get() == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	const ErrorCode errorCode = trade->toggleConform(isCompleted, isConform, playerId);
	if (isCompleted) {
        std::lock_guard<LockType> lock(lock_);

        tradePtrMap_.erase(tradeId);
	}
	return errorCode;
}


ErrorCode TradeManager::addItem(ObjectId itemId, TradeId tradeId, ObjectId playerId)
{
	TradePtr trade = getTrade(tradeId);

	if (trade.get() == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	return trade->addItem(itemId, playerId);
}


ErrorCode TradeManager::removeItem(ObjectId itemId, TradeId tradeId, ObjectId playerId)
{
	TradePtr trade = getTrade(tradeId);

	if (trade.get() == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	return trade->removeItem(itemId, playerId);
}


ErrorCode TradeManager::swapItem(ObjectId invenItemId, ObjectId tradeItemId, TradeId tradeId, ObjectId playerId)
{
    TradePtr trade = getTrade(tradeId);

    if (trade.get() == nullptr) {
        return ecTradeInvalidTradeInfo;
    }

    return trade->swapItem(invenItemId, tradeItemId, playerId);
}


TradeManager::TradePtr TradeManager::getWaitTrade(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

	WaitPlayerMap::iterator pos = waitPlayerMap_.find(playerId);
	if (pos != waitPlayerMap_.end()) {
		return (*pos).second;
	}

	return TradePtr();
}


TradeManager::TradePtr TradeManager::getTrade(TradeId tradeId)
{
    std::lock_guard<LockType> lock(lock_);

	TradePtrMap::iterator pos = tradePtrMap_.find(tradeId);
	if (pos != tradePtrMap_.end()) {
		return (*pos).second;
	}

    return TradePtr();
}


ErrorCode TradeManager::checkCanTrade(go::Entity*& order, go::Entity& owner,
	ObjectId playerId) const
{	
	go::Entity* entity = owner.queryKnowable()->getEntity(GameObjectInfo(otPc, playerId));
	if (entity == nullptr) {
		return ecEntityNotFound;
	}

	TradeState* tradeState = entity->queryTradeState();
	if (tradeState == nullptr) {
		return ecTradeCannotTradeObject;
	}

    // 요청한사람.
	if (! tradeState->isTradeRequstState()) {
		return ecTradeCannotOrderTradeState;
	}

    if (! DISTANCE_CHECKER->checkTradeDistance(entity->getPosition(), owner.getPosition())) {
		return ecTradeTargetFarDistance;
	}

	order = static_cast<go::Entity*>(entity);

	return ecOk;
}


ErrorCode TradeManager::checkCanRespondTrade(go::Entity*& order, 
    go::Entity& owner, ObjectId playerId) const
{
    go::Entity* entity = owner.queryKnowable()->getEntity(GameObjectInfo(otPc, playerId));
    if (entity == nullptr) {
        return ecEntityNotFound;
    }

    TradeState* tradeState = entity->queryTradeState();
    if (tradeState == nullptr) {
        return ecTradeCannotTradeObject;
    }

    if (! tradeState->canStartTrade()) {
        return ecTradeCannotOrderTradeState;
    }

    if (! DISTANCE_CHECKER->checkTradeDistance(entity->getPosition(), owner.getPosition())) {
        return ecTradeTargetFarDistance;
    }

    order = static_cast<go::Entity*>(entity);
    return ecOk;
}


bool TradeManager::isWaited(ObjectId playerId) const
{
    std::lock_guard<LockType> lock(lock_);

	return waitPlayerMap_.find(playerId) != waitPlayerMap_.end();
}

}}  //namespace gideon { namespace zoneserver {
