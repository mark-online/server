#pragma once

#include "../../../model/gameobject/EntityEvent.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/ArenaCallback.h"
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

/**
 * @class ArenaEnterPlayerEvent
 */
class ArenaEnterPlayerEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaEnterPlayerEvent>
{
public:
    ArenaEnterPlayerEvent(const ArenaPlayerInfo& addPlyaerInfo, ArenaTeamType enterTeamType) :
        addPlyaerInfo_(addPlyaerInfo),
        enterTeamType_(enterTeamType) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaMatchMemberEntered(addPlyaerInfo_, enterTeamType_);
        }
    }

private:
    const ArenaPlayerInfo addPlyaerInfo_;
    const ArenaTeamType enterTeamType_;
};


/**
 * @class ArenaLeavePlayerEvent
 */
class ArenaLeavePlayerEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaLeavePlayerEvent>
{
public:
    ArenaLeavePlayerEvent(ObjectId leaveId, ArenaTeamType leaveTeamType) :
        leaveId_(leaveId),
        leaveTeamType_(leaveTeamType) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaMatchMemberLeft(leaveTeamType_, leaveId_);
        }
    }

private:
    ObjectId leaveId_;
    ArenaTeamType leaveTeamType_;
};


/**
 * @class ArenaStartCountDownEvent
 */
class ArenaStartCountDownEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaStartCountDownEvent>
{
public:
    ArenaStartCountDownEvent(sec_t serverLocalTime) :
        serverLocalTime_(serverLocalTime) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaCountdownStarted(serverLocalTime_);
        }
    }
private:
    sec_t serverLocalTime_;
};


/**
 * @class ArenaPlayerKillEvent
 */
class ArenaPlayerKillEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaPlayerKillEvent>
{
public:
    ArenaPlayerKillEvent(const GameObjectInfo& killerInfo, const GameObjectInfo& deadManInfo) :
        killerInfo_(killerInfo),
        deadManInfo_(deadManInfo)
    {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaPlayerKilled(killerInfo_, deadManInfo_);
        }
    }

private:
    GameObjectInfo killerInfo_;
    GameObjectInfo deadManInfo_;
};


/**
 * @class ArenaScoreUpdateEvent
 */
class ArenaScoreUpdateEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaScoreUpdateEvent>
{
public:
    ArenaScoreUpdateEvent(uint32_t blueTeamScore, uint32_t redTeamScore) :
        blueTeamScore_(blueTeamScore),
        redTeamScore_(redTeamScore)
      {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaMatchScoreUpdated(blueTeamScore_, redTeamScore_);
        }
    }

private:
    uint32_t blueTeamScore_;
    uint32_t redTeamScore_;
};


/**
 * @class ArenaPlayerScoreUpdateEvent
 */
class ArenaPlayerScoreUpdateEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaPlayerScoreUpdateEvent>
{
public:
    ArenaPlayerScoreUpdateEvent(ObjectId playerId, uint32_t score) :
        playerId_(playerId),
        score_(score)
      {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaPlayerScoreUpdated(playerId_, score_);
        }
    }

private:
    ObjectId playerId_;
    uint32_t score_;
};


/**
 * @class ArenaPlayerSayEvent
 */
class ArenaPlayerSayEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaPlayerSayEvent>
{
public:
    ArenaPlayerSayEvent(ObjectId playerId, const ChatMessage& message) :
        playerId_(playerId),
        chatMessage_(message)
      {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaPlayerSaid(playerId_, chatMessage_);
        }
    }

private:
    ObjectId playerId_;
    ChatMessage chatMessage_;
};


/**
 * @class ArenaPlayerReviveEvent
 */
class ArenaPlayerReviveEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaPlayerReviveEvent>
{
public:
    ArenaPlayerReviveEvent(ObjectId playerId, const ObjectPosition& position) :
        playerId_(playerId),
        position_(position)
      {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->aranaPlayerRevived(playerId_, position_);
        }
    }

private:
    ObjectId playerId_;
    ObjectPosition position_;
};


/**
 * @class ArenaWaypointEvent
 */
class ArenaWaypointEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaWaypointEvent>
{
public:
    ArenaWaypointEvent(ObjectId playerId, const Waypoint& waypoint) :
        playerId_(playerId),
        waypoint_(waypoint)
      {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaWaypointSetted(playerId_, waypoint_);
        }
    }

private:
    ObjectId playerId_;
    Waypoint waypoint_;
};


/**
 * @class ArenaTeamPlayerMoveEvent
 */
class ArenaTeamPlayerMoveEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ArenaTeamPlayerMoveEvent>
{
public:
    ArenaTeamPlayerMoveEvent(ObjectId playerId, float32_t x, float32_t y) :
        playerId_(playerId),
        x_(x),
        y_(y)
    {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        gc::ArenaCallback* callback = entity.getController().queryArenaCallback();
        if (callback) {
            callback->arenaTeamPlayerMoved(playerId_, x_, y_);
        }
    }

private:
    ObjectId playerId_;
    float32_t x_;
    float32_t y_;
};

}} // namespace gideon { namespace zoneserver {