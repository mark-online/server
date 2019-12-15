#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/ArenaInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/RegionInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Chatting.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class ArenaCallback
 */
class ArenaCallback
{
public:
    virtual ~ArenaCallback() {}

public:
	virtual void soloChallenged(ErrorCode errorCode, MapCode modeMapCode) = 0;
	virtual void partyChallenged(ErrorCode errorCode, MapCode modeMapCode) = 0;
	virtual void challengeCanceled(ErrorCode errorCode, MapCode modeMapCode) = 0;
	virtual void arenaMatched(ObjectId arenaId, MapCode modeMapCode, MapCode arenaWorldMapCode) = 0;
    virtual void arenaMatchMemberEntered(const ArenaPlayerInfo& memberInfo, ArenaTeamType teamType) = 0;
    virtual void arenaMatchInfos(const ArenaMatchModeInfo& matchInfo, ArenaTeamType myTeam,
        const ArenaPlayerInfos& teamInfos, const ArenaPlayerInfos& enemyInfos, RegionCode regionCode) = 0;
    virtual void arenaMatchMemberLeft(ArenaTeamType teamType, ObjectId playerId) = 0;
    virtual void arenaCountdownStarted(sec_t startServerLocalTime) = 0;
    virtual void arenaPlayerKilled(const GameObjectInfo& killerId, const GameObjectInfo& deadManId) = 0;
    virtual void arenaMatchScoreUpdated(uint32_t blueScore, uint32_t redScore) = 0;
    virtual void arenaStopped(ArenaModeType arenaMode, ArenaPoint rewardPoint, 
        uint32_t resultScore, ArenaResultType resultType) = 0;
    virtual void aranaPlayerRevived(ObjectId playerId, const ObjectPosition& position) = 0;
    virtual void aranaReviveLeftTime(sec_t leftTime) = 0;
    virtual void arenaPlayerScoreUpdated(ObjectId playerId, uint32_t score) = 0;
    virtual void arenaPlayerSaid(ObjectId playerId, const ChatMessage& message) = 0;
    virtual void arenaWaypointSetted(ObjectId playerId, const Waypoint& waypoint) = 0;
    virtual void arenaTeamPlayerMoved(ObjectId playerId, float32_t x, float32_t y) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
