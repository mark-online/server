#pragma once

#include "BuildingState.h"
#include <gideon/cs/datatable/ResourcesProductionTable.h>
#include <sne/core/container/Containers.h>

namespace gideon { namespace zoneserver { namespace go {


class BuildingCompleteState : public BuildingState
{
	struct BuildingProduceResourceInfo
	{
		datatable::ResourcesProductionInfo info_;
		sec_t nextTime_;

		BuildingProduceResourceInfo(const datatable::ResourcesProductionInfo& info,
			sec_t nextTime) :
			info_(info),
			nextTime_(nextTime) {}
	};

	typedef sne::core::Vector<BuildingProduceResourceInfo> BuildingProduceResourceInfos;

	struct ConsumeItemInfo
	{
		datatable::BuildingFuelItem fuelItemInfo_;
		sec_t nextTime_;

		ConsumeItemInfo(const datatable::BuildingFuelItem& fuelItemInfo,
			sec_t nextTime) :
			fuelItemInfo_(fuelItemInfo),
			nextTime_(nextTime)
		{
		}       
	};

	typedef sne::core::Vector<ConsumeItemInfo> ConsumeItemInfos;


	struct AutoSkillInfo
	{
		datatable::AnchorSkillInfo skillInfo_;
		sec_t nextTime_;

		AutoSkillInfo(const datatable::AnchorSkillInfo& skillInfo,
			sec_t nextTime) :
			skillInfo_(skillInfo),
			nextTime_(nextTime)
		{
		}   
	};

	typedef sne::core::Vector<AutoSkillInfo> AutoSkillInfos;

public:

	BuildingCompleteState(Building& owner, const datatable::BuildingTemplate& buildingTemlate,
		BuildingInfo& buildingInfo, MoreBuildingInfo& moreBuildingInfo, Inventory* fundationInventory) :
		BuildingState(owner, buildingTemlate, buildingInfo, moreBuildingInfo, fundationInventory),
		expireInvincibleTime_(0),
		expireIncibleCooltime_(0) {}

	virtual bool shouldChangeNextState(go::Entity* player) const;
	virtual bool isActiveAbillity() const {
		return getActivate();
	}
	virtual void reInitialzie();
	virtual void changeNextState(go::Entity* player);
	virtual void handleExpiredTasks();
	virtual void fillStateInfo(sec_t& expireIncibleCooltime,
		BuildingItemTaskInfos& consumeTaskInfos, BuildingItemTaskInfos& resouceTaskInfos,
        BuildingItemTaskInfos& selectProductionTaskInfos);


	virtual BuildingStateType getNextState() const {
		return bstWreckage;
	}
	virtual BuildingStateType getPreState() const {
		return bstUnknown;
	}
	virtual BuildingStateType getCurrentState() const {
		return bstComplete;
	}
	virtual sec_t getInvincibleExpireTime() const {
		return expireInvincibleTime_;
	}

    virtual ErrorCode startRecipeProduction(RecipeCode recipeCode, uint8_t count);

private:
    Inventory* getBuildingInventory();
	Inventory* getMercenaryInventory();
    Inventory* getProductionInventory();    
    Inventory* getSelectProductionInventory();

	void initailizeTasks();
	bool useInvincibleItem(sec_t& activateSec);
	void activateInvincible(sec_t activateSec);
	void releaseInvincible();

	bool handleTimeTasks();

    void updateDelay();


private:
	BuildingProduceResourceInfos resources_;
	ConsumeItemInfos consumeItems_;
	AutoSkillInfos autoSkillInfos_;
	sec_t expireInvincibleTime_;
	sec_t expireIncibleCooltime_;
	DominionType dominionType_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
