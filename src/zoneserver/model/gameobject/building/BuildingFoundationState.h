#pragma once

#include "BuildingState.h"

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class BuildingFoundationState
 */
class BuildingFoundationState : public BuildingState
{
public:
	BuildingFoundationState(Building& owner, const datatable::BuildingTemplate& buildingTemlate,
		BuildingInfo& buildingInfo, MoreBuildingInfo& moreBuildingInfo, Inventory* fundationInventory) :
		BuildingState(owner, buildingTemlate, buildingInfo, moreBuildingInfo, fundationInventory) {}

private:
    virtual void reInitialzie();
	virtual bool shouldChangeNextState(go::Entity* player) const;
	virtual void changeNextState(go::Entity* player);
	virtual void handleExpiredTasks();

	virtual BuildingStateType getNextState() const {
		return bstBuilding;
	}
	virtual BuildingStateType getPreState() const {
		return bstUnknown;
	}
	virtual BuildingStateType getCurrentState() const {
		return bstFoundation;
	}
};


/**
 * @class BuildingWreckageState
 */
class BuildingWreckageState : public BuildingFoundationState
{
public:
	BuildingWreckageState(Building& owner, const datatable::BuildingTemplate& buildingTemlate,
		BuildingInfo& buildingInfo, MoreBuildingInfo& moreBuildingInfo, Inventory* fundationInventory) :
		BuildingFoundationState(owner, buildingTemlate, buildingInfo, moreBuildingInfo, fundationInventory) {}

private:
	virtual BuildingStateType getCurrentState() const {
		return bstWreckage;
	}
};


}}} // namespace gideon { namespace zoneserver { namespace go {
