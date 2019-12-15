#include "ZoneServerPCH.h"
#include "BuildingCompleteState.h"
#include "../Building.h"
#include "../ability/OutsideInventoryable.h"
#include "../ability/impl/SkillCastableAbility.h"
#include "../ability/Summonable.h"
#include "../status/StaticObjectStatus.h"
#include "../../item/Inventory.h"
#include "../../../s2s/ZoneCommunityServerProxy.h"
#include "../../../world/WorldMap.h"
#include "../../../world/World.h"
#include "../../../ZoneService.h"
#include "../../../helper/InventoryHelper.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/BuildingStateCallback.h"
#include "../../../controller/callback/BuildCallback.h"
#include "../../../controller/callback/SelectProductionCallback.h"
#include <gideon/cs/datatable/SelectRecipeProductionTable.h>
#include <gideon/cs/datatable/RecipeTable.h>


namespace gideon { namespace zoneserver { namespace go {

namespace {


inline ErrorCode checkSelectRecipeProduction(uint32_t selectCode, RecipeCode recipeCode, uint8_t count)
{
    if (count == 0) {
        return ecAnchorMinCountCreateSelectRecipe;
    }

    const datatable::SelectRecipeProductionInfos* infos = 
        SELECT_RECIPE_PRODUCTION_TABLE->getSelectRecipeProductionInfos(selectCode);
    if (! infos) {
        return ecAnchorNotSelectProduction;
    }

    for (const datatable::SelectRecipeProductionInfo& info : *infos) {
        if (info.recipeCode_ == recipeCode) {
            if (info.productionCreateCount_ <  count) {
                return ecAnchorOverCountCreateSelectRecipe;
            }
            return ecOk;
        }
    }

    return ecAnchorHasNotSelectRecipe; 
}

/**
 * @class BuildingActivateEvent
 */
class BuildingActivateEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingActivateEvent>
{
public:
    BuildingActivateEvent(const GameObjectInfo& objectInfo, bool isActivate) :
        objectInfo_(objectInfo),
		isActivate_(isActivate) {}

private:
    virtual void call(go::Entity& entity) {
        gc::BuildCallback * callback =
            entity.getController().queryBuildCallback();
        if (callback != nullptr) {
            callback->buildingActivateStateChanged(objectInfo_, isActivate_);
        }
    }

private:
    const GameObjectInfo& objectInfo_;
	bool isActivate_;
};


/**
 * @class BuildingInInvincibleEvent
 */
class BuildingInInvincibleEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingInInvincibleEvent>
{
public:
    BuildingInInvincibleEvent(const GameObjectInfo& objectInfo, bool isActivate) :
        objectInfo_(objectInfo),
		isActivate_(isActivate) {}

private:
    virtual void call(go::Entity& entity) {
        gc::BuildCallback * callback =
            entity.getController().queryBuildCallback();
        if (callback != nullptr) {
            callback->buildingInvincibleStateChanged(objectInfo_, isActivate_);
        }
    }

private:
    const GameObjectInfo& objectInfo_;
	bool isActivate_;
};

}

bool BuildingCompleteState::shouldChangeNextState(go::Entity* /*player*/) const
{
	return getOwner().getStaticObjectStatus().isMinHp();
}
	

void BuildingCompleteState::reInitialzie()
{
    initailizeTasks();
}


void BuildingCompleteState::changeNextState(go::Entity* /*player*/)
{
	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
	BuildingInfo& buildingInfo = getBuildingInfo();

	const datatable::BuildingFoundationInfo& fundationInfo = buildingTemplate.getBuildingFoundationInfo();
	buildingInfo.points_.maxHp_ = fundationInfo.foundationHp_;
	buildingInfo.points_.hp_ = fundationInfo.foundationHp_;
	buildingInfo.fundationExpireTime_ = 0;
	buildingInfo.startBuildTime_ = 0;
	if (fundationInfo.foundationLiveTime_ > 0) {
		buildingInfo.fundationExpireTime_ = getTime() + fundationInfo.foundationLiveTime_;
	}

	DominionType dominionType = buildingTemplate.getDominionType();
	if (dominionType != dtNone && buildingInfo.ownerInfo_.isValid()) {
		ZoneCommunityServerProxy& communityServerProxy =
			ZONE_SERVICE->getCommunityServerProxy();
		communityServerProxy.z2m_removeDominion(
			WORLD->getGlobalWorldMap().getMapCode(), buildingInfo.objectId_);
	}

	buildingInfo.ownerInfo_.reset();
	buildingInfo.state_ = bstWreckage;

    BaseItemInfos baseItems;
	Inventory* buildingInventory = getBuildingInventory();
	if (buildingInventory) {
		buildingInventory->fillHalfItem(baseItems);
        buildingInventory->removeAllItems();
	}
	Inventory* mercenaryInventory = getMercenaryInventory();
	if (mercenaryInventory) {
		mercenaryInventory->fillHalfItem(baseItems);
        mercenaryInventory->removeAllItems();
	}
    Inventory* productionInventory = getProductionInventory();
    if (productionInventory) {
        productionInventory->fillHalfItem(baseItems);
        productionInventory->removeAllItems();
    }
    Inventory* selectProductionInventory = getMercenaryInventory();
    if (selectProductionInventory) {
        selectProductionInventory->fillHalfItem(baseItems);
        selectProductionInventory->removeAllItems();
    }

	updateMoreInfo(false);

    Inventory* fundationInventory = getFundationInventory();
	if (fundationInventory) {
		for (const BaseItemInfo& baseItemInfo : baseItems) {
			fundationInventory->addItem(createAddItemInfoByBaseItemInfo(baseItemInfo));
		}
        BaseItemInfos workerItems = getWorkerItems(buildingTemplate.getMaterials());
        for (const BaseItemInfo& workerItem : workerItems) {
            fundationInventory->addItem(createAddItemInfoByBaseItemInfo(workerItem));
        }
	}

    getOwner().getController().queryBuildingStateCallback()->buildingToFoundationReverted();

    go::Summonable* summonable = getOwner().querySummonable();
    if (summonable != nullptr) {
        for (go::Summonable::EntitySet::value_type value : summonable->getSummons()) {
            const GameObjectInfo& info = (value);
            go::Knowable* knowable = getOwner().queryKnowable();
            if (knowable) {
                go::Entity* entity = knowable->getEntity(info);
                if (entity) {
                    ErrorCode errorCode = entity->despawn();
                    if (isFailed(errorCode)) {
                        assert(false);
                    }
                }
            }
        }
    }
}


void BuildingCompleteState::handleExpiredTasks()
{
	BuildingState::handleExpiredTasks();

	bool isPreActivate = getActivate();
	bool isActivate = handleTimeTasks();
	if (isPreActivate != isActivate && dominionType_ != dtNone) {
		const BuildingInfo& buildingInfo = getBuildingInfo();
		ZoneCommunityServerProxy& communityServerProxy =
			ZONE_SERVICE->getCommunityServerProxy();
		communityServerProxy.z2m_setActivateDominion(
			WORLD->getGlobalWorldMap().getMapCode(), buildingInfo.objectId_, isActivate);

		/// 브로드 케스팅
	}
    if (isPreActivate != isActivate) {
        updateMoreInfo(isActivate);
        
        auto event = std::make_shared<BuildingActivateEvent>(getOwner().getGameObjectInfo(), isActivate);
        Knowable* knowalbe = getOwner().queryKnowable();
        if (knowalbe) {
            knowalbe->broadcast(event);
        }
        
    }
}


ErrorCode BuildingCompleteState::startRecipeProduction(RecipeCode recipeCode, uint8_t count)
{
    const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
    BuildingSelectProductionInfos& selectProductionInfos = getBuildingInfo().selectProducionInfos_;
    if (buildingTemplate.getSelectProductionCount() <= getBuildingInfo().selectProducionInfos_.size()) {
        return ecAnchorOverSelectProductionType;
    }

    for (const BuildingSelectProductionInfo& selectProductionInfo : selectProductionInfos) {
        if (selectProductionInfo.recipeCode_ == recipeCode) {
            return ecAnchorAlreadySelectRecipeProduction;
        }
    }

    ErrorCode errorCode = checkSelectRecipeProduction(buildingTemplate.getSelectProducionCode(), recipeCode, count);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(recipeCode);
    if (! recipe) {
        return ecServerInternalError;
    }
    
    Inventory* mercenaryInventory = getMercenaryInventory();
    if (! mercenaryInventory) {
        return ecServerInternalError;
    }

    const BaseItemInfos& needItemInfos = recipe->getNeedItemInfos();
    
    BaseItemInfos needItems;
    for (uint8_t i = 0; i < count; ++i) {
        needItems.insert(needItems.end(), needItemInfos.begin(), needItemInfos.end());
    }

    if (! mercenaryInventory->checkHasItemsByBaseItemInfos(needItemInfos)) {
        return ecAnchorHasNotNeedComponentsSelectRecipe;
    }

    Inventory* selectProductionInventory = getSelectProductionInventory();
    if (! selectProductionInventory) {
        return ecServerInternalError;
    }

    BaseItemInfo completeItem(recipe->getCompleteItemCode(), 1);
    BaseItemInfos completeItems;
    for (uint8_t i = 0; i < count; ++i) {
        completeItems.push_back(completeItem);
    }

    GameTime completeMilSec = recipe->getCompleteMilSec();
    if (! selectProductionInventory->canAddItemByBaseItemInfos(completeItems)) {
        return ecAnchorNotEnoughInventorySelectRecipe;
    }
    
    mercenaryInventory->useItemsByBaseItemInfos(needItems);

    sec_t completeTime = getTime() + (completeMilSec / 1000);
    selectProductionInfos.push_back(BuildingSelectProductionInfo(recipeCode, count, completeTime));
    
    getOwner().getController().querySelectProductionCallback()->selectRecipeProductionAdded(recipeCode, count, completeTime);

    return ecOk;
}


void BuildingCompleteState::fillStateInfo(sec_t& expireIncibleCooltime,
	BuildingItemTaskInfos& consumeTaskInfos, BuildingItemTaskInfos& resouceTaskInfos,
     BuildingItemTaskInfos& selectProductionTaskInfos)
{
	expireIncibleCooltime = expireIncibleCooltime_;
	for (ConsumeItemInfo& consumeItemInfo : consumeItems_) {
		consumeTaskInfos.push_back(BuildingItemTaskInfo(consumeItemInfo.fuelItemInfo_.consumeItem_, consumeItemInfo.nextTime_));
	}

	for (BuildingProduceResourceInfo& resourceInfo : resources_) {
		resouceTaskInfos.push_back(BuildingItemTaskInfo(resourceInfo.info_.itemInfo_, resourceInfo.nextTime_));
	}

    BuildingSelectProductionInfos& selectProductionInfos = getBuildingInfo().selectProducionInfos_;
    for (BuildingSelectProductionInfo& item : selectProductionInfos) {
        selectProductionTaskInfos.push_back(BuildingItemTaskInfo(BaseItemInfo(item.recipeCode_, item.completeCount_), item.completeTime_));
    }
}


Inventory* BuildingCompleteState::getBuildingInventory()
{
	return getOwner().queryOutsideInventoryable()->getInventory(itBuilding);
}


Inventory* BuildingCompleteState::getMercenaryInventory()
{
	return getOwner().queryOutsideInventoryable()->getInventory(itMercenary);
}


Inventory* BuildingCompleteState::getProductionInventory()
{
    return getOwner().queryOutsideInventoryable()->getInventory(itBuildingProduction);
}


Inventory* BuildingCompleteState::getSelectProductionInventory()
{
    return getOwner().queryOutsideInventoryable()->getInventory(itBuildingSelectProduction);
}


void BuildingCompleteState::initailizeTasks()
{
	sec_t now = getTime();

	consumeItems_.clear();
	resources_.clear();
	autoSkillInfos_.clear();
    expireInvincibleTime_ = 0;
    expireIncibleCooltime_ = 0;
    dominionType_ = getBuildingTemplate().getDominionType();

	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
	
	for (const datatable::BuildingFuelItem& info : buildingTemplate.getFuelItems()) {
		consumeItems_.push_back(ConsumeItemInfo(info, now));
	}

	const datatable::ResourcesProductionTemplate* resourcesProductionTemplate = 
		RESOURCES_PRODUCTION_TABLE->getResourcesProductionTemplate(
		buildingTemplate.getProduceResourcesCode());
	if (resourcesProductionTemplate) {
		const datatable::ResourcesProductionInfos& resourcesProductionInfos = 
			resourcesProductionTemplate->getResourcesProductionInfos();
		for (const datatable::ResourcesProductionInfo& info : resourcesProductionInfos) {
			resources_.push_back(BuildingProduceResourceInfo(info, now + info.produceTime_));
		}
	}


	for (const datatable::AnchorSkillInfo& skillInfo : buildingTemplate.getSkillInfos()) {
		if (skillInfo.castType_ == datatable::ssctAutoActivate || skillInfo.castType_ == datatable::ssctAutoComplete) {
			autoSkillInfos_.push_back(AutoSkillInfo(skillInfo,
				now + skillInfo.autoCastTime_));
			getOwner().getSkillCastableAbility().learn(skillInfo.skillCode_);
		}
	} 
}


bool BuildingCompleteState::useInvincibleItem(sec_t& activateSec)
{
	if (getTime() < expireIncibleCooltime_) {
		return false;
	}

	Inventory* buildingInventory = getBuildingInventory();
	if (! buildingInventory) {
		return false;
	}

	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
	const datatable::InvincibleItemInfos& infos = buildingTemplate.getInvincibleItemInfos();

	if (infos.cashInvincibleSec_ > 0) {
		if (buildingInventory->checkHasItemsByBaseItemInfo(infos.cashInvincibleInfo_)) {
			buildingInventory->useItemsByBaseItemInfo(infos.cashInvincibleInfo_);
			activateSec = infos.cashInvincibleSec_;
			return true;
		}
	}
	if (infos.freeInvincibleSec_ > 0) {
		if (buildingInventory->checkHasItemsByBaseItemInfos(infos.freeInvincibleInfos_)) {
			buildingInventory->useItemsByBaseItemInfos(infos.freeInvincibleInfos_);
			activateSec = infos.freeInvincibleSec_;
			return true;
		}
	}

	return false;
}


void BuildingCompleteState::activateInvincible(sec_t activateSec)
{
	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
	sec_t now = getTime();
	expireInvincibleTime_ = now + activateSec;
	expireIncibleCooltime_ = now + buildingTemplate.getIncibleCooltimeSec();
	BuildingInfo& buildingInfo = getBuildingInfo();
	buildingInfo.isInvincible_ = true;
	updateMoreInfo(false);

    auto event = std::make_shared<BuildingInInvincibleEvent>(getOwner().getGameObjectInfo(), true);
    Knowable* knowalbe = getOwner().queryKnowable();
    if (knowalbe) {
        knowalbe->broadcast(event);
    }
}


void BuildingCompleteState::releaseInvincible()
{
	BuildingInfo& buildingInfo = getBuildingInfo();
	buildingInfo.isInvincible_ = false;
	updateMoreInfo(getActivate());
    auto event = std::make_shared<BuildingInInvincibleEvent>(getOwner().getGameObjectInfo(), false);
    Knowable* knowalbe = getOwner().queryKnowable();
    if (knowalbe) {
        knowalbe->broadcast(event);
    }
}


bool BuildingCompleteState::handleTimeTasks()
{
    sec_t now = getTime();	
	for (AutoSkillInfo& autoSkillInfo : autoSkillInfos_) {
		if (autoSkillInfo.nextTime_ < now && autoSkillInfo.skillInfo_.castType_ == datatable::ssctAutoComplete) {
			getOwner().querySkillCastable()->castTo(getOwner().getGameObjectInfo(), autoSkillInfo.skillInfo_.skillCode_);
			autoSkillInfo.nextTime_ = now + autoSkillInfo.skillInfo_.autoCastTime_;
		}
	}

    if (getBuildingInfo().isInvincible_ && expireInvincibleTime_ < getTime()) {
        releaseInvincible();
    }

	if (getOwner().isHalfBelowHp()) {
		BuildingInfo& buildingInfo = getBuildingInfo();
		if (! buildingInfo.isInvincible_) {
			if (expireIncibleCooltime_ < getTime()) {
				sec_t activateTime = 0;
				if (useInvincibleItem(activateTime)) {
					activateInvincible(activateTime);
				}
			}			
		}
		updateDelay();
		return false;
	}

	Inventory* buildingInventory = getBuildingInventory();
	if (! buildingInventory && ! consumeItems_.empty()) {
		return false;
	}


	for (const ConsumeItemInfo& consumeItemInfo : consumeItems_) {
		if (consumeItemInfo.nextTime_ <= now) {
			if (! buildingInventory->checkHasItemsByBaseItemInfo(consumeItemInfo.fuelItemInfo_.consumeItem_)) {
				updateDelay();
				return false;
			}
		}
	}

	for (ConsumeItemInfo& consumeItemInfo : consumeItems_) {
		if (consumeItemInfo.nextTime_ <= now) {
            buildingInventory->useItemsByBaseItemInfo(consumeItemInfo.fuelItemInfo_.consumeItem_);
			consumeItemInfo.nextTime_ = now + consumeItemInfo.fuelItemInfo_.supportConsumeTime_;
		}
	}

    Inventory* productionInventory = getProductionInventory();
	for (BuildingProduceResourceInfo& resourceInfo : resources_) {
        if (! productionInventory) {
            assert(productionInventory);
            continue;
        }
		if (resourceInfo.nextTime_ <= now) {
			productionInventory->addItem(createAddItemInfoByBaseItemInfo(resourceInfo.info_.itemInfo_));
			resourceInfo.nextTime_ = now + resourceInfo.info_.produceTime_;
		}
	}
    

	for (AutoSkillInfo& autoSkillInfo : autoSkillInfos_) {
		if (autoSkillInfo.nextTime_ <= now && autoSkillInfo.skillInfo_.castType_ == datatable::ssctAutoActivate) {
			getOwner().querySkillCastable()->castTo(getOwner().getGameObjectInfo(), autoSkillInfo.skillInfo_.skillCode_);
			autoSkillInfo.nextTime_ = now + autoSkillInfo.skillInfo_.autoCastTime_;
		}
	}

    BuildingSelectProductionInfos& selectProductionInfos = getBuildingInfo().selectProducionInfos_;
    BuildingSelectProductionInfos::iterator pos = selectProductionInfos.begin();
    while (pos != selectProductionInfos.end()) {
        BuildingSelectProductionInfo& selectProduction = (*pos);
        if (selectProduction.completeTime_ <= now ) {            
            if (selectProduction.completeCount_ > 0) {
                const datatable::RecipeTemplate* recipe = RECIPE_TABLE->getRecipe(selectProduction.recipeCode_);
                if (! recipe) {
                    assert(false);
                    continue;
                }
                Inventory* selectInventory = getSelectProductionInventory();
                if (! selectInventory) {
                    assert(false);
                    continue;
                }
                selectInventory->addItem(createAddItemInfoByItemCode(recipe->getCompleteItemCode(), 1));
                --selectProduction.completeCount_;
                if (selectProduction.completeCount_ == 0) {
                    getOwner().getController().querySelectProductionCallback()->selectRecipeProductionRemoved(selectProduction.recipeCode_);
                    pos = selectProductionInfos.erase(pos);
                    continue;
                }
                sec_t delaySec = recipe->getCompleteMilSec() / 1000;
                selectProduction.completeTime_ = now + delaySec;
                getOwner().getController().querySelectProductionCallback()->selectRecipeProductionUpdated(selectProduction.recipeCode_,
                    selectProduction.completeCount_, selectProduction.completeTime_);
            }
        }
        ++pos;
    }

	return true;
}

void BuildingCompleteState::updateDelay()
{
    for (ConsumeItemInfo& consumeItemInfo : consumeItems_) {
        ++consumeItemInfo.nextTime_;
    }

    for (BuildingProduceResourceInfo& resourceInfo : resources_) {
        ++resourceInfo.nextTime_;
    }
    BuildingSelectProductionInfos& selectProductionInfos = getBuildingInfo().selectProducionInfos_;
    for (BuildingSelectProductionInfo& item : selectProductionInfos) {
        ++item.completeTime_;
    }
}

}}} // namespace gideon { namespace zoneserver { namespace go {
