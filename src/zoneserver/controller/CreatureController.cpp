#include "ZoneServerPCH.h"
#include "CreatureController.h"
#include "MoveController.h"
#include "SkillController.h"
#include "CreatureEffectController.h"
#include "callback/StatsCallback.h"
#include "../world/WorldMap.h"
#include "../service/skill/Skill.h"
#include "../service/movement/MovementManager.h"
#include "../model/gameobject/Creature.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/status/CreatureStatus.h"
#include "../model/gameobject/ability/SkillCastable.h"
#include "../model/gameobject/ability/Castable.h"
#include "../model/gameobject/skilleffect/CreatureEffectScriptApplier.h"

namespace gideon { namespace zoneserver { namespace gc {


// = CreatureController

EffectCallback* CreatureController::queryEffectCallback()
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	return &owner.getEffectController();
}


CreatureEffectCallback* CreatureController::queryCreatureEffectCallback()
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    return &owner.getEffectController();
}


MovementCallback* CreatureController::queryMovementCallback()
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	return &owner.getMoveController();
}


void CreatureController::died(go::Entity* /*from*/)
{
    go::Creature& owner = getOwnerAs<go::Creature>();

    go::SkillCastable* skillCastable = owner.querySkillCastable();
    if (skillCastable != nullptr) {
        skillCastable->cancelAll();
    }

	go::Castable* castable = owner.queryCastable();
	if (castable) {
		castable->cancelCasting();
	}

    owner.getMoveController().stop();
}

// = EntityController overriding

void CreatureController::spawned(WorldMap& worldMap)
{
    EntityController::spawned(worldMap);
}


void CreatureController::despawned(WorldMap& worldMap)
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    owner.queryKnowable()->forgetAll();

    owner.queryMoveable()->getMovementManager().reset();

    EntityController::despawned(worldMap);
}

// = InterestAreaCallback overriding

void CreatureController::entitiesAppeared(const go::EntityMap& entities)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Creature>().queryKnowable()->know(entities);
}


void CreatureController::entityAppeared(go::Entity& entity,
    const UnionEntityInfo& entityInfo)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    entityInfo;
    getOwnerAs<go::Creature>().queryKnowable()->know(entity);
}


void CreatureController::entitiesDisappeared(const go::EntityMap& entities)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Creature>().queryKnowable()->forget(entities);
}


void CreatureController::entityDisappeared(go::Entity& entity)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Creature>().queryKnowable()->forget(entity);
}


void CreatureController::entityDisappeared(const GameObjectInfo& info)
{
    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    getOwnerAs<go::Creature>().queryKnowable()->forget(info);
}


}}} // namespace gideon { namespace zoneserver { namespace gc {