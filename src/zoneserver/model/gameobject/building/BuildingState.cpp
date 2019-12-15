#include "ZoneServerPCH.h"
#include "BuildingState.h"
#include "../Building.h"
#include "../ability/OutsideInventoryable.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/BuildingStateCallback.h"

namespace gideon { namespace zoneserver { namespace go {

BuildingState::BuildingState(Building& owner, const datatable::BuildingTemplate& buildingTemplate,
	BuildingInfo& buildingInfo, MoreBuildingInfo& moreBuildingInfo,
    Inventory* fundationInventory) :
	owner_(owner),
	buildingTemplate_(buildingTemplate),
	buildingInfo_(buildingInfo),
	moreBuildingInfo_(moreBuildingInfo),
	fundationInventory_(fundationInventory)
{
}


void BuildingState::updateMoreInfo(bool isActivate)
{
	static_cast<BaseBuildingInfo&>(moreBuildingInfo_) = buildingInfo_;
	moreBuildingInfo_.isActivate_ = isActivate;
}


void BuildingState::handleExpiredTasks()
{
	sec_t now = getTime();
	sec_t liveTime = buildingTemplate_.getBaseAnchorInfo().liveTime_;
	if (liveTime > 0) {
		sec_t expireTime = buildingInfo_.createTime_ + liveTime;
		if (now > expireTime) {
			getOwner().getController().queryBuildingStateCallback()->buildDestroyed();
			const ErrorCode ec = static_cast<Entity&>(getOwner()).despawn();       
            if (isFailed(ec)) {
                assert(false);
            }
        }
	}
}

}}} // namespace gideon { namespace zoneserver { namespace go {