#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/ArenaCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/ArenaRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerArenaController
 * 투기장 담당
 */
class ZoneServer_Export PlayerArenaController : public Controller,
    public rpc::ArenaRpc,
	public ArenaCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerArenaController);
public:
    PlayerArenaController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

public:
	virtual void soloChallenged(ErrorCode errorCode, MapCode modeMapCode);
	virtual void partyChallenged(ErrorCode errorCode, MapCode modeMapCode);
	virtual void challengeCanceled(ErrorCode errorCode, MapCode modeMapCode);
	virtual void arenaMatched(ObjectId arenaId, MapCode modeMapCode, MapCode arenaWorldMapCode);
    virtual void arenaMatchMemberEntered(const ArenaPlayerInfo& memberInfo, ArenaTeamType teamType);
    virtual void arenaMatchInfos(const ArenaMatchModeInfo& matchInfo, ArenaTeamType myTeam,
        const ArenaPlayerInfos& teamInfos, const ArenaPlayerInfos& enemyInfos, RegionCode regionCode);
    virtual void arenaMatchMemberLeft(ArenaTeamType teamType, ObjectId playerId);
    virtual void arenaCountdownStarted(sec_t startServerLocalTime);
    virtual void arenaPlayerKilled(const GameObjectInfo& killerInfo, const GameObjectInfo& deadManInfo);
    virtual void arenaMatchScoreUpdated(uint32_t blueScore, uint32_t redScore);
    virtual void arenaStopped(ArenaModeType arenaMode, ArenaPoint rewardPoint,
        uint32_t resultScore, ArenaResultType resultType);
    virtual void aranaPlayerRevived(ObjectId playerId, const ObjectPosition& position);
    virtual void aranaReviveLeftTime(sec_t leftTime);
    virtual void arenaPlayerScoreUpdated(ObjectId playerId, uint32_t score);
    virtual void arenaPlayerSaid(ObjectId playerId, const ChatMessage& message);
    virtual void arenaWaypointSetted(ObjectId playerId, const Waypoint& waypoint);
    virtual void arenaTeamPlayerMoved(ObjectId playerId, float32_t x, float32_t y);

public:
    OVERRIDE_SRPC_METHOD_1(challengeSolo,
        MapCode, modeMapCode);
    OVERRIDE_SRPC_METHOD_1(challengeParty,
        MapCode, modeMapCode);
    OVERRIDE_SRPC_METHOD_1(cancelChallenge,
        MapCode, modeMapCode);
	OVERRIDE_SRPC_METHOD_2(joinArena,
		ObjectId, arenaId, MapCode, arenaWorldMapCode);
    OVERRIDE_SRPC_METHOD_0(leaveArena);
    OVERRIDE_SRPC_METHOD_1(sayInArena,
        ChatMessage, message);
    OVERRIDE_SRPC_METHOD_1(setArenaWaypoint,
        Waypoint, waypoint);

	OVERRIDE_SRPC_METHOD_2(onChallengeSolo,
		ErrorCode, errorCode, MapCode, modeMapCode);
	OVERRIDE_SRPC_METHOD_2(onChallengeParty,
		ErrorCode, errorCode, MapCode, modeMapCode);
	OVERRIDE_SRPC_METHOD_2(onCancelChallenge,
		ErrorCode, errorCode, MapCode, modeMapCode);
	OVERRIDE_SRPC_METHOD_3(onJoinArena,
		ErrorCode, errorCode, MigrationTicket, migrationTicket, ArenaPoint, arenaPoint);
    OVERRIDE_SRPC_METHOD_2(onLeaveArena,
        ErrorCode, errorCode, MigrationTicket, migrationTicket);

	OVERRIDE_SRPC_METHOD_3(evArenaMatched,
		ObjectId, arenaId, MapCode, modeMapCode, MapCode, arenaWorldMapCode);
    OVERRIDE_SRPC_METHOD_2(evArenaMatchMemberEntered,
        ArenaTeamType, teamType, ArenaPlayerInfo, playerInfo);
    OVERRIDE_SRPC_METHOD_5(evArenaMatchInfos,
        ArenaMatchModeInfo, matchInfo, ArenaTeamType, myTeam,
        ArenaPlayerInfos, teamInfos, ArenaPlayerInfos, enemyInfos, RegionCode, regionCode);    
    OVERRIDE_SRPC_METHOD_2(evArenaMatchMemberLeft,
        ArenaTeamType, teamType, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evArenaCountdownStarted,
        sec_t, startTime);
    OVERRIDE_SRPC_METHOD_2(evArenaPlayerKilled,
        GameObjectInfo, killerInfo, GameObjectInfo, deadManInfo);
    OVERRIDE_SRPC_METHOD_2(evArenaMatchScoreUpdated,
        uint32_t, blueScore, uint32_t, redScore);
    OVERRIDE_SRPC_METHOD_2(evArenaStopped,
        ArenaPoint, rewardPoint, ArenaResultType, resultType);
    OVERRIDE_SRPC_METHOD_2(evArenaPlayerRevived,
        ObjectId, playerId, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(evReviveLeftTimeInfo,
        sec_t, leftTime);
    OVERRIDE_SRPC_METHOD_2(evArenaPlayerScoreUpdated,
        ObjectId, playerId, uint32_t, scoreId);
    OVERRIDE_SRPC_METHOD_2(evArenaPlayerSaid,
        ObjectId, playerId, ChatMessage, message);
    OVERRIDE_SRPC_METHOD_2(evArenaWaypointSetted,
        ObjectId, playerId, Waypoint, waypoint);
    OVERRIDE_SRPC_METHOD_3(evArenaTeamPlayerMoved,
        ObjectId, playerId, float32_t, x, float32_t, y);
private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
