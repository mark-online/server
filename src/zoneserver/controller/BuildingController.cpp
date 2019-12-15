#include "ZoneServerPCH.h"
#include "BuildingController.h"
#include "callback/BuildCallback.h"
#include "callback/AnchorAbillityCallback.h"
#include "callback/BuildingProductionTaskCallback.h"
#include "../world/WorldMap.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/Building.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/ability/Inventoryable.h"
#include "../model/gameobject/EntityEvent.h"
#include "../service/anchor/AnchorService.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class InventoryItemMovedEvent
 */
class InventoryItemMovedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<InventoryItemMovedEvent>
{
public:
    InventoryItemMovedEvent(InvenType invenType,
        ObjectId itemId, SlotId slotId) :
        invenType_(invenType),
        itemId_(itemId), 
        slotId_(slotId) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        if (! entity.queryInventoryable()->isOpenOutsideInventory(invenType_)) {
            return;   
        }
		InventoryCallback* inventoryCallback =
			entity.getController().queryInventoryCallback();
		if (inventoryCallback) {
			inventoryCallback->inventoryItemMoved(invenType_, itemId_, slotId_, false);
		}
    }

private:
    InvenType invenType_;
    ObjectId itemId_; 
    SlotId slotId_;
};


/**
 * @class InventoryItemSwitchedEvent
 */
class InventoryItemSwitchedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<InventoryItemSwitchedEvent>
{
public:
    InventoryItemSwitchedEvent(InvenType invenType, ObjectId itemId1,
        ObjectId itemId2) :
        invenType_(invenType),
        itemId1_(itemId1),
        itemId2_(itemId2)
    {
    }

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        if (! entity.queryInventoryable()->isOpenOutsideInventory(invenType_)) {
            return;   
        }
		InventoryCallback* inventoryCallback =
			entity.getController().queryInventoryCallback();
		if (inventoryCallback) {
			inventoryCallback->inventoryItemSwitched(invenType_, itemId1_, itemId2_, false);
		}
    }

private:
    InvenType invenType_;
    ObjectId itemId1_;
    ObjectId itemId2_;
};


/**
 * @class InventoryItemAddedEvent
 */
class InventoryItemAddedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<InventoryItemAddedEvent>
{
public:
    InventoryItemAddedEvent(InvenType invenType, const ItemInfo& itemInfo) :
        invenType_(invenType),
        itemInfo_(itemInfo)
    {
    }

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        if (! entity.queryInventoryable()->isOpenOutsideInventory(invenType_)) {
            return;   
        }
		InventoryCallback* inventoryCallback =
			entity.getController().queryInventoryCallback();
		if (inventoryCallback) {
			inventoryCallback->inventoryItemAdded(invenType_, itemInfo_, false);
		}
    }

private:
    InvenType invenType_;
    const ItemInfo itemInfo_;
};


/**
 * @class InventoryItemRemovedEvent
 */
class InventoryItemRemovedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<InventoryItemRemovedEvent>
{
public:
    InventoryItemRemovedEvent(InvenType invenType, ObjectId itemId) :
        invenType_(invenType),
        itemId_(itemId)
    {
    }

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        if (! entity.queryInventoryable()->isOpenOutsideInventory(invenType_)) {
            return;   
        }
		InventoryCallback* inventoryCallback =
			entity.getController().queryInventoryCallback();
		if (inventoryCallback) {
			inventoryCallback->inventoryItemRemoved(invenType_, itemId_, false);
		}
    }

private:
    InvenType invenType_;
    ObjectId itemId_;
};


/**
 * @class InventoryItemCountUpdatedEvent
 */
class InventoryItemCountUpdatedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<InventoryItemCountUpdatedEvent>
{
public:
    InventoryItemCountUpdatedEvent(InvenType invenType, ObjectId itemId, uint8_t itemCount) :
        invenType_(invenType),
        itemId_(itemId),
        itemCount_(itemCount)
    {
    }

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        if (! entity.queryInventoryable()->isOpenOutsideInventory(invenType_)) {
            return;   
        }
		InventoryCallback* inventoryCallback =
			entity.getController().queryInventoryCallback();
		if (inventoryCallback) {
			inventoryCallback->inventoryItemCountUpdated(invenType_, itemId_, itemCount_, false);
		}
    }

private:
    InvenType invenType_;
    ObjectId itemId_;
    uint8_t itemCount_;
};


/**
 * @class BuildingStartBuildEvent
 */
class BuildingStartBuildEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingStartBuildEvent>
{
public:
    BuildingStartBuildEvent(const GameObjectInfo& buildingInfo,
        sec_t startBuildTime) :
        buildingInfo_(buildingInfo),
		startBuildTime_(startBuildTime) {}

private:
    virtual void call(go::Entity& entity) {
		BuildCallback* buildCallback =
			entity.getController().queryBuildCallback();
		if (buildCallback) {
			buildCallback->buildingStartBuilt(buildingInfo_, startBuildTime_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
	const sec_t startBuildTime_;
};


/**
 * @class BuildingCompleteEvent
 */
class BuildingCompleteEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingCompleteEvent>
{
public:
    BuildingCompleteEvent(const GameObjectInfo& buildingInfo) :
        buildingInfo_(buildingInfo) {}

private:
    virtual void call(go::Entity& entity) {
		BuildCallback* buildCallback =
			entity.getController().queryBuildCallback();
		if (buildCallback) {
			buildCallback->buildingCompleted(buildingInfo_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;	
};

/**
 * @class BuildingToFoundationRevertEvent
 */
class BuildingToFoundationRevertEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingToFoundationRevertEvent>
{
public:
    BuildingToFoundationRevertEvent(const GameObjectInfo& buildingInfo) :
        buildingInfo_(buildingInfo){}

private:
    virtual void call(go::Entity& entity) {
		BuildCallback* buildCallback =
			entity.getController().queryBuildCallback();
		if (buildCallback) {
			buildCallback->buildingToFoundationReverted(buildingInfo_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
};


/**
 * @class BuildingBrokenEvent
 */
class BuildingBrokenEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingBrokenEvent>
{
public:
    BuildingBrokenEvent(const GameObjectInfo& buildingInfo) :
        buildingInfo_(buildingInfo) {}

private:
    virtual void call(go::Entity& entity) {
		BuildCallback* buildCallback =
			entity.getController().queryBuildCallback();
		if (buildCallback) {
			buildCallback->buildingBroken(buildingInfo_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
};


/**
 * @class BuildingDestoryedEvent
 */
class BuildingDestoryedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingDestoryedEvent>
{
public:
    BuildingDestoryedEvent(const GameObjectInfo& buildingInfo) :
        buildingInfo_(buildingInfo) {}

private:
    virtual void call(go::Entity& entity) {
		BuildCallback* buildCallback =
			entity.getController().queryBuildCallback();
		if (buildCallback) {
			buildCallback->buildingDestoryed(buildingInfo_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
};


/**
 * @class BuildingInvincibleStateChangedEvent
 */
class BuildingInvincibleStateChangedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingInvincibleStateChangedEvent>
{
public:
    BuildingInvincibleStateChangedEvent(const GameObjectInfo& buildingInfo, bool isActivate) :
        buildingInfo_(buildingInfo),
        isActivate_(isActivate) {}

private:
    virtual void call(go::Entity& entity) {
		BuildCallback* buildCallback =
			entity.getController().queryBuildCallback();
		if (buildCallback) {
			buildCallback->buildingInvincibleStateChanged(buildingInfo_, isActivate_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
    bool isActivate_;
};


/**
 * @class BuildingDoorToggleEvent
 */
class BuildingDoorToggleEvent: public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingDoorToggleEvent>
{
public:
    BuildingDoorToggleEvent(const GameObjectInfo& buildingInfo, bool isOpen) :
        buildingInfo_(buildingInfo),
        isOpen_(isOpen) {}

private:
    virtual void call(go::Entity& entity) {
		BuildCallback* buildCallback =
			entity.getController().queryBuildCallback();
		if (buildCallback) {
			buildCallback->buildingDoorToggled(buildingInfo_, isOpen_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
    bool isOpen_;
};

/**
 * @class AddSelectProductionTaskEvent
 */
class AddSelectProductionTaskEvent: public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<AddSelectProductionTaskEvent>
{
public:
    AddSelectProductionTaskEvent(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& info) :
        buildingInfo_(buildingInfo),
        info_(info) {}

private:
    virtual void call(go::Entity& entity) {
		BuildingProductionTaskCallback* callback =
			entity.getController().queryBuildingProductionTaskCallback();
		if (callback) {
			callback->buildingSelectRecipeProductionAdded(buildingInfo_, info_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
    BuildingItemTaskInfo info_;
};

/**
 * @class UpdateSelectProductionTaskEvent
 */
class UpdateSelectProductionTaskEvent: public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<UpdateSelectProductionTaskEvent>
{
public:
    UpdateSelectProductionTaskEvent(const GameObjectInfo& buildingInfo, const BuildingItemTaskInfo& info) :
      buildingInfo_(buildingInfo),
          info_(info) {}

private:
    virtual void call(go::Entity& entity) {
        BuildingProductionTaskCallback* callback =
            entity.getController().queryBuildingProductionTaskCallback();
        if (callback) {
            callback->buildingSelectRecipeProductionUpdated(buildingInfo_, info_);
        }
    }

private:
    const GameObjectInfo buildingInfo_;
    BuildingItemTaskInfo info_;
};


/**
 * @class RemoveSelectProductionTaskEvent
 */
class RemoveSelectProductionTaskEvent: public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<RemoveSelectProductionTaskEvent>
{
public:
    RemoveSelectProductionTaskEvent(const GameObjectInfo& buildingInfo, RecipeCode recipeCode) :
        buildingInfo_(buildingInfo),
        recipeCode_(recipeCode) {}

private:
    virtual void call(go::Entity& entity) {
		BuildingProductionTaskCallback* callback =
			entity.getController().queryBuildingProductionTaskCallback();
		if (callback) {
			callback->buildingSelectRecipeProductionRemoved(buildingInfo_, recipeCode_);
		}
    }

private:
    const GameObjectInfo buildingInfo_;
    RecipeCode recipeCode_;
};




} // namespace

// = StaticObjectEffectCallback override

void BuildingController::inventoryItemMoved(InvenType invenType, ObjectId itemId, SlotId slotId, bool /*dbSave*/)
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<InventoryItemMovedEvent>(invenType, itemId, slotId);
    owner.notifyToObervers(event, true);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncMoveBuildingInventoryItem(owner.getObjectId(), invenType, itemId, slotId);
}


void BuildingController::inventoryItemSwitched(InvenType invenType, ObjectId itemId1, ObjectId itemId2, bool  /*dbSave*/)
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<InventoryItemSwitchedEvent>(invenType, itemId1, itemId2);
    SlotId slotId1 = owner.getItemSlotId(invenType, itemId1);
    SlotId slotId2 = owner.getItemSlotId(invenType, itemId2);
    owner.notifyToObervers(event, true);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncSwitchBuildingInventoryItem(owner.getObjectId(), invenType, itemId1, itemId2, slotId1, slotId2);  
}


void BuildingController::inventoryItemAdded(InvenType invenType, const ItemInfo& itemInfo, bool  /*dbSave*/)
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<InventoryItemAddedEvent>(invenType, itemInfo);
    owner.notifyToObervers(event, true);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAddBuildingInventoryItem(owner.getObjectId(), invenType, itemInfo);
}


void BuildingController::inventoryItemRemoved(InvenType invenType, ObjectId itemId, bool  /*dbSave*/)
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<InventoryItemRemovedEvent>(invenType, itemId);
    owner.notifyToObervers(event, true);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncRemoveBuildingInventoryItem(owner.getObjectId(), invenType, itemId);
}


void BuildingController::inventoryItemCountUpdated(InvenType invenType, ObjectId itemId, uint8_t itemCount, bool  /*dbSave*/)
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<InventoryItemCountUpdatedEvent>(invenType, itemId, itemCount);
    owner.notifyToObervers(event, true);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUpdateBuildingInventoryItemCount(owner.getObjectId(), invenType, itemId, itemCount);
}


void BuildingController::inventoryInfoUpdated(InvenType /*invenType*/, bool /*isCashSlot*/, uint8_t /*count*/, bool /*dbSave*/)
{
    assert(false && "not call");
}


void BuildingController::inventoryItemAllRemoved(InvenType invenType)
{
    go::Entity& owner = getOwner();
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncRemoveAllBuildingInventoryItem(owner.getObjectId(), invenType);
}

// = BuildingStateCallback overriding

void BuildingController::buildingCreated()
{
    go::Building& owner = getOwnerAs<go::Building>();

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncCreateBuilding(owner.getBuildingInfo());
}


void BuildingController::buildingToFoundationReverted()
{
    go::Building& owner = getOwnerAs<go::Building>();
   
    auto event = std::make_shared<BuildingToFoundationRevertEvent>(owner.getGameObjectInfo());
    owner.queryKnowable()->broadcast(event);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    const BuildingInfo& buildingInfo = owner.getBuildingInfo();
    db->asyncUpdateBuildingState(buildingInfo.objectId_, buildingInfo.state_, buildingInfo.startBuildTime_, buildingInfo.fundationExpireTime_,
        buildingInfo.points_.hp_);

}


void BuildingController::buildingStartBuilt(sec_t startTime)
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<BuildingStartBuildEvent>(owner.getGameObjectInfo(), startTime);
    owner.queryKnowable()->broadcast(event);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    const BuildingInfo& buildingInfo = owner.getBuildingInfo();
    db->asyncUpdateBuildingState(buildingInfo.objectId_, buildingInfo.state_, buildingInfo.startBuildTime_, buildingInfo.fundationExpireTime_,
        buildingInfo.points_.hp_);
}


void BuildingController::buildCurrentStateSaved()
{
    go::Building& owner = getOwnerAs<go::Building>();
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    const BuildingInfo& buildingInfo = owner.getBuildingInfo();
    db->asyncUpdateBuildingState(buildingInfo.objectId_, buildingInfo.state_, buildingInfo.startBuildTime_, buildingInfo.fundationExpireTime_,
        buildingInfo.points_.hp_);
}


void BuildingController::buildCompleted()
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<BuildingCompleteEvent>(owner.getGameObjectInfo());
    owner.queryKnowable()->broadcast(event);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    const BuildingInfo& buildingInfo = owner.getBuildingInfo();
    db->asyncUpdateBuildingState(buildingInfo.objectId_, buildingInfo.state_, buildingInfo.startBuildTime_, buildingInfo.fundationExpireTime_,
        buildingInfo.points_.hp_);
}


void BuildingController::buildingBroken()
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<BuildingBrokenEvent>(owner.getGameObjectInfo());
    owner.queryKnowable()->broadcast(event);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    const BuildingInfo& buildingInfo = owner.getBuildingInfo();
    db->asyncUpdateBuildingState(buildingInfo.objectId_, buildingInfo.state_, buildingInfo.startBuildTime_, buildingInfo.fundationExpireTime_,
        buildingInfo.points_.hp_);
}


void BuildingController::buildDestroyed()
{
    go::Building& owner = getOwnerAs<go::Building>();
    //auto event = std::make_shared<BuildingBrokenEvent>(owner.getGameObjectInfo());
    //owner.queryKnowable()->broadcast(event);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncDeleteBuilding(owner.getObjectId());
}


void BuildingController::buildOwnerUpdated()
{
   go::Building& owner = getOwnerAs<go::Building>();
   DatabaseGuard db(SNE_DATABASE_MANAGER);
   const BuildingInfo& buildingInfo = owner.getBuildingInfo();
   db->asyncUpdateBuildingOwner(buildingInfo.objectId_, buildingInfo.ownerInfo_.ownerType_,
       buildingInfo.ownerInfo_.playerOwnerInfo_.objectId_, buildingInfo.ownerInfo_.guildInfo_.guildId_);
}


void BuildingController::buildingInvincibleStateChanged(bool isActivate)
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<BuildingInvincibleStateChangedEvent>(
        owner.getGameObjectInfo(), isActivate);
    owner.queryKnowable()->broadcast(event);
}


ErrorCode BuildingController::toggleDoor(go::Entity& player)
{
    go::Building& owner = getOwnerAs<go::Building>();
    bool isOpenDoor = false;
    const ErrorCode errorCode = owner.toggleDoor(isOpenDoor, player);
    if (isSucceeded(errorCode)) {
        auto event = std::make_shared<BuildingDoorToggleEvent>(owner.getGameObjectInfo(), isOpenDoor);
        owner.queryKnowable()->broadcast(event);
    }
    return errorCode;
}

// = SelectProductionCallback overriding

void BuildingController::selectRecipeProductionAdded(RecipeCode recipeCode, uint8_t count, sec_t completeTime)
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<AddSelectProductionTaskEvent>(owner.getGameObjectInfo(),
        BuildingItemTaskInfo(BaseItemInfo(recipeCode, count), completeTime));
    owner.notifyToObervers(event, true);
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAddSelectRecipeProduction(owner.getObjectId(), recipeCode, count, completeTime);
}


void BuildingController::selectRecipeProductionRemoved(RecipeCode recipeCode)
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<RemoveSelectProductionTaskEvent>(
        owner.getGameObjectInfo(), recipeCode);
    owner.notifyToObervers(event, true);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncRemoveSelectRecipeProduction(owner.getObjectId(), recipeCode);
}


void BuildingController::selectRecipeProductionUpdated(RecipeCode recipeCode, uint8_t count, sec_t completeTime)
{
    go::Building& owner = getOwnerAs<go::Building>();
    auto event = std::make_shared<UpdateSelectProductionTaskEvent>(owner.getGameObjectInfo(),
        BuildingItemTaskInfo(BaseItemInfo(recipeCode, count), completeTime));
    owner.notifyToObervers(event, true);

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUpdateSelectRecipeProduction(owner.getObjectId(), recipeCode, count, completeTime);
}

}}} // namespace gideon { namespace zoneserver { namespace gc {

