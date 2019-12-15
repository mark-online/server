#pragma once

#include "AbstractAnchor.h"
#include "ability/Buildable.h"
#include "ability/OutsideInventoryable.h"
#include "ability/ConsumeItemable.h"
#include "ability/SelectRecipeProductionable.h"
#include "ability/BuildingProductionInventoryable.h"
#include "ability/Bankable.h"
#include "ability/NpcHireable.h"
#include "ability/Thinkable.h"
#include <gideon/cs/shared/data/BindRecallInfo.h>
#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace datatable {
struct ResourcesProductionInfo;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

class Inventory;
class BuildingMarkCallback;

namespace ai {
class BuildingStateBrain;
} // namespace ai {

}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

class Knowable;
class BuildingState;

/**
 * @class Building
 * 조정 가능한 설치물
 */
class Building : public AbstractAnchor,
    private Buildable,
    private OutsideInventoryable,
    private SelectRecipeProductionable,
    private BuildingProductionInventoryable,
    private Bankable,
    private NpcHireable,
    private Thinkable
{
    typedef AbstractAnchor Parent;
    typedef sne::core::Map<BuildingStateType, BuildingState*> BuildingStateMap;
    
public:
    Building(std::unique_ptr<gc::EntityController> controller,
        BuildingMarkCallback& markCallback);
    virtual ~Building();

    bool initialize(DataCode dataCode, ObjectId objectId, ObjectId linkBuildingId,
        const ObjectPosition& position);
    bool initialize(ObjectId linkBuildingId, const BuildingInfo& buildingInfo);

    bool initializeGuard();

    ErrorCode fillBindRecallInfo(BindRecallInfo& bindRecallInfo, ObjectId playerId);

    SlotId getItemSlotId(InvenType invenType, ObjectId itemId) const;
      
    const BuildingInfo& getBuildingInfo() const {
        return buildingInfo_;
    }

    virtual bool isCompleteState() const {
        return buildingInfo_.isCompleteState();
    }

	bool isHalfBelowHp() const {
		HitPoint halfHp = toHitPoint(buildingInfo_.points_.maxHp_ / 2);
		return buildingInfo_.points_.hp_ < halfHp;
	}

    bool isCoreBuilding() const {
        return buildingTemplate_->isCoreBuilding();
    }

    ErrorCode toggleDoor(bool& isOpenDoor, go::Entity& player);

    ObjectId getLinkCoreBuildingId() const {
        return linkCoreBuildingId_;
    }

    void addLinkBuilding(ObjectId buildingId);
    void removeLinkBuilding(ObjectId buildingId);

private:
    virtual void finalize();

public:
    virtual ErrorCode downBuildTime(sec_t downSec);
    virtual ErrorCode addBuildingMaterials();
    virtual sec_t getExpireTime() const;
    virtual sec_t getInvincibleExpireTime() const;

    virtual void fillBuildingInfo(sec_t& expireIncibleCooltime,
        BuildingOwnerInfo& ownerInfo, BuildingItemTaskInfos& consumeTaskInfos,
        BuildingItemTaskInfos& resouceTaskInfos, BuildingItemTaskInfos& selectProductionTaskInfos);
    virtual ErrorCode canTeleport(go::Entity& player) const;

public:
    // =  EntityAbility overriding
    virtual Buildable* queryBuildable() {
        return this;
    }
    virtual OutsideInventoryable* queryOutsideInventoryable() {
        return this;
    }
    virtual SelectRecipeProductionable* querySelectRecipeProductionable() { return this; } 
    virtual BuildingProductionInventoryable* queryBuildingProductionInventoryable() { return this; }
    virtual Bankable* queryBankable() {return this;} 
    virtual NpcHireable* queryNpcHireable() {return this;}
    virtual Thinkable* queryThinkable() { return this; }


public:
    virtual float32_t getModelingRadiusSize() const {
        if (! buildingTemplate_) {
            return 0;
        }
        return buildingTemplate_->getModelingRadius();
    }

    // = Entity overrding
    virtual DataCode getEntityCode() const {
        return buildingInfo_.buildingCode_;
    }

private:
    // =  Buildable overriding
    virtual ErrorCode build(Entity& player);
    virtual void hiringNpcDespawned(ObjectId mercenaryId);
    virtual ObjectId getOwnerId() const;
    virtual GuildId getOwnerGuildId() const;

private:
    // = Bankable overriding
    virtual ErrorCode canBankable(go::Entity& player) const;

private:
    // = SelectRecipeProductionable overriding
    virtual ErrorCode startRecipeProduction(go::Entity& player, RecipeCode recipeCode, uint8_t count);

private:
    // = BuildingProductionInventoryable overriding
    virtual ErrorCode startLooting(Entity& player, ObjectId itemId, InvenType invenType);
    virtual ErrorCode completeLooting(Entity& player, ObjectId itemId, InvenType invenType);

private:
    // = OutsideInventoryable overriding
    virtual Inventory* getInventory(InvenType invenType);
    virtual const Inventory* getInventory(InvenType invenType) const;
    virtual ErrorCode checkInventoryUseable(InvenType invenType, go::Entity& player) const;
    virtual ErrorCode canAddItem(InvenType invenType, DataCode itemCode) const;
    virtual ErrorCode canOutItem(InvenType invenType, go::Entity& player) const;
    virtual bool isActiveAbillity() const;
	virtual bool canForgeFunction() const { return buildingTemplate_->canForgeFunction(); }

private:
    // = Thinkable overriding
    virtual bool hasWalkRoutes() const {
        return false;
    }
    virtual ai::Brain& getBrain() {
        return *brain_;
    }

private:
    // = StaticObject overriding
    virtual HitPoints& getCurrentPoints() {
        return buildingInfo_.points_;
    }

    virtual const HitPoints& getCurrentPoints() const {
        return buildingInfo_.points_;
    }

    virtual bool isInvincible() const {
        return buildingInfo_.isInvincible_;
    }
    virtual bool canDestory() const {
        return buildingTemplate_->canDestory();
    }
    //virtual AttributeRate getResistRate() const {
    //    if (isCompleteState()) {
    //        return buildingTemplate_->getCompleteResistRate();
    //    }
    //    return buildingTemplate_->getFoundationResistRate();
    //}

private:
    // = AbstractAnchor overriding
    
    virtual const datatable::AnchorSkillInfo& getSkillInfo(DataCode anchorSkillCode) const {        
        return buildingTemplate_->getSkillInfo(anchorSkillCode);
    }
    virtual ObjectPosition& getAnchorPosition() {
        return buildingInfo_.position_;
    }
    virtual const ObjectPosition& getAnchorPosition() const {
        return buildingInfo_.position_;
    }
    virtual DataCode getOwnerCode() const {
        return buildingInfo_.buildingCode_;
    }

	virtual void handleExpiredTasks();
	
private:
    // = NpcHireable overriding
	virtual ErrorCode buyBuildingGuard(Entity& player, BuildingGuardCode code, uint8_t rendDay);
    virtual ErrorCode spawnBuildingGuard(Entity& player, ObjectId mercenaryId, const ObjectPosition& position);

private:
    // = StaticObjectOwnerable overriding
    virtual const BuildingOwnerInfo& getBuildingOwnerInfo() const {
        return buildingInfo_.ownerInfo_;
    }

private:
    void initInventories();
    void initBuildingState();
    void initBuildingBrain();

	void saveHitPoint();

    void fillFoundationInfo(DataCode dataCode, ObjectId objectId,
        const ObjectPosition& position);

    ErrorCode spawnBuildingGuard(ObjectId mercenaryId, 
        NpcCode npcCode, msec_t lifeTime, const ObjectPosition& position);
    
private:
    bool isEnoughBuildingActionGrade(go::Entity& player) const;

private:
    const datatable::BuildingTemplate* buildingTemplate_;
    BuildingMarkCallback& markCallback_;
    BuildingInfo buildingInfo_;
    sec_t saveTime_;

    std::unique_ptr<Inventory> buildingInventory_;
    std::unique_ptr<Inventory> mercenaryInventory_;
    std::unique_ptr<Inventory> productionInventory_;
    std::unique_ptr<Inventory> selectProductionInventory_;
    std::unique_ptr<Inventory> fundationInventory_;

    std::unique_ptr<ai::Brain> brain_;

	BuildingStateMap buildingStateMap_;
	BuildingState* currentState_;
    ObjectIdSet linkBuildingIds_;
    ObjectId linkCoreBuildingId_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
