#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/RegionInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <sne/core/utility/Singleton.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

class WorldMap;


/**
 * @class TeleportService
 *
 */
class TeleportService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(TeleportService);

public:
    TeleportService() {
        TeleportService::instance(this);
    }

    /// @param shouldForce 강제 이동?
    ErrorCode teleportTo(MigrationTicket& migrationTicket, ObjectPosition& spawnPosition,
        go::Entity& entity, MapCode toWorldMapCode, RegionCode toRegionCode,
        const Position& toPosition);

	ErrorCode teleportZoneToArena(MigrationTicket& migrationTicket,
		go::Entity& entity, MapCode arenaMapCode, ObjectId arenaId);

	ErrorCode teleportArenaToZone(MigrationTicket& migrationTicket, go::Entity& entity,
		MapCode zoneGlobalWorldMapCode);

    ErrorCode teleportRecall(MigrationTicket& migrationTicket, go::Entity& entity,
        const WorldPosition& worldPosition);

    /// 같은 월드맵 내에서의 이동
    ErrorCode teleportPosition(const WorldPosition& toPosition, go::Entity& entity);

private:
    ErrorCode teleportInCurrentZone(ObjectPosition& spawnPosition,
        WorldMap& worldMap, go::Entity& entity, RegionCode toRegionCode,
        const Position& toPosition);
    ErrorCode teleportToOtherZone(MigrationTicket& migrationTicket,
        go::Entity& entity, MapCode targetMapCode, RegionCode targetRegionCode,
        ObjectId arenaId, const Position& spawnPosition = Position());
};

}} // namespace gideon { namespace zoneserver {

#define TELEPORT_SERVICE gideon::zoneserver::TeleportService::instance()
