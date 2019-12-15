#include "ZoneServerPCH.h"
#include "Arena.h"
#include "ArenaDualDeathMatchMode.h"
#include "ArenaTeamDeathMatchMode.h"
#include "../ArenaServiceCallback.h"
#include "../../../world/WorldMap.h"
#include "../../../world/region/MapRegion.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/ability/ArenaMatchable.h"
#include <gideon/serverbase/datatable/DataTableLoader.h>
#include <gideon/cs/datatable/ArenaTable.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver {


Arena::Arena(ObjectId arenaId, ArenaServiceCallback& serviceCallback,
    WorldMapHelper& worldMapHelper) :
    arenaId_(arenaId),
    serviceCallback_(serviceCallback),
    worldMapHelper_(worldMapHelper)
{
}


Arena::~Arena()
{
}


bool Arena::initialize(MapCode mapCode)
{
    arenaTempate_ = ARENA_TABLE->getArenaTemplate(mapCode);
    assert(arenaTempate_);

    MapInfo mapInfo;
    mapInfo.mapCode_ = mapCode;
    mapCode_ = mapCode;
    mapInfo.sectorLength_ = arenaTempate_->sector_length();
    mapInfo.xSectorCount_ = arenaTempate_->x_sector_count();
    mapInfo.ySectorCount_ = arenaTempate_->y_sector_count();
    mapInfo.zSectorCount_ = arenaTempate_->z_sector_count();

    worldMap_ = std::make_unique<WorldMap>(worldMapHelper_, arenaId_);
    if (! worldMap_->initialize(mapInfo)) {
        return false;
    }

    const sec_t playingTime = arenaTempate_->match_time() * 60;
    const ArenaModeType modeType = toArenaModeType(arenaTempate_->arena_mode_type());
    if (modeType == amtDualMatch) {
        arenaMode_ = std::make_unique<ArenaDualDeathMatchMode>(modeType,
            arenaTempate_->min_player_count(), arenaTempate_->max_player_count(),
            playingTime, *worldMap_);
    }
    else if (modeType == amtTeamDeathMatch) {
        arenaMode_ = std::make_unique<ArenaTeamDeathMatchMode>(modeType,
            arenaTempate_->min_player_count(), arenaTempate_->max_player_count(),
            playingTime, *worldMap_);
    }
    else {
        assert(false);
    }

    return arenaMode_->initialize();
}


void Arena::finalize()
{
    if (arenaMode_.get() != nullptr) {
        arenaMode_->finalize();
    }

    if (worldMap_.get() != nullptr) {
		worldMap_->finalizeForSpawnRegionTickable();
		worldMap_->finalize();
    }
}


bool Arena::addWaitMatcher(ObjectId challengerId)
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->addWaitMatcher(challengerId);
}


void Arena::removeWaitMatcher(ObjectId challengerId)
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->removeWaitMatcher(challengerId);
}


void Arena::despawn(go::Entity& player)
{
    worldMap_->despawn(player);
}


void Arena::leave(AccountId accountId, ObjectId playerId)
{
    if (isPlaying()) {
        if (isEnoughValidPlayTime()) {
            const ArenaModeType arenaModeType = toArenaModeType(arenaTempate_->arena_mode_type());
            serviceCallback_.registDeserter(playerId);
            DatabaseGuard db(SNE_DATABASE_MANAGER);
            db->asyncUpdateArenaRecord(accountId, playerId, arenaModeType, 0, artLose);
        }

    }

    {
        std::lock_guard<LockType> lock(lock_);

        return arenaMode_->leave(playerId);
    }
}


MapCode Arena::getPreGlobalMapCode(ObjectId playerId) const
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->getPreGlobalMapCode(playerId);
}


void Arena::killed(go::Entity& killer, go::Entity& victim)
{
    std::lock_guard<LockType> lock(lock_);

    arenaMode_->killed(killer, victim);
}


void Arena::stop()
{
    std::lock_guard<LockType> lock(lock_);

    if (arenaMode_.get() != nullptr) {
        arenaMode_->stop();
    }

    if (worldMap_.get() != nullptr) {
        worldMap_->finalizeForSpawnRegionTickable();
        worldMap_->finalize();

    }
}


void Arena::say(ObjectId playerId, const ChatMessage& message)
{
    std::lock_guard<LockType> lock(lock_);

    arenaMode_->say(playerId, message);
}


bool Arena::shouldDestroy() const
{
    std::lock_guard<LockType> lock(lock_);

    size_t count = worldMap_->getEntityCount(otPc);
    if (count > 0) {
        return false;
    }
    return arenaMode_->shouldDestroy();
}


bool Arena::canJoin() const
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->canJoin();
}


bool Arena::shouldStop() const
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->shouldStop();
}


bool Arena::isPlaying() const
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->isPlaying();
}

bool Arena::canMountVehicle() const
{
    return arenaTempate_->can_mount_vehicle() != 0;
}


bool Arena::canMonutGlider() const
{
    return arenaTempate_->can_glider() != 0;
}


bool Arena::isEnoughValidPlayTime() const
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->isEnoughValidPlayTime();
}


bool Arena::isSameTeam(ObjectId sourceId, ObjectId targetId) const
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->isSameTeam(sourceId, targetId);
}


bool Arena::canMoving(ObjectId playerId, const Position& movePosition) const
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->canMoving(playerId, movePosition);
}


void Arena::setWaypoint(ObjectId playerId, const Waypoint& waypoint)
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->setWaypoint(playerId, waypoint);
}


void Arena::notifyMoved(ObjectId playerId, float32_t x, float32_t y)
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->notifyMoved(playerId, x, y);
}


void Arena::revivePlayers()
{
    std::lock_guard<LockType> lock(lock_);

    return arenaMode_->revivePlayers();
}


ObjectPosition Arena::arrangePosition(go::Entity& player, MapCode preGlobalMapCode)
{
    std::lock_guard<LockType> lock(lock_);

    ObjectPosition position;
    if (! arenaMode_->arrangePosition(position, player, preGlobalMapCode)) {
        assert(false);
        return position;
    }
	player.queryArenaMatchable()->setArena(this);
    return position;
}


void Arena::readyToPlay(go::Entity& player)
{
    std::lock_guard<LockType> lock(lock_);

    arenaMode_->readyToPlay(player);
}


WorldMap& Arena::getWorldMap()
{
	return *worldMap_;
}

}} // namespace gideon { namespace zoneserver {
