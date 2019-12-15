#pragma once

#include <gideon/cs/shared/data/DominionInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace communityserver {


/**
 * @class CommunityDominionManager
 * 세력도 관리자
 */
class CommunityDominionManager : public boost::noncopyable
{
    typedef std::mutex LockType;
	typedef sne::core::HashMap<MapCode, BuildingDominionInfos> BuildingDominionInfosMap;

public:
	SNE_DECLARE_SINGLETON(CommunityDominionManager);

public:
	CommunityDominionManager() {}

	void addDominion(MapCode worldMapCode, const MoreBuildingDominionInfo& info);
	void removeDominion(MapCode worldMapCode, ObjectId buildingId);
	void setActivate(MapCode worldMapCode, ObjectId buildingId, bool isActivate);

	ErrorCode checkTeleport(BuildingCode& buildingCode, WorldPosition& buildingPosition,
		MapCode worldMapCode, GuildId guildId, ObjectId buildingId) const;

	const DominionWorldInfoMap& getDominionWorldInfoMap() const {
		return worldIDominionnfos_;
	}

	const BuildingDominionInfos* getDominionZoneInfos(MapCode worldMapCode) const;

private:
    mutable LockType lock_;
	DominionWorldInfoMap worldIDominionnfos_;
	BuildingDominionInfosMap buildingDominionInfos_;

};

}} // namespace gideon { namespace communityserver {


#define COMMUNITYDOMINON_MANAGER gideon::communityserver::CommunityDominionManager::instance()