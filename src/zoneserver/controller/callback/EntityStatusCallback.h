#pragma once

#include <gideon/cs/shared/data/EntityStatusInfo.h>

namespace gideon { namespace zoneserver { namespace go {
class Npc;
class Creature;
class Player;
class GraveStone;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class EntityStatusCallback
 */
class EntityStatusCallback
{
public:
    virtual ~EntityStatusCallback() {}

public:
    virtual void playerDied(const go::Player& player, const GameObjectInfo& killerInfo) = 0;

	virtual void playerRevived(const go::Player& player) = 0;

    virtual void npcDied(const GameObjectInfo& npcInfo, const GameObjectInfo& killerInfo, go::GraveStone* graveStone) = 0;

public:
    /// Entity가 위협을 감지하였다
    virtual void entityThreated(const GameObjectInfo& entityInfo) = 0;

    virtual void entityTargetSelected(const GameObjectInfo& entityInfo, const GameObjectInfo& target, bool isAgressive) = 0;

    virtual void entityTargetChanged(const GameObjectInfo& entityInfo, const EntityStatusInfo& targetStatusInfo) = 0;

public:
    virtual void creaturePointsRestored(const go::Creature& creature) = 0;

public:
    virtual void playerCombatStateChanged(bool isCombatState) = 0;

public:
    virtual void begginerProtectionReleased() = 0;
    virtual void begginerProtectionReleased(ObjectId playerId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
