#include "ZoneServerPCH.h"
#include "PlayerArenaController.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneArenaServerProxy.h"
#include "../../model/gameobject/Creature.h"
#include "../../model/gameobject/ability/Networkable.h"
#include "../../model/gameobject/ability/ArenaMatchable.h"
#include "../../model/gameobject/ability/Achievementable.h"
#include "../../model/state/CreatureState.h"
#include "../../service/teleport/TeleportService.h"
#include "../../service/arena/mode/Arena.h"
#include "../../service/arena/ArenaService.h"
#include <sne/server/utility/Profiler.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerArenaController);

PlayerArenaController::PlayerArenaController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerArenaController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerArenaController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerArenaController::soloChallenged(ErrorCode errorCode, MapCode modeMapCode)
{
	onChallengeSolo(errorCode, modeMapCode);
}


void PlayerArenaController::partyChallenged(ErrorCode errorCode, MapCode modeMapCode)
{
	onChallengeParty(errorCode, modeMapCode);
}


void PlayerArenaController::challengeCanceled(ErrorCode errorCode, MapCode modeMapCode)
{
	onCancelChallenge(errorCode, modeMapCode);
}


void PlayerArenaController::arenaMatched(ObjectId arenaId, MapCode modeMapCode, MapCode arenaWorldMapCode)
{
	evArenaMatched(arenaId, modeMapCode, arenaWorldMapCode);
}


void PlayerArenaController::arenaMatchMemberEntered(const ArenaPlayerInfo& memberInfo, ArenaTeamType teamType)
{
    evArenaMatchMemberEntered(teamType, memberInfo);
}


void PlayerArenaController::arenaMatchInfos(const ArenaMatchModeInfo& matchInfo, ArenaTeamType myTeam,
    const ArenaPlayerInfos& teamInfos, const ArenaPlayerInfos& enemyInfos, RegionCode regionCode)
{
    evArenaMatchInfos(matchInfo, myTeam, teamInfos, enemyInfos, regionCode);
}


void PlayerArenaController::arenaMatchMemberLeft(ArenaTeamType teamType, ObjectId playerId)
{
    evArenaMatchMemberLeft(teamType, playerId);
}


void PlayerArenaController::arenaCountdownStarted(sec_t startServerLocalTime)
{
    evArenaCountdownStarted(startServerLocalTime);
}


void PlayerArenaController::arenaPlayerKilled(const GameObjectInfo& killerInfo, const GameObjectInfo& deadManInfo)
{
    evArenaPlayerKilled(killerInfo, deadManInfo);
}


void PlayerArenaController::arenaMatchScoreUpdated(uint32_t blueScore, uint32_t redScore)
{
    evArenaMatchScoreUpdated(blueScore, redScore);
}


void PlayerArenaController::arenaStopped(ArenaModeType arenaMode, ArenaPoint rewardPoint,
    uint32_t resultScore, ArenaResultType resultType)
{
    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    if (! isCancel(resultType)) {
        CharacterArenaPlayResults& results = owner.queryArenaMatchable()->getCharacterArenaPlayResults();
        results.updateResult(arenaMode, resultScore, resultType);
        owner.queryArenaMatchable()->upArenaPoint(rewardPoint);
        {
            DatabaseGuard db(SNE_DATABASE_MANAGER);
            db->asyncUpdateArenaRecord(owner.getAccountId(), owner.getObjectId(), arenaMode, resultScore, resultType);
        }
    }

    evArenaStopped(rewardPoint, resultType);

    owner.queryAchievementable()->updateAchievement(acmtPvPWin, nullptr);
}


void PlayerArenaController::aranaPlayerRevived(ObjectId playerId, const ObjectPosition& position)
{
    evArenaPlayerRevived(playerId, position);
}


void PlayerArenaController::aranaReviveLeftTime(sec_t leftTime)
{
    evReviveLeftTimeInfo(leftTime);
}


void PlayerArenaController::arenaPlayerScoreUpdated(ObjectId playerId, uint32_t score)
{
    evArenaPlayerScoreUpdated(playerId, score);
}


void PlayerArenaController::arenaPlayerSaid(ObjectId playerId, const ChatMessage& message)
{
    evArenaPlayerSaid(playerId, message);
}


void PlayerArenaController::arenaWaypointSetted(ObjectId playerId, const Waypoint& waypoint)
{
    evArenaWaypointSetted(playerId, waypoint);
}


void PlayerArenaController::arenaTeamPlayerMoved(ObjectId playerId, float32_t x, float32_t y)
{
    evArenaTeamPlayerMoved(playerId, x, y);
}


RECEIVE_SRPC_METHOD_1(PlayerArenaController, challengeSolo,
    MapCode, mapCode)
{
	sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (ZONE_SERVICE->isArenaServer()) {
        return;
    }

    if (static_cast<go::Creature&>(owner).getCreatureLevel() < arenaEnterableLevel) {
        onChallengeSolo(ecArenaNotEnterableLevel, mapCode);
        return;
    }
	ZONE_SERVICE->getArenaServerProxy().z2a_challengeSolo(getOwner().getObjectId(), mapCode);
}


RECEIVE_SRPC_METHOD_1(PlayerArenaController, challengeParty,
    MapCode, mapCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwnerAs<go::Entity>();
    if (! owner.isValid()) {
        return;
    }

    if (ZONE_SERVICE->isArenaServer()) {
        return;
    }
	mapCode;
}


RECEIVE_SRPC_METHOD_1(PlayerArenaController, cancelChallenge,
    MapCode, mapCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwnerAs<go::Entity>();
    if (! owner.isValid()) {
        return;
    }

    if (ZONE_SERVICE->isArenaServer()) {
        return;
    }

	ZONE_SERVICE->getArenaServerProxy().z2a_cancelChallenge(getOwner().getObjectId(), mapCode);	
}


