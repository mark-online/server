#pragma once

#include <gideon/cs/shared/data/TradeInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Money.h>
#include <sne/core/utility/Singleton.h>
#include <atomic>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

class Trade;

/**
 * @class TradeManager
 */
class TradeManager
{
	typedef std::mutex LockType;

	typedef std::shared_ptr<Trade> TradePtr;
	typedef sne::core::HashMap<TradeId, TradePtr> TradePtrMap;
	typedef sne::core::HashMap<ObjectId, TradePtr> WaitPlayerMap;

	SNE_DECLARE_SINGLETON(TradeManager);
public:
	TradeManager();
	~TradeManager();

	ErrorCode requestTrade(go::Entity& owner, ObjectId playerId);
	ErrorCode respondTrade(TradeId& tradeId, go::Entity& owner, ObjectId playerId, bool isAnswer);
	ErrorCode cancelTrade(ObjectId playerId, TradeId tradeId);
	ErrorCode toggleReady(bool& isCancel, bool& isReady, TradeId tradeId, ObjectId playerId, GameMoney gameMoney);
	ErrorCode toggleConform(bool& isCompleted, bool& isConform, TradeId tradeId, ObjectId playerId);
	ErrorCode addItem(ObjectId itemId, TradeId tradeId, ObjectId playerId);
	ErrorCode removeItem(ObjectId itemId, TradeId tradeId, ObjectId playerId);
    ErrorCode swapItem(ObjectId invenItemId, ObjectId tradeItemId, TradeId tradeId, ObjectId playerId);
	ErrorCode setGameMoney(GameMoney gameMoney, TradeId tradeId, ObjectId playerId);

private:
	TradeManager::TradePtr getWaitTrade(ObjectId playerId);
	TradeManager::TradePtr getTrade(TradeId tradeId);

private:
	ErrorCode checkCanTrade(go::Entity*& order, go::Entity& owner,
		ObjectId playerId) const;
    ErrorCode checkCanRespondTrade(go::Entity*& order, 
        go::Entity& owner, ObjectId playerId) const;
	bool isWaited(ObjectId playerId) const;

private:
	std::atomic<TradeId> tradeIdGenerator_;
	WaitPlayerMap waitPlayerMap_;
	TradePtrMap tradePtrMap_;

	mutable LockType lock_;

};


}}  //namespace gideon { namespace zoneserver {

#define TRADE_MANAGER TradeManager::instance()
