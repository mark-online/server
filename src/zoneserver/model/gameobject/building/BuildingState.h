#pragma once

#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/RecipeInfo.h>
#include <gideon/cs/shared/data/UnionEntityInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/datatable/BuildingTable.h>


namespace gideon { namespace zoneserver {
class Inventory;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

class Building;
class Entity;

/**
 * @class BuildingState
 */
class BuildingState
{
public:
	BuildingState(Building& owner, const datatable::BuildingTemplate& buildingTemplate,
		BuildingInfo& buildingInfo, MoreBuildingInfo& moreBuildingInfo, Inventory* fundationInventory);
	virtual ~BuildingState() {}

	virtual bool shouldChangeNextState(go::Entity* player) const = 0;
	virtual bool isActiveAbillity() const { return false; }
	virtual bool shouldChangePreState() const { return false; }

	virtual void reInitialzie() {}
	virtual void changeNextState(go::Entity* player) = 0;
	virtual void changePreState() {}
	virtual void handleExpiredTasks();

	virtual void fillStateInfo(sec_t& /*expireIncibleCooltime*/,
		BuildingItemTaskInfos& /*consumeTaskInfos*/,
		BuildingItemTaskInfos& /*resouceTaskInfos*/,
         BuildingItemTaskInfos& /*selectProductionTaskInfos*/) {}

	virtual BuildingStateType getNextState() const = 0;
	virtual BuildingStateType getPreState() const = 0;
	virtual BuildingStateType getCurrentState() const =0;
	virtual sec_t getInvincibleExpireTime() const {
		return 0;
	}

    virtual ErrorCode startRecipeProduction(RecipeCode /*recipeCode*/, uint8_t /*count*/) {
        return ecAnchorNotSelectProduction;
    }

protected:
	const datatable::BuildingTemplate& getBuildingTemplate() const {
		return buildingTemplate_;
	}
	const BuildingInfo& getBuildingInfo() const {
		return buildingInfo_;
	}

	BuildingInfo& getBuildingInfo() {
		return buildingInfo_;
	}

	const Inventory* getFundationInventory() const {
		return fundationInventory_;
	}

	Inventory* getFundationInventory() {
		return fundationInventory_;
	}

	Building& getOwner() {
		return owner_;
	}
	const Building& getOwner() const {
		return owner_;
	}

protected:
	void updateMoreInfo(bool isActivate);

    bool getActivate() const {
        return moreBuildingInfo_.isActivate_;
    }
	
private:
	const datatable::BuildingTemplate& buildingTemplate_;
	BuildingInfo& buildingInfo_;
	MoreBuildingInfo& moreBuildingInfo_;
	Building& owner_;
	Inventory* fundationInventory_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
