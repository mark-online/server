#include "ZoneServerPCH.h"
#include "BuildingFoundationState.h"
#include "../Building.h"
#include "../Entity.h"
#include "../ability/Guildable.h"
#include "../../item/Inventory.h"
#include "../../../service/guild/GuildService.h"
#include "../../../service/guild/Guild.h"
#include "../../../s2s/ZoneCommunityServerProxy.h"
#include "../../../world/WorldMap.h"
#include "../../../world/World.h"
#include "../../../ZoneService.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/BuildingStateCallback.h"

namespace gideon { namespace zoneserver { namespace go {


void BuildingFoundationState::reInitialzie()
{
    updateMoreInfo(false);
}


bool BuildingFoundationState::shouldChangeNextState(go::Entity* player) const
{
	if (! player) {
		return false;
	}

	const BuildingInfo& buidlingInfo = getBuildingInfo();

	HitPoint canBuildHp = toHitPoint(buidlingInfo.points_.maxHp_ / 2);
	if (buidlingInfo.points_.hp_ < canBuildHp) {
		return false;
	}

	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();

	const datatable::BaseAnchorInfo & baseAnchorInfo_ = buildingTemplate.getBaseAnchorInfo();
	if (isGuildOwnerType(baseAnchorInfo_.ownerType_)) {
		GuildId guildId = player->queryGuildable()->getGuildId();
		if (! isValidObjectId(guildId)) {
			return false;
		}
	}
	
	const Inventory* inventory = getFundationInventory();
	if (! inventory) {
		return false;
	}

	return inventory->checkHasItemsByBaseItemInfos(buildingTemplate.getMaterials());
}


void BuildingFoundationState::changeNextState(go::Entity* player)
{
	const datatable::BuildingTemplate& buildingTemplate = getBuildingTemplate();
	const datatable::BaseAnchorInfo & baseAnchorInfo = buildingTemplate.getBaseAnchorInfo();
	BuildingInfo& buildingInfo = getBuildingInfo();

	buildingInfo.ownerInfo_.ownerType_ = baseAnchorInfo.ownerType_;
	if (isGuildOwnerType(baseAnchorInfo.ownerType_)) {
		GuildId guildId = player->queryGuildable()->getGuildId();
		if (! isValidGuildId(guildId)) {
			return;
		}
		GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
		if (guild.get() != nullptr) {
			buildingInfo.ownerInfo_.guildInfo_ = guild->getBaseGuildInfo();
		}

	}
	else if (isPrivateOwnerType(baseAnchorInfo.ownerType_)) {
		buildingInfo.ownerInfo_.playerOwnerInfo_.objectId_ = player->getObjectId();
		buildingInfo.ownerInfo_.playerOwnerInfo_.nickname_ = player->getNickname();
	}

	Inventory* inventory = getFundationInventory();
	if (inventory) {
		inventory->removeAllItems();
	}
	
	DominionType dominionType = buildingTemplate.getDominionType();
	if (dominionType != dtNone && buildingInfo.ownerInfo_.isValid()) {
		MoreBuildingDominionInfo info(buildingInfo.objectId_, buildingInfo.position_,
			buildingInfo.buildingCode_, buildingInfo.ownerInfo_.guildInfo_, L"", 
			false, buildingTemplate.canTeleport(), dominionType);
		ZoneCommunityServerProxy& communityServerProxy =
			ZONE_SERVICE->getCommunityServerProxy();
		communityServerProxy.z2m_addDominion(WORLD->getGlobalWorldMap().getMapCode(), info);
	}

	buildingInfo.state_ = bstBuilding;
	buildingInfo.startBuildTime_ = getTime();
	buildingInfo.fundationExpireTime_ = 0;
    updateMoreInfo(false);


	getOwner().getController().queryBuildingStateCallback()->buildingStartBuilt(buildingInfo.startBuildTime_);
	getOwner().getController().queryBuildingStateCallback()->buildOwnerUpdated();
}


void BuildingFoundationState::handleExpiredTasks()
{
	BuildingState::handleExpiredTasks();
	BuildingInfo& buildingInfo = getBuildingInfo();
	if (buildingInfo.isValid() && buildingInfo.fundationExpireTime_ != 0 && buildingInfo.fundationExpireTime_ < getTime()) {
		getOwner().getController().queryBuildingStateCallback()->buildDestroyed();
		const ErrorCode ec = static_cast<Entity&>(getOwner()).despawn();
        if (isFailed(ec)) {
            assert(false);
        }
	}

    if (buildingInfo.isValid() && buildingInfo.isMinHp() && getBuildingTemplate().canDestory()) {
        getOwner().getController().queryBuildingStateCallback()->buildDestroyed();
        const ErrorCode ec = static_cast<Entity&>(getOwner()).despawn();
        if (isFailed(ec)) {
            assert(false);
        }
    }
}

}}} // namespace gideon { namespace zoneserver { namespace go {