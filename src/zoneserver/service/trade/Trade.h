#pragma once

#include <gideon/cs/shared/data/TradeInfo.h>
#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/Money.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

struct TradeSlot
{
	typedef sne::core::HashMap<ObjectId, ItemInfo*> TradeItemMap;
	typedef sne::core::HashMap<ObjectId, EquipItemInfo*> TradeEquipInfoMap;

	ObjectId playerId_;
	TradeItemMap tradeItemMap_;
	TradeEquipInfoMap equipItemMap_;
	GameMoney gameMoney_;

	InventoryInfo inventoryInfo_;
	bool isReady_;
	bool isConform_;

	TradeSlot(ObjectId playerId, const InventoryInfo& inventoryInfo) :
		playerId_(playerId),
		inventoryInfo_(inventoryInfo),
		isReady_(false),
		isConform_(false),
		gameMoney_(gameMoneyMin) {}

	bool canTradeItem() const {
		return isReady_ && isConform_;
	}

	bool isResistItem(ObjectId itemId) const {
		return tradeItemMap_.find(itemId) != tradeItemMap_.end();
	}

	void insertTradeItem(ItemInfo* itemInfo) {
		tradeItemMap_.emplace(itemInfo->itemId_, itemInfo);	
	}

	void insertTradeEquipItem(ItemInfo* itemInfo, EquipItemInfo* equipItemInfo) {
		tradeItemMap_.emplace(itemInfo->itemId_, itemInfo);
		equipItemMap_.emplace(itemInfo->itemId_, equipItemInfo);
	}

	void removeTradeItem(ObjectId itemId) {
		tradeItemMap_.erase(itemId);
		equipItemMap_.erase(itemId);
	}

	const EquipItemInfo* getTradeEquipItemInfo(ObjectId itemId) const {
		TradeEquipInfoMap::const_iterator pos = equipItemMap_.find(itemId);
		if (pos != equipItemMap_.end()) {
			return (*pos).second;
		}
		return nullptr;
	}
};


/***
 * @class Trade
 ***/
class Trade
{
	typedef std::mutex LockType;
public:
	Trade(go::Entity* requester, go::Entity* responder);
	~Trade();
	
	bool checkWaitTrade(ObjectId requester, ObjectId responder);
	void initialize(TradeId tradeId);

	void cancel(ObjectId canceler);

	ErrorCode toggleReady(bool& isCancel, bool& isReady, ObjectId playerId, GameMoney gameMoney);
	ErrorCode toggleConform(bool& isCompleted, bool& isReady, ObjectId playerId);
	ErrorCode addItem(ObjectId itemId, ObjectId playerId);
	ErrorCode removeItem(ObjectId itemId, ObjectId playerId);
    ErrorCode swapItem(ObjectId invenItemId, ObjectId tradeSlotItemId, ObjectId playerId);

public:
	TradeId getTradeId() const {
		return tradeId_;
	}

private:
	bool isExistPlayer(ObjectId  playerId) const;

	TradeSlot* getOtherTradeSlot(ObjectId  playerId);
	TradeSlot* getTradeSlot(ObjectId  playerId);
	go::Entity* getTradePlayer(ObjectId  playerId);
	go::Entity* getOrderTradePlayer(ObjectId  playerId);

	ErrorCode checkEnoughInventory(TradeSlot& selfSlot, TradeSlot& otherSlot);
	void tradeItems();

private:
	typedef sne::core::HashMap<ObjectId, go::Entity*> TradePlayerMap;
	typedef sne::core::HashMap<ObjectId, TradeSlot*> PlayerTradeSlotMap;

	TradePlayerMap tradePlayerMap_;
	PlayerTradeSlotMap playerTradeSlot_;
	TradeId tradeId_;

	mutable LockType lock_;
};


}}  //namespace gideon { namespace zoneserver {
