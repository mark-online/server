#include "ZoneServerPCH.h"
#include "BuildingBuildState.h"
#include "../Building.h"
#include "../../item/Inventory.h"
#include "../../../helper/InventoryHelper.h"
#include "../../../s2s/ZoneCommunityServerProxy.h"
#include "../../../ZoneService.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/BuildingStateCallback.h"
#include "../../../world/WorldMap.h"
#include "../../../world/World.h"

namespace gideon { namespace zoneserver { namespace go {


bool BuildingBuildState::shouldChangeNextState(go::Entity* /*player*/) const
{
	if (getOwner().isHalfBelowHp()) {
		return false;
	}
	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
	const datatable::BuildingFoundationInfo& fundationInfo = buildingTemplate.getBuildingFoundationInfo();
	sec_t now = getTime();
	const BuildingInfo& buildingInfo = getBuildingInfo();
	return  buildingInfo.startBuildTime_ + fundationInfo.buildTime_ < now;
}


bool BuildingBuildState::shouldChangePreState() const
{
	return getOwner().isHalfBelowHp();
}


void BuildingBuildState::changeNextState(go::Entity* /*player*/)
{
	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
	const datatable::BaseAnchorInfo & baseAnchorInfo_ = buildingTemplate.getBaseAnchorInfo();
	BuildingInfo& buildingInfo = getBuildingInfo();

	buildingInfo.points_.maxHp_ = baseAnchorInfo_.completeHp_;
	buildingInfo.points_.hp_ = baseAnchorInfo_.completeHp_;
	buildingInfo.startBuildTime_ = 0;
	buildingInfo.state_ = bstComplete;
	updateMoreInfo(true);

	buildingInfo.fundationExpireTime_ = 0;
	//cancelFoundationLiveTask();    

	getOwner().getController().queryBuildingStateCallback()->buildCompleted();
}


void BuildingBuildState::changePreState()
{
	BuildingInfo& buildingInfo = getBuildingInfo();
	buildingInfo.state_ = bstWreckage;
	buildingInfo.ownerInfo_.reset();
	updateMoreInfo(false);

    const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
    DominionType dominionType = buildingTemplate.getDominionType();
    if (dominionType != dtNone) {
        ZoneCommunityServerProxy& communityServerProxy =
            ZONE_SERVICE->getCommunityServerProxy();
        communityServerProxy.z2m_removeDominion(
            WORLD->getGlobalWorldMap().getMapCode(), buildingInfo.objectId_);
    
    }

	getOwner().getController().queryBuildingStateCallback()->buildingToFoundationReverted();
    Inventory* fundationInventory = getFundationInventory();
    if (fundationInventory) {        
        BaseItemInfos workerItems = getWorkerItems(buildingTemplate.getMaterials());
        for (const BaseItemInfo& workerItem : workerItems) {
            fundationInventory->addItem(createAddItemInfoByBaseItemInfo(workerItem));
        }
    }
}

void BuildingBuildState::handleExpiredTasks()
{
	BuildingState::handleExpiredTasks();
}

}}} // namespace gideon { namespace zoneserver { namespace go {