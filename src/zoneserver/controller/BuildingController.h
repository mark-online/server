#pragma once

#include "AbstractAnchorController.h"
#include "callback/BuildingStateCallback.h"
#include "callback/InventoryCallback.h"
#include "callback/SelectProductionCallback.h"


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class AnchorController
 * Controller for Anchor
 */
class ZoneServer_Export BuildingController : public AbstractAnchorController,
    public BuildingStateCallback,
    public InventoryCallback,
    public SelectProductionCallback
{
protected:
    virtual BuildingStateCallback* queryBuildingStateCallback() { return this;}
    virtual InventoryCallback* queryInventoryCallback() {return this;}
    virtual SelectProductionCallback* querySelectProductionCallback() {
        return this;
    }
private:
    // = StaticObjectEffectCallback override
    virtual void inventoryItemMoved(InvenType invenType, ObjectId itemId, SlotId slotId, bool dbSave = true);
    virtual void inventoryItemSwitched(InvenType invenType, ObjectId itemId1, ObjectId itemId2, bool dbSave = true);
    virtual void inventoryItemAdded(InvenType invenType, const ItemInfo& itemInfo, bool dbSave = true);
    virtual void inventoryItemRemoved(InvenType invenType, ObjectId itemId, bool dbSave = true);
    virtual void inventoryItemCountUpdated(InvenType invenType, ObjectId itemId, uint8_t itemCount, bool dbSave = true);
    virtual void inventoryInfoUpdated(InvenType invenType, bool isCashSlot, uint8_t count, bool dbSave = true);
    virtual void inventoryItemAllRemoved(InvenType invenType);

private:
    // = BuildingStateCallback overriding
    virtual void buildingCreated();
    virtual void buildingToFoundationReverted();
    virtual void buildingStartBuilt(sec_t startTime);
    virtual void buildCompleted();
    virtual void buildingBroken();
    virtual void buildDestroyed();
    virtual void buildOwnerUpdated();
    virtual void buildingInvincibleStateChanged(bool isActivate);
    virtual void buildCurrentStateSaved();

    virtual ErrorCode toggleDoor(go::Entity& player);

private:
    // = SelectProductionCallback overriding
    virtual void selectRecipeProductionAdded(RecipeCode recipeCode, uint8_t count, sec_t completeTime);
    virtual void selectRecipeProductionRemoved(RecipeCode recipeCode);
    virtual void selectRecipeProductionUpdated(RecipeCode recipeCode, uint8_t count, sec_t completeTime);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {