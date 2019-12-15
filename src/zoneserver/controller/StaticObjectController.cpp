#include "ZoneServerPCH.h"
#include "StaticObjectController.h"
#include "callback/StatsCallback.h"
#include "../model/gameobject/StaticObject.h"
#include "../model/gameobject/Creature.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/skilleffect/CreatureEffectScriptApplier.h"
#include "../model/gameobject/skilleffect/StaticObjectEffectScriptApplier.h"
#include "../model/gameobject/ability/Thinkable.h"
#include "../ai/Brain.h"
#include "../ai/aggro/AggroList.h"
#include "../service/spawn/SpawnService.h"
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class StaticObjectHpChangeEvent
 */
class StaticObjectHpChangeEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<StaticObjectHpChangeEvent>
{
public:
    StaticObjectHpChangeEvent(const GameObjectInfo& gameObjectInfo,
		HitPoint hitPoint) :
        gameObjectInfo_(gameObjectInfo),
		hitPoint_(hitPoint) {}

private:
    virtual void call(go::Entity& entity) {
		StatsCallback* statsCallback =
			entity.getController().queryStatsCallback();
		if (statsCallback) {
			statsCallback->pointChanged(gameObjectInfo_, ptHp, hitPoint_);
		}
    }

private:
    const GameObjectInfo gameObjectInfo_;
	const HitPoint hitPoint_;
};


/**
 * @class StaticObjectMpChangeEvent
 */
class StaticObjectMpChangeEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<StaticObjectMpChangeEvent>
{
public:
    StaticObjectMpChangeEvent(const GameObjectInfo& gameObjectInfo,
		ManaPoint manaPoint) :
        gameObjectInfo_(gameObjectInfo),
		manaPoint_(manaPoint) {}

private:
    virtual void call(go::Entity& entity) {
		StatsCallback* statsCallback =
			entity.getController().queryStatsCallback();
		if (statsCallback) {
			statsCallback->pointChanged(gameObjectInfo_, ptMp, manaPoint_);
		}
    }

private:
    const GameObjectInfo gameObjectInfo_;
	const ManaPoint manaPoint_;
};

} // namespace {

// = StaticObjectController

// = EntityController overriding

void StaticObjectController::spawned(WorldMap& worldMap)
{
    Parent::spawned(worldMap);

    getOwner().reserveDespawn();
}


void StaticObjectController::despawned(WorldMap& worldMap)
{
    Parent::despawned(worldMap);

    getOwner().reserveRespawn(worldMap);
}

// = EffectCallback override

void StaticObjectController::applyEffect(go::Entity& from,
    const SkillEffectResult& skillEffectResult)
{       
    go::EffectScriptApplier& skillAppler = getOwner().getEffectScriptApplier();
    skillAppler.applySkill(from, skillEffectResult);
}


void StaticObjectController::effectApplied(go::Entity& /*to*/, go::Entity& /*from*/,
    const SkillEffectResult& /*skillEffectResult*/)
{
    /*if (to.isSame(getOwner()) && from.isEffectInfoable()) {
        go::Thinkable* thinkable = getOwner().queryThinkable();
        if (thinkable != nullptr) {
            ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
            if (aggroList != nullptr) {
                aggroList->addEffectInfo(from, skillEffectResult);
            }
        }
    }*/
}

// = StaticObjectEffectCallback override

void StaticObjectController::damaged(HitPoint currentPoint, bool observerNotify, bool exceptSelf)
{
    hpChanged(currentPoint, observerNotify, exceptSelf);
}


void StaticObjectController::repaired(HitPoint currentPoint, bool observerNotify, bool exceptSelf)
{
    hpChanged(currentPoint, observerNotify, exceptSelf);
}


void StaticObjectController::redueced(ManaPoint currentPoint, bool observerNotify, bool exceptSelf)
{
    mpChanged(currentPoint, observerNotify, exceptSelf);
}


void StaticObjectController::refilled(ManaPoint currentPoint, bool observerNotify, bool exceptSelf)
{
    mpChanged(currentPoint, observerNotify, exceptSelf);
}


void StaticObjectController::hpChanged(HitPoint currentPoint, bool observerNotify, bool exceptSelf)
{
    go::StaticObject& so = getOwnerAs<go::StaticObject>();
    auto event = std::make_shared<StaticObjectHpChangeEvent>(so.getGameObjectInfo(), currentPoint);
    if (observerNotify) {
        so.notifyToOberversOfObservers(event, exceptSelf);
    }
    else {
        go::Knowable* knowable = so.queryKnowable();
        if (knowable) {
            knowable->broadcast(event, exceptSelf);
        }
    }
}


void StaticObjectController::mpChanged(ManaPoint currentPoint, bool observerNotify, bool exceptSelf)
{
    go::StaticObject& so = getOwnerAs<go::StaticObject>();
    auto event = std::make_shared<StaticObjectMpChangeEvent>(so.getGameObjectInfo(), currentPoint);
    if (observerNotify) {
        so.notifyToObervers(event, exceptSelf);
        // TODO: MP가 대상에 표시가 된다면 notifyToOberversOfObservers() 호출로 변경해야 한다
    }
    else {
        go::Knowable* knowable = so.queryKnowable();
        if (knowable) {
            knowable->broadcast(event, exceptSelf);
        }
    }
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
