#include "CommunityServerPCH.h"
#include "CommunityDominionManager.h"
#include "../guild/CommunityGuildManager.h"

namespace gideon { namespace communityserver {

SNE_DEFINE_SINGLETON(CommunityDominionManager);


void CommunityDominionManager::addDominion(MapCode worldMapCode, const MoreBuildingDominionInfo& info)
{
	Nickname guildMasterName;
	const CommunityGuildPtr guildPtr = COMMUNITYGUILD_MANAGER->getGuild(info.guildInfo_.guildId_);
	if (guildPtr.get() != nullptr) {
		guildMasterName = guildPtr->getGuildMasterName();
	}

    std::lock_guard<LockType> lock(lock_);

	if (info.dominionType_ == dtWorld) {
		worldIDominionnfos_.insert(DominionWorldInfoMap::value_type(info.buildingId_,
			DominionWorldInfo(worldMapCode, info.guildInfo_, guildMasterName)));
	}

	BuildingDominionInfo newInfo = info;
	newInfo.masterName_ = guildMasterName;
	BuildingDominionInfosMap::iterator pos = buildingDominionInfos_.find(worldMapCode);
	if (pos != buildingDominionInfos_.end()) {
		BuildingDominionInfos& infos = (*pos).second;
		infos.push_back(newInfo);
	}
	else {
		BuildingDominionInfos infos;
		infos.push_back(newInfo);
		buildingDominionInfos_.emplace(worldMapCode, infos);
	}
}


void CommunityDominionManager::setActivate(MapCode worldMapCode, ObjectId buildingId, bool isActivate)
{
    std::lock_guard<LockType> lock(lock_);

    BuildingDominionInfosMap::iterator pos = buildingDominionInfos_.find(worldMapCode);
	if (pos != buildingDominionInfos_.end()) {
		BuildingDominionInfos& infos = (*pos).second;
		for (BuildingDominionInfo& info : infos) {
			if (info.buildingId_ == buildingId) {
				info.isActivate_ = isActivate;
			}
		}
	}
}


void CommunityDominionManager::removeDominion(MapCode worldMapCode, ObjectId buildingId)
{
    std::lock_guard<LockType> lock(lock_);

    worldIDominionnfos_.erase(buildingId);

	BuildingDominionInfosMap::iterator posMap = buildingDominionInfos_.find(worldMapCode);
	if (posMap != buildingDominionInfos_.end()) {
		BuildingDominionInfos& infos = (*posMap).second;

		BuildingDominionInfos::iterator pos = infos.begin();
		BuildingDominionInfos::iterator end = infos.end();
		for (pos ; pos != end; ++pos) {
			BuildingDominionInfo& info = (*pos);
			if (info.buildingId_ == buildingId) {
				infos.erase(pos);
				return;
			}
		}
	}
}


ErrorCode CommunityDominionManager::checkTeleport(BuildingCode& buildingCode,
	WorldPosition& buildingPosition, MapCode worldMapCode, 
	GuildId guildId, ObjectId buildingId) const
{
    std::lock_guard<LockType> lock(lock_);

	BuildingDominionInfosMap::const_iterator pos = buildingDominionInfos_.find(worldMapCode);
	if (pos == buildingDominionInfos_.end()) {
		return ecAnchorNotFind;
	}

	const BuildingDominionInfos& infos = (*pos).second;
	for (const BuildingDominionInfo& info : infos) {
		if (info.buildingId_ == buildingId) {
			if (info.buildingId_ != guildId) {
				return ecAnchorTeleportNotEnoughGrade;
			}
			if (! info.canTeleport_) {
				return ecAnchorNotTeleportBuilding;
			}
			if (! info.isActivate_) {
				return ecAnchorTeleportNotActivate;
			}
			static_cast<ObjectPosition&>(buildingPosition) = info.position_;
			buildingPosition.mapCode_ = worldMapCode;
			buildingCode = info.buildingCode_;
			return ecOk;
		}		
	}
	return ecAnchorNotFind;
}


const BuildingDominionInfos* CommunityDominionManager::getDominionZoneInfos(MapCode worldMapCode) const
{
    std::lock_guard<LockType> lock(lock_);

    const BuildingDominionInfosMap::const_iterator pos = buildingDominionInfos_.find(worldMapCode);
	if (pos != buildingDominionInfos_.end()) {
		const BuildingDominionInfos& infos = (*pos).second;
		return &infos;
	}
	return nullptr;
}


}} // namespace gideon { namespace communityserver {