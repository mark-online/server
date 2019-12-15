#include "ZoneServerPCH.h"
#include "LootItemInventory.h"
#include "Inventory.h"
#include "../gameobject/EntityEvent.h"
#include "../gameobject/ability/Castable.h"
#include "../gameobject/ability/CastNotificationable.h"
#include "../gameobject/ability/SOInventoryable.h"
#include "../gameobject/ability/Inventoryable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../service/item/ItemIdGenerator.h"
#include "../../service/item/GraveStoneService.h"
#include "../../service/time/GameTimer.h"
#include "../../world/WorldMap.h"
#include "../../helper/InventoryHelper.h"
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <sne/base/concurrent/TaskScheduler.h>


namespace gideon { namespace zoneserver {

namespace {

/**
 * @class RewardTask
 */
class RewardTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<RewardTask>
{
public:
    RewardTask(go::Entity& player, go::Entity& owner, LootInvenId itemId) :
        player_(player),
        owner_(owner),
		itemId_(itemId) {}

private:
    virtual void run() {
		go::SOInventoryable* soInventoryable = owner_.querySOInventoryable();
		if (! soInventoryable) {
			return;
		}
        const ErrorCode errorCode = soInventoryable->completeLooting(player_, itemId_);
        if (isFailed(errorCode)) {
			go::CastNotificationable* notificationable = player_.queryCastNotificationable();
			if (notificationable) {
				FailCompletedCastResultInfo failInfo;
				failInfo.setDataInfo(uctItemLooting, errorCode,
					player_.getGameObjectInfo(), invalidDataCode);
				notificationable->notifyCompleteFailedCasting(failInfo);
			}
        }
    }

private:
    go::Entity& player_;
	go::Entity& owner_;
	LootInvenId itemId_;    
};

} // namespace {


LootItemInventory::LootItemInventory(go::Entity& owner, 
	const LootInvenItemInfoMap& ItemInfoMap) :
	owner_(owner),
	ItemInfoMap_(ItemInfoMap)
{
}



ErrorCode LootItemInventory::startLooting(go::Entity& player, LootInvenId graveStoneItemId)
{
	GameTime lootingDelay = 0;
    DataCode itemCode = invalidDataCode;
	{
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

		const LootInvenItemInfo* lootingItem = getItemInfo(graveStoneItemId);
		if (! lootingItem) {
			return ecItemNotExist;
		}
		lootingDelay = getLootingDelay(lootingItem->type_, lootingItem->baseItemInfo_.itemCode_);
        itemCode = lootingItem->baseItemInfo_.itemCode_;
	}
	

	go::Castable* castable = player.queryCastable();
	go::CastNotificationable* notificationable = player.queryCastNotificationable();
	if (! castable & ! notificationable) {
		return ecServerInternalError;
	}

	auto task = std::make_unique<RewardTask>(player, owner_, graveStoneItemId);
	castable->startCasting(invalidDataCode, false, std::move(task), lootingDelay);
	StartCastResultInfo startCastInfo;
	startCastInfo.set(uctItemLooting, player.getGameObjectInfo(), owner_.getGameObjectInfo(), itemCode);
	notificationable->notifyStartCasting(startCastInfo);

	return ecOk;
}


ErrorCode LootItemInventory::completeLooting(go::Entity& player, LootInvenId graveStoneItemId)
{
	go::Castable* castable = player.queryCastable();
	if (castable) {
		castable->releaseCastState(false);
	}
	LootInvenItemInfo stoneInvenItemInfo;
	{
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

		const LootInvenItemInfo* stoneItem = getItemInfo(graveStoneItemId);
		if (stoneItem == nullptr) {
			return ecItemNotExist;
		}
		stoneInvenItemInfo = *stoneItem;
	}
	
	ErrorCode errorCode = ecOk;
	if (stoneInvenItemInfo.type_ == liitGameMoney) {
		gc::RewardCallback* callback = player.getController().queryRewardCallback();	
		if (callback) {
			callback->gameMoneyRewarded(stoneInvenItemInfo.money_, true);
		}
	}
	else if (stoneInvenItemInfo.type_ == liitExp) {
		gc::RewardCallback* callback = player.getController().queryRewardCallback();	
		if (callback) {
			callback->expRewarded(stoneInvenItemInfo.exp_);
		}
	}
	else if (stoneInvenItemInfo.type_ == liitItem) {
        AddItemInfo itemInfo(stoneInvenItemInfo.baseItemInfo_.itemCode_, stoneInvenItemInfo.baseItemInfo_.count_);
        if (stoneInvenItemInfo.baseItemInfo_.isEquipment()) {
            itemInfo.equipItemInfo_ = stoneInvenItemInfo.equipItemInfo_;
        }
        else if (stoneInvenItemInfo.baseItemInfo_.isAccessory()) {
            itemInfo.accessoryItemInfo_ = stoneInvenItemInfo.accessoryInfo_;
        }
        errorCode = player.queryInventoryable()->getInventory().addItem(itemInfo);	
	}

	if (isFailed(errorCode)) {
		return errorCode;
	}

	{
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        ItemInfoMap_.erase(graveStoneItemId);
	}

	go::CastNotificationable* notificationable = player.queryCastNotificationable();
	CompleteCastResultInfo comleteInfo;
	comleteInfo.set(player.getGameObjectInfo(), owner_.getGameObjectInfo(), graveStoneItemId);
	notificationable->notifyCompleteCasting(comleteInfo);	

	return ecOk;
}


const LootInvenItemInfo* LootItemInventory::getItemInfo(LootInvenId lootInvenId) const 
{
	const LootInvenItemInfoMap::const_iterator pos = ItemInfoMap_.find(lootInvenId);
	if (pos != ItemInfoMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}

}}// namespace gideon { namespace zoneserver 
