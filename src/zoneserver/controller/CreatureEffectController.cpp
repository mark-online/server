#include "ZoneServerPCH.h"
#include "CreatureEffectController.h"
#include "EntityController.h"
#include "PlayerController.h"
#include "MoveController.h"
#include "../model/state/CreatureState.h"
#include "../model/state/SkillCasterState.h"
#include "../model/gameobject/skilleffect/CreatureEffectScriptApplier.h"
#include "../model/gameobject/Creature.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/ability/SkillCastable.h"
#include "../model/gameobject/ability/Castable.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/ability/Liveable.h"
#include "../model/gameobject/ability/Partyable.h"
#include "../service/movement/MovementManager.h"
#include "../world/WorldMap.h"
#include "../world/Sector.h"
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class MesmerizationEvent
 */
class MesmerizationEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<MesmerizationEvent>
{
public:
    MesmerizationEvent(const GameObjectInfo& creatureInfo, 
        MesmerizationType type, bool isActivate) :
        creatureInfo_(creatureInfo),
        type_(type),
        isActivate_(isActivate) {}

private:
    virtual void call(go::Entity& entity) {
        CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
        if (callback != nullptr) {
            callback->mesmerizationEffected(creatureInfo_, type_, isActivate_);
        }
    }

private:
    const GameObjectInfo creatureInfo_;
    MesmerizationType type_;
    const bool isActivate_;
};


/**
 * @class AddEffectEvent
 */
class AddEffectEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<AddEffectEvent>
{
public:
    AddEffectEvent(const GameObjectInfo& objectInfo, const DebuffBuffEffectInfo& info) :
		objectInfo_(objectInfo),
        info_(info) {}

private:
    virtual void call(go::Entity& entity) {
		CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
		if (callback != nullptr) {
			callback->effectAdded(objectInfo_, info_);
		}
    }

private:
	const GameObjectInfo objectInfo_;
    const DebuffBuffEffectInfo info_;
};


/**
 * @class RemoveEffectsEvent
 */
class RemoveEffectEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<RemoveEffectEvent>
{
public:
    RemoveEffectEvent(const GameObjectInfo& objectInfo, DataCode dataCode, bool isCaster) :
        objectInfo_(objectInfo),
        dataCode_(dataCode),
        isCaster_(isCaster) {}

private:
    virtual void call(go::Entity& entity) {
        CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
        if (callback != nullptr) {
            callback->effectRemoved(objectInfo_, dataCode_, isCaster_);
        }
    }

private:
    const GameObjectInfo objectInfo_;
    DataCode dataCode_;
    bool isCaster_;
};


/**
 * @class CreatureMutateEvent
 */
class CreatureMutateEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CreatureMutateEvent>
{
public:
    CreatureMutateEvent(const GameObjectInfo& info, NpcCode npcCode, bool isActivate) :
		info_(info),
        npcCode_(npcCode),
        isActivate_(isActivate) {}

private:
    virtual void call(go::Entity& entity) {
		CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
		if (callback != nullptr) {
			callback->mutated(info_, npcCode_, isActivate_);
		}
    }

private:
	const GameObjectInfo info_;
    const NpcCode npcCode_;
    const bool isActivate_;
};


/**
 * @class CreatureTransformEvent
 */
class CreatureTransformEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CreatureTransformEvent>
{
public:
    CreatureTransformEvent(const GameObjectInfo& info, NpcCode npcCode, bool isActivate) :
		info_(info),
        npcCode_(npcCode),
        isActivate_(isActivate) {}

private:
    virtual void call(go::Entity& entity) {
		CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
		if (callback != nullptr) {
			callback->transformed(info_, npcCode_, isActivate_);
		}
    }

private:
	const GameObjectInfo info_;
    const NpcCode npcCode_;
    const bool isActivate_;
};


/**
 * @class CreatureReleaseHideEvent
 */
class CreatureReleaseHideEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CreatureReleaseHideEvent>
{
public:
    CreatureReleaseHideEvent(const UnionEntityInfo& info) :
		info_(info) {}

private:
    virtual void call(go::Entity& entity) {
		CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
		if (callback != nullptr) {
			callback->releaseHidden(info_);
		}
    }

private:
	const UnionEntityInfo info_;
};


/**
 * @class CreatureChangeSpeedEvent
 */
class CreatureChangeSpeedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CreatureChangeSpeedEvent>
{
public:
    CreatureChangeSpeedEvent(const GameObjectInfo& info, float32_t speed) :
		info_(info),
        speed_(speed) {}

private:
    virtual void call(go::Entity& entity) {
		CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
		if (callback != nullptr) {
			callback->moveSpeedChanged(info_, speed_);
		}
    }

private:
	const GameObjectInfo info_;
    const float32_t speed_;
};


/**
 * @class CreatureFrenzyEvent
 */
class CreatureFrenzyEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CreatureFrenzyEvent>
{
public:
    CreatureFrenzyEvent(const GameObjectInfo& info, float32_t currentScale, bool isActivate) :
		info_(info),
        currentScale_(currentScale),
        isActivate_(isActivate) {}

private:
    virtual void call(go::Entity& entity) {
		CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
		if (callback != nullptr) {
			callback->frenzied(info_, currentScale_, isActivate_);
		}
    }

private:
	const GameObjectInfo info_;
    const float32_t currentScale_;
    const bool isActivate_;
};


/**
 * @class KnockBackEvent
 */
class KnockBackEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<KnockBackEvent>
{
public:
    KnockBackEvent(const GameObjectInfo& creatureInfo, const Position& position) :
        creatureInfo_(creatureInfo),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
        if (callback != nullptr) {
            callback->knockbacked(creatureInfo_, position_);
        }
    }

private:
    const GameObjectInfo creatureInfo_;
    const Position position_;
};


/**
 * @class KnockBackReleaseEvent
 */
class KnockBackReleaseEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<KnockBackReleaseEvent>
{
public:
    KnockBackReleaseEvent(const GameObjectInfo& creatureInfo) :
        creatureInfo_(creatureInfo) {}

private:
    virtual void call(go::Entity& entity) {
        CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
        if (callback != nullptr) {
            callback->knockbackReleased(creatureInfo_);
        }
    }

private:
    const GameObjectInfo creatureInfo_;
};


/**
 * @class DashEvent
 */
class DashEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<DashEvent>
{
public:
    DashEvent(const GameObjectInfo& creatureInfo, const Position& position) :
        creatureInfo_(creatureInfo),
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
        if (callback != nullptr) {
            callback->dashing(creatureInfo_, position_);
        }
    }

private:
    const GameObjectInfo creatureInfo_;
    const Position position_;
};


/**
 * @class ReviveEffectEvent
 */
class ReviveEffectEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ReviveEffectEvent>
{
public:
    ReviveEffectEvent(const GameObjectInfo& creatureInfo, const ObjectPosition& position, HitPoint currentHp) :
        creatureInfo_(creatureInfo),
        position_(position),
        currentHp_(currentHp) {}

private:
    virtual void call(go::Entity& entity) {
        CreatureEffectCallback* callback = 
            entity.getController().queryCreatureEffectCallback();
        if (callback != nullptr) {
            callback->reviveEffected(creatureInfo_, position_, currentHp_);
        }
    }

private:
    const GameObjectInfo creatureInfo_;
    const ObjectPosition position_;
    const HitPoint currentHp_;
};

/**
 * @class HitEffectEvent
 */
class HitEffectEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<HitEffectEvent>
{
public:
    HitEffectEvent(go::Entity& to, go::Entity& from, DataCode dataCode) :
      to_(to),
      from_(from),
      dataCode_(dataCode) {}

private:
    virtual void call(go::Entity& entity) {
        EffectCallback* callback = entity.getController().queryEffectCallback();
        if (callback != nullptr) {
            callback->effectHit(to_, from_, dataCode_);
        }
    }

private:
    go::Entity& to_;
    go::Entity& from_;
    DataCode dataCode_;
};


} // namespace {

// = CreatureEffectController

CreatureEffectController::CreatureEffectController(go::Entity* owner) :
	Controller(owner)
{
}

// = EffectCallback overriding

void CreatureEffectController::applyEffect(go::Entity& from,
	const SkillEffectResult& skillEffectResult)
{
	go::EffectScriptApplier& skillAppler = getOwner().getEffectScriptApplier();
	skillAppler.applySkill(from, skillEffectResult);
}


void CreatureEffectController::hitEffect(go::Entity& from, DataCode dataCode)
{
    auto event = std::make_shared<HitEffectEvent>(getOwner(), from, dataCode);
    getOwner().queryKnowable()->broadcast(event);
}


void CreatureEffectController::cancelEffect(EffectStackCategory category)
{
    go::EffectScriptApplier& casterSkillApplyer = getOwner().getEffectScriptApplier();
    casterSkillApplyer.cancelRemoveEffect(category);
}