RECEIVE_SRPC_METHOD_2(PlayerArenaController, joinArena,
	ObjectId, arenaId, MapCode, arenaMapCode)
{
	sne::server::Profiler profiler(__FUNCTION__);

	go::Entity& owner = getOwner();
	if (! owner.isValid()) {
		return;
	}

    if (ZONE_SERVICE->isArenaServer()) {
        return;
    }

	if (owner.queryCreatureState()->isDied()) {
		onJoinArena(ecArenaDeadNotJoin, MigrationTicket(), 0);
		return;
	}

	MigrationTicket migrationTicket;
	migrationTicket.arenaId_ = arenaId;
	const ErrorCode errorCode =
		TELEPORT_SERVICE->teleportZoneToArena(migrationTicket, owner, arenaMapCode, arenaId);

	onJoinArena(errorCode, migrationTicket, 0);

	if (isFailed(errorCode)) {
		return;
	}

	if (migrationTicket.isValid()) {
        owner.queryNetworkable()->logout();
	}
}


RECEIVE_SRPC_METHOD_0(PlayerArenaController, leaveArena)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! ZONE_SERVICE->isArenaServer()) {
        onLeaveArena(ecArenaNotEnterArenaServer, MigrationTicket());
        return;
    }
    
    ErrorCode errorCode = ecOk;
    MigrationTicket migrationTicket;
	
    Arena* arena = owner.queryArenaMatchable()->getArena();
    if (arena) {
        MapCode preGlobalMapCode = arena->getPreGlobalMapCode(owner.getObjectId());
        if (isValidMapCode(preGlobalMapCode)) {
            errorCode = TELEPORT_SERVICE->teleportArenaToZone(migrationTicket, owner, preGlobalMapCode);            
        }
        arena->leave(getOwner().getAccountId(), owner.getObjectId());
    }
    else {
        errorCode = ecServerInternalError; 
    }

    onLeaveArena(errorCode, migrationTicket);

    if (isSucceeded(errorCode)) {
        if (migrationTicket.isValid()) {
            owner.queryNetworkable()->logout();
        }
    }
}


RECEIVE_SRPC_METHOD_1(PlayerArenaController, sayInArena,
    ChatMessage, message)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwnerAs<go::Entity>();
    if (! owner.isValid()) {
        return;
    }

    if (! ZONE_SERVICE->isArenaServer()) {
        return;
    }
    
	Arena* arena = owner.queryArenaMatchable()->getArena();
	if (arena) {
        arena->say(owner.getObjectId(), message);
    }
}


RECEIVE_SRPC_METHOD_1(PlayerArenaController, setArenaWaypoint,
    Waypoint, waypoint)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    if (! ZONE_SERVICE->isArenaServer()) {
        return;
    }

    Arena* arena = owner.queryArenaMatchable()->getArena();
    if (arena) {
        arena->setWaypoint(owner.getObjectId(), waypoint);
    }
}


FORWARD_SRPC_METHOD_2(PlayerArenaController, onChallengeSolo,
	ErrorCode, errorCode, MapCode, modeMapCode);


FORWARD_SRPC_METHOD_2(PlayerArenaController, onChallengeParty,
	ErrorCode, errorCode, MapCode, modeMapCode);


FORWARD_SRPC_METHOD_2(PlayerArenaController, onCancelChallenge,
	ErrorCode, errorCode, MapCode, modeMapCode);


FORWARD_SRPC_METHOD_3(PlayerArenaController, onJoinArena,
	ErrorCode, errorCode, MigrationTicket, migrationTicket, ArenaPoint, arenaPoint);


FORWARD_SRPC_METHOD_2(PlayerArenaController, onLeaveArena,
    ErrorCode, errorCode, MigrationTicket, migrationTicket);


FORWARD_SRPC_METHOD_3(PlayerArenaController, evArenaMatched,
	ObjectId, arenaId, MapCode, modeMapCode, MapCode, arenaWorldMapCode);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaMatchMemberEntered,
    ArenaTeamType, teamType, ArenaPlayerInfo, playerInfo);


FORWARD_SRPC_METHOD_5(PlayerArenaController, evArenaMatchInfos,
    ArenaMatchModeInfo, matchInfo, ArenaTeamType, myTeam,
    ArenaPlayerInfos, teamInfos, ArenaPlayerInfos, enemyInfos, RegionCode, regionCode);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaMatchMemberLeft,
    ArenaTeamType, teamType, ObjectId, playerId);


FORWARD_SRPC_METHOD_1(PlayerArenaController, evArenaCountdownStarted,
    sec_t, startTime);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaPlayerKilled,
    GameObjectInfo, killerInfo, GameObjectInfo, deadManInfo);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaMatchScoreUpdated,
    uint32_t, blueScore, uint32_t, redScore);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaStopped,
    ArenaPoint, rewardPoint, ArenaResultType, resultType);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaPlayerRevived,
    ObjectId, playerId, ObjectPosition, position);


FORWARD_SRPC_METHOD_1(PlayerArenaController, evReviveLeftTimeInfo,
    sec_t, leftTime);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaPlayerScoreUpdated,
    ObjectId, playerId, uint32_t, scoreId);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaPlayerSaid,
    ObjectId, playerId, ChatMessage, message);


FORWARD_SRPC_METHOD_2(PlayerArenaController, evArenaWaypointSetted,
    ObjectId, playerId, Waypoint, waypoint);


FORWARD_SRPC_METHOD_3(PlayerArenaController, evArenaTeamPlayerMoved,
    ObjectId, playerId, float32_t, x, float32_t, y)

// = sne::srpc::RpcForwarder overriding

void PlayerArenaController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerArenaController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerArenaController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {