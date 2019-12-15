#include "ZoneServerPCH.h"
#include "Trade.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/item/Inventory.h"
#include "../../controller/EntityController.h"
#include "../../controller/player/PlayerTradeController.h"
#include "../../controller/callback/TradeCallback.h"
#include "../../helper/InventoryHelper.h"


namespace gideon { namespace zoneserver {

namespace  {

inline bool addInventoryItems(InventoryInfo& inventory, TradeSlot& orderTradeSlot)
{
	for (TradeSlot::TradeItemMap::value_type& value : orderTradeSlot.tradeItemMap_) {
		ItemInfo itemInfo = *value.second;
		if (! addInventoryItem(inventory, itemInfo)) {
			return false;
		}		
	}
	return true;
}


inline void removeInventoryItems(InventoryInfo& inventory, TradeSlot& selfTradeSlot)
{
	for (TradeSlot::TradeItemMap::value_type& value : selfTradeSlot.tradeItemMap_) {
		ObjectId itemId = value.first;
		inventory.removeItem(itemId);
	}
}

} // namespace {


Trade::Trade(go::Entity* requester, go::Entity* responder)
{
	tradePlayerMap_.emplace(requester->getObjectId(), requester);
	tradePlayerMap_.emplace(responder->getObjectId(), responder);
}


Trade::~Trade()
{
	for (PlayerTradeSlotMap::value_type& value : playerTradeSlot_) {
		delete value.second;
	}
}


bool Trade::checkWaitTrade(ObjectId requesterId, ObjectId responderId)
{
    std::lock_guard<LockType> lock(lock_);

	if (! playerTradeSlot_.empty()) {
		return false;
	}
	
	if (! isExistPlayer(requesterId) && ! isExistPlayer(responderId)) {
		return false;
	}

	return true;
}


void Trade::initialize(TradeId tradeId)
{
    std::lock_guard<LockType> lock(lock_);

	tradeId_ = tradeId;

	for (TradePlayerMap::value_type& value : tradePlayerMap_) {
		const ObjectId playerId = value.first;
		go::Entity* player = value.second;
		playerTradeSlot_.insert(PlayerTradeSlotMap::value_type(playerId,
			new TradeSlot(playerId, player->queryInventoryable()->getInventoryInfo())));
	}
}


void Trade::cancel(ObjectId canceler)
{
    std::lock_guard<LockType> lock(lock_);

	for (TradePlayerMap::value_type& value : tradePlayerMap_) {
		const ObjectId playerId = value.first;
		go::Entity* player = value.second;
		if (playerId == canceler) {
			continue;
		}
		
		gc::TradeCallback* callback = player->getController().queryTradeCallback();
		if (callback) {
			callback->tradeCancelled(tradeId_, canceler);
		}
	}
}


ErrorCode Trade::toggleReady(bool& isCancel, bool& isReady, ObjectId playerId, GameMoney gameMoney)
{
    std::lock_guard<LockType> lock(lock_);

	TradeSlot* selfSlot = getTradeSlot(playerId);
	if (selfSlot == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	if (selfSlot->isConform_) {
		return ecTradeActivateConform;
	}

    TradeSlot* otherSlot = getOtherTradeSlot(playerId);
    if (! otherSlot) {
        return ecServerInternalError;
    }

    if (selfSlot->isReady_ && otherSlot->isReady_) {
        isCancel = true;
        return ecOk;
    }

    go::Entity* player = getTradePlayer(playerId);
    assert(player);
    if (player->queryMoneyable()->getGameMoney() < gameMoney) {
        return ecTradeNotEnoughTradeGameMoney;
    }

	if (selfSlot->isReady_) {
		if (otherSlot->canTradeItem()) {
			otherSlot->isConform_ = false;
		}
		selfSlot->isReady_ = false;
	}
	else {
		selfSlot->isReady_ = true;
	}

    if (selfSlot->isReady_) {
        selfSlot->gameMoney_ = gameMoney;
    }
    else {
        selfSlot->gameMoney_ = 0;
    }
    
	isReady = selfSlot->isReady_;

	go::Entity* orderPlayer = getOrderTradePlayer(playerId);
	assert(orderPlayer);
	gc::TradeCallback* callback = orderPlayer->getController().queryTradeCallback();	
	if (callback) {
		callback->tradeReadyToggled(isReady, selfSlot->gameMoney_);
	}

	return ecOk;
}


ErrorCode Trade::toggleConform(bool& isCompleted, bool& isConform, ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    isCompleted = false;
	TradeSlot* selfSlot = getTradeSlot(playerId);
	if (selfSlot == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	if (! selfSlot->isReady_) {
		return ecTradeInactiveReady;
	}

	isConform = ! (selfSlot->isConform_);

	if (isConform) {
		TradeSlot* otherSlot = getOtherTradeSlot(playerId);
		if (otherSlot->canTradeItem()) {
			ErrorCode errorCode  = checkEnoughInventory(*selfSlot, *otherSlot);
			if (isFailed(errorCode)) {
				return errorCode;
			}
			tradeItems();
			isCompleted = true;			
		}
	}
	
	selfSlot->isConform_ = isConform;
		
	go::Entity* orderPlayer = getOrderTradePlayer(playerId);
	gc::TradeCallback* callback = orderPlayer->getController().queryTradeCallback();
	if (callback) {
		if (isCompleted) {
			callback->tradeCompleted(tradeId_);
		}
		else {
			callback->tradeConformToggled(isConform);
		}
	}	

	return ecOk;
}


ErrorCode Trade::addItem(ObjectId itemId, ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

	TradeSlot* selfSlot = getTradeSlot(playerId);
	if (selfSlot == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	if (selfSlot->isReady_) {
		return ecTradeReadyCannotTradeItemsChange;
	}
    
    const int maxItemCount = 20;
    if (maxItemCount <= selfSlot->tradeItemMap_.size()) {
        return ecTradeMaxOverTradeItem;
    }

	if (selfSlot->isResistItem(itemId)) {
		return ecTradeAlreadyRegistItem;
	}	

	ItemInfo* itemInfo = selfSlot->inventoryInfo_.getItemInfo(itemId);
	if (itemInfo == nullptr) {
		return ecItemNotExist;
	}

	if(itemInfo->isEquipped()) {
		return ecTradeCannotTradeEquipped;
	}

	go::Entity* orderPlayer = getOrderTradePlayer(playerId);
	assert(orderPlayer);
	gc::TradeCallback* callback = orderPlayer->getController().queryTradeCallback();
	assert(callback);

	selfSlot->insertTradeItem(itemInfo);
	callback->tradeItemAdded(*itemInfo);	
	
	return ecOk;
}


ErrorCode Trade::removeItem(ObjectId itemId, ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

	TradeSlot* selfSlot = getTradeSlot(playerId);
	if (selfSlot == nullptr) {
		return ecTradeInvalidTradeInfo;
	}

	if (selfSlot->isReady_) {
		return ecTradeReadyCannotTradeItemsChange;
	}

	if (! selfSlot->isResistItem(itemId)) {
		return ecTradeNotRegistItem;
	}

	selfSlot->removeTradeItem(itemId);
	go::Entity* orderPlayer = getOrderTradePlayer(playerId);
	assert(orderPlayer);
	gc::TradeCallback* callback = orderPlayer->getController().queryTradeCallback();
	assert(callback);
	callback->tradeItemRemoved(itemId);

	return ecOk;
}


ErrorCode Trade::swapItem(ObjectId invenItemId, ObjectId tradeSlotItemId, ObjectId playerId)
{
    TradeSlot* selfSlot = getTradeSlot(playerId);
    if (selfSlot == nullptr) {
        return ecTradeInvalidTradeInfo;
    }

    if (selfSlot->isReady_) {
        return ecTradeReadyCannotTradeItemsChange;
    }

    if (! selfSlot->isResistItem(tradeSlotItemId)) {
        return ecTradeNotRegistItem;
    }

    ItemInfo* itemInfo = selfSlot->inventoryInfo_.getItemInfo(invenItemId);
    if (itemInfo == nullptr) {
        return ecItemNotExist;
    }

    selfSlot->removeTradeItem(tradeSlotItemId);

    go::Entity* orderPlayer = getOrderTradePlayer(playerId);
    assert(orderPlayer);
    gc::TradeCallback* callback = orderPlayer->getController().queryTradeCallback();
    assert(callback);
    callback->tradeItemRemoved(tradeSlotItemId);

    selfSlot->insertTradeItem(itemInfo);
    callback->tradeItemAdded(*itemInfo);
    
    return ecOk;
}


bool Trade::isExistPlayer(ObjectId  playerId) const
{
	return tradePlayerMap_.find(playerId) != tradePlayerMap_.end();
}


TradeSlot* Trade::getOtherTradeSlot(ObjectId  playerId)
{
	PlayerTradeSlotMap::iterator pos = playerTradeSlot_.begin();
	for (; pos != playerTradeSlot_.end(); ++pos) {
		if ((*pos).first != playerId) {
			return (*pos).second;
		}
	}
	return nullptr;
}


TradeSlot* Trade::getTradeSlot(ObjectId  playerId)
{
	PlayerTradeSlotMap::iterator pos = playerTradeSlot_.begin();
	for (; pos != playerTradeSlot_.end(); ++pos) {
		if ((*pos).first == playerId) {
			return (*pos).second;
		}
	}
	return nullptr;
}


go::Entity* Trade::getTradePlayer(ObjectId  playerId)
{
	TradePlayerMap::iterator pos = tradePlayerMap_.begin();
	for (; pos != tradePlayerMap_.end(); ++pos) {
		if ((*pos).first == playerId) {
			return (*pos).second;
		}
	}
	return nullptr;
}


go::Entity* Trade::getOrderTradePlayer(ObjectId  playerId)
{
	TradePlayerMap::iterator pos = tradePlayerMap_.begin();
	for (; pos != tradePlayerMap_.end(); ++pos) {
		if ((*pos).first != playerId) {
			return (*pos).second;
		}
	}
	return nullptr;
}


ErrorCode Trade::checkEnoughInventory(TradeSlot& selfSlot, TradeSlot& otherSlot)
{
	InventoryInfo selfInventory = selfSlot.inventoryInfo_;
	InventoryInfo otherInventory = otherSlot.inventoryInfo_;

	removeInventoryItems(selfInventory, selfSlot);
	removeInventoryItems(otherInventory, otherSlot);

	if (! addInventoryItems(selfInventory, otherSlot)) {
		return ecTradeSelfNotEnoughInventory;
	}

	if (! addInventoryItems(otherInventory, selfSlot)) {
		return ecTradeOrderNotEnoughInventory;
	}

	return ecOk;
}


void Trade::tradeItems()
{
	for (TradePlayerMap::value_type& playerValue : tradePlayerMap_) {
		ObjectId playerId = playerValue.first;
		go::Entity* player = playerValue.second;
		Inventory& invetory= player->queryInventoryable()->getInventory();
		TradeSlot* selfSlot = getTradeSlot(playerId);
		TradeSlot* otherSlot = getOtherTradeSlot(playerId);

		for (TradeSlot::TradeItemMap::value_type& selfItemValue : selfSlot->tradeItemMap_) {
			invetory.removeItem(selfItemValue.first);		
		}
		for (TradeSlot::TradeItemMap::value_type& otherItemValue : otherSlot->tradeItemMap_) {
			ItemInfo* itemInfo = otherItemValue.second;
			invetory.addItem(createAddItemInfoByItemInfo(*itemInfo), invalidSlotId);
		}
		if (selfSlot->gameMoney_ > 0) {
			player->queryMoneyable()->downGameMoney(selfSlot->gameMoney_);
		}	
		if (otherSlot->gameMoney_ > 0) {
			player->queryMoneyable()->upGameMoney(otherSlot->gameMoney_);
		}
	}
}

}}  //namespace gideon { namespace zoneserver {