void CreatureEffectController::addEffect(const DebuffBuffEffectInfo& info)
{
    auto event = std::make_shared<AddEffectEvent>(getOwner().getGameObjectInfo(), info);
	getOwner().notifyToOberversOfObservers(event, true);
}


void CreatureEffectController::removeEffect(DataCode dataCode, bool isCaster)
{
    auto event = std::make_shared<RemoveEffectEvent>(getOwner().getGameObjectInfo(), dataCode, isCaster);
	getOwner().notifyToOberversOfObservers(event);
}

// = CreatureEffectCallback overriding

void CreatureEffectController::effectiveMesmerization(MesmerizationType mezt, bool isActivate)
{
    go::Creature& owner = getOwnerAs<go::Creature>();
	CreatureState* creatureState = owner.queryCreatureState();
	if (! creatureState) {
		return;
	}

    if (mezt != meztHide && isActivate && creatureState->isHidden()) {
        mesmerizationEffected(meztHide, false);       
        auto event = std::make_shared<CreatureReleaseHideEvent>(owner.getUnionEntityInfo());
        owner.queryKnowable()->broadcast(event);
    }

    if (mesmerizationEffected(mezt, isActivate)) {
        if (! isActivate && mezt == meztHide) {
            // 패킷 크기를 줄이고자 자기자신은 따로 처리..
            CreatureEffectCallback* callback = 
                owner.getController().queryCreatureEffectCallback();
            if (callback != nullptr) {
                callback->mesmerizationEffected(owner.getGameObjectInfo(), meztHide, false);
            }
            auto event = std::make_shared<CreatureReleaseHideEvent>(owner.getUnionEntityInfo());
            owner.queryKnowable()->broadcast(event, true);
        }
        else if (mezt == meztFrenzy) {
            auto event = std::make_shared<CreatureFrenzyEvent>(owner.getGameObjectInfo(),
                owner.getCurrentScale(), isActivate);
            owner.queryKnowable()->broadcast(event);
        }
        else {
            auto event = std::make_shared<MesmerizationEvent>(getOwner().getGameObjectInfo(),
                mezt, isActivate);
            owner.queryKnowable()->broadcast(event);
        }

		if (isActivate && (shouldCancelCasting(mezt))) {
			go::Castable* castable = owner.queryCastable();
			if (castable) {
				if (castable->isCasting()) {
					castable->cancelCasting();
				}
			}
		}
    }
}


void CreatureEffectController::mutate(NpcCode npcCode, sec_t duration, bool isActivate)
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	CreatureState* creatureState = owner.queryCreatureState();
	if (! creatureState) {
		return;
	}

    go::Castable* castable = owner.queryCastable();
    if (castable) {
        if (castable->isCasting() && isActivate) {
            castable->cancelCasting();
        }
    }

    if (creatureState->mutated(npcCode, duration, isActivate)) {
        auto event = std::make_shared<CreatureMutateEvent>(owner.getGameObjectInfo(), npcCode, isActivate);
        owner.queryKnowable()->broadcast(event);
    }
    if (isActivate && creatureState->isHidden()) {
        owner.getEffectScriptApplier().cancelRemoveEffect(escHide);
    }
}


void CreatureEffectController::transform(NpcCode npcCode, bool isActivate)
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	CreatureState* creatureState = owner.queryCreatureState();
	if (! creatureState) {
		return;
	}

    if (isActivate && creatureState->isHidden()) {
        owner.getEffectScriptApplier().cancelRemoveEffect(escHide);
    }

    if (creatureState->transformed(npcCode, isActivate)) {
        auto event = std::make_shared<CreatureTransformEvent>(
            owner.getGameObjectInfo(), npcCode, isActivate);
        owner.queryKnowable()->broadcast(event);
    }

}


void CreatureEffectController::panic(sec_t duration, bool isActivate)
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    CreatureState* creatureState = owner.queryCreatureState();
    if (! creatureState) {
        return;
    }

    go::Castable* castable = owner.queryCastable();
    if (castable) {
        if (castable->isCasting() && isActivate) {
            castable->cancelCasting();
        }
    }

    if (isActivate && creatureState->isHidden()) {
        owner.getEffectScriptApplier().cancelRemoveEffect(escHide);
    }

    if (creatureState->panicked(duration, isActivate)) {
        auto event = std::make_shared<MesmerizationEvent>(
            getOwner().getGameObjectInfo(), meztPanic, isActivate);
        owner.queryKnowable()->broadcast(event);
    }
}


