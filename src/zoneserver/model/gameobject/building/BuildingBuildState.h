#pragma once

#include "BuildingState.h"

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class BuildingBuildState
 */
class BuildingBuildState : public BuildingState
{
public:
	BuildingBuildState(Building& owner, const datatable::BuildingTemplate& buildingTemlate,
		BuildingInfo& buildingInfo, MoreBuildingInfo& moreBuildingInfo, Inventory* fundationInventory) :
		BuildingState(owner, buildingTemlate, buildingInfo, moreBuildingInfo, fundationInventory) {}

	virtual bool shouldChangeNextState(go::Entity* player) const;
	virtual bool shouldChangePreState() const;
	virtual void changeNextState(go::Entity* player);
	virtual void changePreState();
	virtual void handleExpiredTasks();

	virtual BuildingStateType getNextState() const {
		return bstComplete;
	}
	virtual BuildingStateType getPreState() const {
		return bstWreckage;
	}
	virtual BuildingStateType getCurrentState() const {
		return bstBuilding;
	}
};

}}} // namespace gideon { namespace zoneserver { namespace go {
