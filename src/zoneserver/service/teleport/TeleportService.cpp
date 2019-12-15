#include "ZoneServerPCH.h"
#include "TeleportService.h"
#include "../../ZoneService.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../world/region/MapRegion.h"
#include "../../model/gameobject/ability/Moveable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../controller/EntityController.h"
#include "../../controller/MoveController.h"
#include <gideon/3d/3d.h>
#include <esut/Random.h>

namespace gideon { namespace zoneserver {

// = TeleportService

SNE_DEFINE_SINGLETON(TeleportService);

ErrorCode TeleportService::teleportTo(
    MigrationTicket& migrationTicket, ObjectPosition& spawnPosition,
    go::Entity& entity, MapCode toWorldMapCode, RegionCode toRegionCode,
    const Position& toPosition)
{
    // TODO: 텔레포트 가능 상태 검사 & 다른 행동(스킬 시전 등) 취소

    if (! isValidMapCode(toWorldMapCode)) {
        return ecTeleportNotAvailable;
    }

    // FYI: 현재는 글로벌 월드맵 간 이동만 지원
    assert(getMapType(toWorldMapCode) == mtGlobalWorld);
    if (getMapType(toWorldMapCode) != mtGlobalWorld) {
        return ecTeleportNotAvailable;
    }

    // TODO: 포탈인 경우 포탈 영역 내에 있는지 검사해야 한다
    // TODO: obelisk인 경우 일정 반경 내에 있는지 검사해야 한다

    WorldMap* currentWorldMap = entity.getCurrentWorldMap();
    if (! currentWorldMap) {
        return ecTeleportNotAvailable;
    }

    if (toWorldMapCode == currentWorldMap->getMapCode()) {
        return teleportInCurrentZone(spawnPosition, *currentWorldMap, entity, toRegionCode,
            toPosition);
    }

    return teleportToOtherZone(migrationTicket, entity, toWorldMapCode, toRegionCode,
        invalidObjectId, spawnPosition);
}


ErrorCode TeleportService::teleportZoneToArena(MigrationTicket& migrationTicket,
	go::Entity& entity, MapCode arenaMapCode, ObjectId arenaId)
{
	// TODO: 텔레포트 가능 상태 검사 & 다른 행동(스킬 시전 등) 취소

	if (! isValidMapCode(arenaMapCode)) {
		return ecTeleportNotAvailable;
	}

	return teleportToOtherZone(migrationTicket, entity, arenaMapCode, invalidRegionCode,
        arenaId);
}


ErrorCode TeleportService::teleportArenaToZone(MigrationTicket& migrationTicket, go::Entity& entity,
    MapCode zoneGlobalWorldMapCode)
{
    // TODO: 텔레포트 가능 상태 검사 & 다른 행동(스킬 시전 등) 취소

    if (! isValidMapCode(zoneGlobalWorldMapCode)) {
        return ecTeleportNotAvailable;
    }

    return teleportToOtherZone(migrationTicket, entity, zoneGlobalWorldMapCode,
        invalidRegionCode, invalidObjectId);
}


ErrorCode TeleportService::teleportRecall(MigrationTicket& migrationTicket, go::Entity& entity,
    const WorldPosition& worldPosition)
{
    // TODO: 텔레포트 가능 상태 검사 & 다른 행동(스킬 시전 등) 취소

    if (getMapType(worldPosition.mapCode_) != mtGlobalWorld) {
        return ecTeleportNotAvailable;
    }

    WorldMap* currentWorldMap = entity.getCurrentWorldMap();
    if (! currentWorldMap) {
        return ecTeleportNotAvailable;
    }

    if (worldPosition.mapCode_ == currentWorldMap->getMapCode()) {
        return teleportPosition(worldPosition, entity);
    }

    return teleportToOtherZone(migrationTicket, entity, worldPosition.mapCode_, invalidRegionCode,
        invalidObjectId, worldPosition);
}


ErrorCode TeleportService::teleportPosition(const WorldPosition& toPosition, go::Entity& entity)
{
    // TODO: 텔레포트 가능 상태 검사 & 다른 행동(스킬 시전 등) 취소
    // TODO: 포탈인 경우 포탈 영역 내에 있는지 검사해야 한다

    if (! isValidMapCode(toPosition.mapCode_)) {
        return ecTeleportNotAvailable;
    }

    WorldMap* currentWorldMap = entity.getCurrentWorldMap();
    if (! currentWorldMap) {
        return ecTeleportNotAvailable;
    }

    if (toPosition.mapCode_ != currentWorldMap->getMapCode()) {
        return ecTeleportNotAvailable;
    }

    go::Moveable* moveable = entity.queryMoveable();
    if (! moveable) {
        return ecTeleportNotAvailable;
    }

    moveable->getMoveController().stop(&toPosition, true);

    return ecOk;
}


ErrorCode TeleportService::teleportInCurrentZone(ObjectPosition& spawnPosition,
    WorldMap& worldMap, go::Entity& entity, RegionCode toRegionCode,
    const Position& toPosition)
{
    // FYI: 현재는 같은 월드맵 내에서만 텔레포트할 수 있다

    if (isValidRegionCode(toRegionCode)) {
        if (isTeleportable(getRegionType(toRegionCode))) {
            return ecTeleportNotAvailable;
        }

        const MapRegion* toMapRegion = worldMap.getMapRegion(toRegionCode);
        if (! toMapRegion) {
            return ecTeleportNotAvailable;
        }

        spawnPosition = toMapRegion->getRandomPosition();
    }
    else {
        if (isZero(asVector3(toPosition))) {
            return ecTeleportNotAvailable;
        }

        static_cast<Position&>(spawnPosition) = toPosition;
        spawnPosition.heading_ = esut::random() % maxHeading;
        keepAway(spawnPosition, esut::random(1.0f, 2.0f));
    }

    if (entity.isPlayer()) {
        if (entity.queryMoneyable()->getGameMoney() < teleportFee) {
            return ecTeleportNotEnoughGameMoney;
        }
    }

    go::Moveable* moveable = entity.queryMoveable();
    if (! moveable) {
        return ecTeleportNotAvailable;
    }
    moveable->getMoveController().stop(&spawnPosition, true);

    if (entity.isPlayer()) {
        entity.queryMoneyable()->downGameMoney(teleportFee);
    }

    return ecOk;
}


ErrorCode TeleportService::teleportToOtherZone(MigrationTicket& migrationTicket,
    go::Entity& entity, MapCode targetMapCode, RegionCode targetRegionCode,
    ObjectId arenaId, const Position& toPosition)
{
    const float32_t radiusDelta = 5.0f;

    if (! entity.isPlayer()) {
        return ecTeleportNotAvailable;
    }

    Position spawnPosition = toPosition;

	if (isValidObjectId(arenaId)) {
        migrationTicket.globalMapCode_ = entity.getWorldPosition().mapCode_;
    }
    else {
        if (isValidRegionCode(targetRegionCode)) {
		    WorldMap& globalWorldMap = WORLD->getGlobalWorldMap();

		    const MapRegion* teleportMapRegion =
			    globalWorldMap.getTeleportMapRegion(targetMapCode, targetRegionCode);
		    if (! teleportMapRegion) {
			    return ecTeleportNotAvailable;
		    }

		    if (! teleportMapRegion->isContained(entity.getPosition(), radiusDelta)) {
			    return ecTeleportNotAvailable;
		    }
        }
        else {
            if (isZero(asVector3(toPosition))) {
                SNE_LOG_WARNING("TeleportService::teleportToOtherZone(%u,%u) - regionCode is not valid.",
                    targetMapCode, targetRegionCode);
                // TODO: 차후에 해킹을 통해 텔레포트할 수 있으므로, 아래 주석을 풀어야 한다
                //return ecTeleportNotAvailable;
            }
            else {
                keepAway(spawnPosition, esut::random(1.0f, 2.0f));
            }
        }
	}

    const AccountId accountId = entity.getAccountId();

    const ErrorCode errorCode = ZONE_SERVICE->reserveMigration(migrationTicket,
        accountId, targetMapCode, targetRegionCode, arenaId, spawnPosition);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    return ecOk;
}

}} // namespace gideon { namespace zoneserver {