void CreatureEffectController::changeMoveSpeed(float32_t oldSpeed)
{
    go::Moveable* moveable = getOwner().queryMoveable();
    assert(moveable != nullptr);
    moveable->updateSpeed();
    const float32_t newSpeed = moveable->getSpeed();
    if (fabs(newSpeed - oldSpeed) < 0.1f) {
        return;
    }

    auto event = std::make_shared<CreatureChangeSpeedEvent>(
        getOwner().getGameObjectInfo(), newSpeed);
    getOwner().queryKnowable()->broadcast(event);    
}


void CreatureEffectController::knockback(const Position& effectPosition)
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	CreatureState* creatureState = owner.queryCreatureState();
	if (! creatureState) {
		return;
	}

    go::Castable* castable = owner.queryCastable();
    if (castable) {
        if (castable->isCasting()) {
            castable->cancelCasting();
        }
    }

    if (creatureState->isHidden()) {
        owner.getEffectScriptApplier().cancelRemoveEffect(escHide);
    }

    if (creatureState->knockbacked(true)) {
        ObjectPosition position = owner.getPosition();
        static_cast<Position&>(position) = effectPosition;
        owner.setPosition(position);

        auto event = std::make_shared<KnockBackEvent>(
            owner.getGameObjectInfo(), effectPosition);
        owner.queryKnowable()->broadcast(event);

        // TODO: 이동 동기화
        //if (owner.isNpcOrMonster()) {
        //    owner.queryMoveable()->getMovementManager().setKnockbackMovement(position);
        //}
    }
}


void CreatureEffectController::dash(const Position& effectPosition)
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }
    ObjectPosition source = owner.getPosition();
    ObjectPosition destin(effectPosition, source.heading_);
    const Sector* destinSector = worldMap->migrate(owner, source, destin);
    if (destinSector) {
        owner.setSectorAddress(destinSector->getAddress());
    }

    owner.setPosition(destin);
    
    auto event = std::make_shared<DashEvent>(
        owner.getGameObjectInfo(), effectPosition);
    owner.queryKnowable()->broadcast(event);

    // TODO: 이동 동기화
    //if (owner.isNpcOrMonster()) {
    //    owner.queryMoveable()->getMovementManager().setDashMovement(position);
    //}
    
}


void CreatureEffectController::reviveAtOnce(permil_t perRefillHp)
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    HitPoint currentPoint = hpMin;
    go::Liveable* liveable = owner.queryLiveable();
    if (liveable && liveable->reviveByEffect(currentPoint, perRefillHp)) {
        auto event = std::make_shared<ReviveEffectEvent>(
            owner.getGameObjectInfo(), owner.getPosition(), currentPoint);
        owner.queryKnowable()->broadcast(event);
        go::Partyable* partyable = owner.queryPartyable();
        if (partyable) {
            PartyPtr party = partyable->getParty();
            if (party.get() != nullptr) {
                party->notifyNearEvent(event, owner.getObjectId(), true);
            }
        }
    }
}


void CreatureEffectController::knockbackRelease()
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    CreatureState* creatureState = owner.queryCreatureState();
    if (! creatureState) {
        return;
    }
    if (creatureState->knockbacked(false)) {
        auto event = std::make_shared<KnockBackReleaseEvent>(
            owner.getGameObjectInfo());
        owner.queryKnowable()->broadcast(event);
    }
}


bool CreatureEffectController::mesmerizationEffected(MesmerizationType mezt, bool isActivate)
{
	go::Creature& owner = getOwnerAs<go::Creature>();
	CreatureState* creatureState = owner.queryCreatureState();
	if (! creatureState) {
		return false;
	}
    switch (mezt) {
    case meztStun:
        return creatureState->stunned(isActivate);
    case meztShackles:
        return creatureState->shackled(isActivate);
    case meztSleep:
        return creatureState->slept(isActivate);
    case meztHide:
        return creatureState->hidden(isActivate);
    case meztFrenzy:
        return creatureState->frenzied(isActivate);
    case meztParalyze:
        return creatureState->paralysed(isActivate);
    case meztSpawnProtection:
        return creatureState->spawnProtection(isActivate);
    case meztDisableMagic:
        return creatureState->magicDisabled(isActivate);
    case meztInvincible:
        return creatureState->invincible(isActivate);
    }
    assert(false);
    return false;
}

}}} // namespace gideon { namespace zoneserver { namespace gc {


