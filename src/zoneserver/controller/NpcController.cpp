#include "ZoneServerPCH.h"
#include "NpcController.h"
#include "npc/NpcSkillController.h"
#include "../model/gameobject/Npc.h"
#include "../model/gameobject/GraveStone.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/status/CreatureStatus.h"
#include "../model/gameobject/skilleffect/EffectScriptApplier.h"
#include "../model/gameobject/ability/Questable.h"
#include "../model/gameobject/ability/Partyable.h"
#include "../model/gameobject/ability/Formable.h"
#include "../model/state/CreatureState.h"
#include "../ai/Brain.h"
#include "../ai/event/npc/NpcBrainEventDef.h"
#include "../world/WorldMap.h"
#include "../service/party/Party.h"
#include "../service/spawn/SpawnService.h"
#include "../service/item/GraveStoneService.h"
#include "../service/skill/SkillService.h"
#include "../service/skill/NpcSkillList.h"
#include "../service/world_event/callback/WorldEventMissionCallback.h"
#include "callback/EntityStatusCallback.h"
#include "callback/RewardCallback.h"
#include <gideon/cs/datatable/NpcTable.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class NpcDiedEvent
 */
class NpcDiedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<NpcDiedEvent>
{
public:
    NpcDiedEvent(const GameObjectInfo& npcInfo, const GameObjectInfo& killerInfo, go::GraveStone* graveStone) :
        npcInfo_(npcInfo),
        killerInfo_(killerInfo),
        graveStone_(graveStone) {}

private:
    virtual void call(go::Entity& entity) {
        EntityStatusCallback* entityStatusCallback =
            entity.getController().queryEntityStatusCallback();
        if (entityStatusCallback != nullptr) {
            entityStatusCallback->npcDied(npcInfo_, killerInfo_, graveStone_);
        }
    }

private:
    const GameObjectInfo npcInfo_;
    const GameObjectInfo killerInfo_;
    go::GraveStone* graveStone_;
};

} // namespace {

// = NpcController

NpcController::NpcController() :
    isDieFromInvader_(false)
{

}

// = EntityController overriding

void NpcController::spawned(WorldMap& worldMap)
{
    Parent::spawned(worldMap);

    go::Npc& owner = getOwnerAs<go::Npc>();

    ai::Brain& brain = owner.queryThinkable()->getBrain();
    brain.activate();

    ai::Stateable* stateable = brain.queryStateable();
    if (stateable != nullptr) {
        if (owner.shouldActiveBrain()) {
            stateable->asyncHandleEvent(ai::eventActive);
        }
    }

    owner.reserveDespawn();

    go::Formable* formable = owner.queryFormable();
    if ((formable != nullptr) && formable->hasForm()) {
        formable->formUp();
    }
}


void NpcController::despawned(WorldMap& worldMap)
{
    Parent::despawned(worldMap);

    if (! isDieFromInvader_) {
        getOwner().reserveRespawn(worldMap);
        isDieFromInvader_ = false;
    }
}

// = CreatureController overriding

void NpcController::died(go::Entity* from)
{
    go::Npc& owner = getOwnerAs<go::Npc>();

    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        return;
    }

    {
        std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());
        const CreatureState* state = owner.queryCreatureState();
        if ((! state) || state->isDied()) {
            return;
        }

        getOwner().queryCreatureState()->died();
    }
    

    go::Entity* topScorePlayer = owner.getTopScorePlayer();
    if (topScorePlayer) {
        assert(topScorePlayer->isPlayer());
        processQuest(*topScorePlayer);
        rewardExp(*topScorePlayer);
    }

    owner.queryThinkable()->getBrain().died();
    owner.getEffectScriptApplier().revert();
    
    go::GraveStone* graveStone = GRAVE_STONE_SERVICE->registerGraveStone(owner, *worldMap, from);
    GameObjectInfo killerInfo = from ? from->getGameObjectInfo() : GameObjectInfo();
    auto event = std::make_shared<NpcDiedEvent>(owner.getGameObjectInfo(), killerInfo, graveStone);
    owner.queryKnowable()->broadcast(event);
    
    processWorldEvent(topScorePlayer);
      
    Parent::died(from);

    bool shouldRevive = true;
    if (owner.isNpc() && (from != nullptr)) {
        go::WorldEventable* worldEventable = from->queryWorldEventable();
        if (worldEventable != nullptr) {
            WorldEventMissionCallback* callback = worldEventable->getWorldEventMissionCallback();
            if (callback != nullptr) {
                shouldRevive = false;
                isDieFromInvader_ = true;
                const ErrorCode errorCode = getOwner().despawn();
                if (isFailed(errorCode)) {
                    assert(false);
                    SNE_LOG_ERROR("Failed to despawn NPC(%d,%" PRIu64 ")[E%d]",
                        getOwner().getObjectType(), getOwner().getObjectId(), errorCode);
                    return;
                }
                callback->reserveRespawn(owner);
            }
        }
    }

    if (shouldRevive) {
        const ErrorCode errorCode = owner.queryLiveable()->revive();
        if (isFailed(errorCode)) {
            SNE_LOG_ERROR("Failed to revive NPC(%d,%" PRIu64 ")[E%d]",
                getOwner().getObjectType(), getOwner().getObjectId(), errorCode);
            return;
        }
    }
}

// = InterestAreaCallback overriding

void NpcController::entitiesAppeared(const go::EntityMap& entities)
{
    Parent::entitiesAppeared(entities);
}


void NpcController::entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo)
{
    Parent::entityAppeared(entity, entityInfo);

    CreatureState* state = entity.queryCreatureState();
    if (state) {
        if (state->isHidden()) {
            return;
        }
    }
}


void NpcController::entitiesDisappeared(const go::EntityMap& entities)
{
    Parent::entitiesDisappeared(entities);
}


void NpcController::entityDisappeared(go::Entity& entity)
{
    Parent::entityDisappeared(entity);

    go::Npc& owner = getOwnerAs<go::Npc>();
    go::Dialogable* dialogable = owner.queryDialogable();
    if (dialogable != nullptr) {
        dialogable->closeDialog(entity.getGameObjectInfo());
    }
}


void NpcController::entityDisappeared(const GameObjectInfo& info)
{
    Parent::entityDisappeared(info);

    go::Npc& owner = getOwnerAs<go::Npc>();

    go::Dialogable* dialogable = owner.queryDialogable();
    if (dialogable != nullptr) {
        dialogable->closeDialog(info);
    }
}


void NpcController::processQuest(go::Entity& killEntity)
{
    if (! killEntity.isValid()) {
        return;
    }
    go::Partyable* partyable = killEntity.queryPartyable();
    if (! partyable) {
        return;
    }

    PartyPtr party = partyable->getParty();
    if (party.get() != nullptr) {
        party->killed(getOwner());
    }
    else {
        go::Questable* questable = killEntity.queryQuestable();
        if (questable != nullptr) {
            questable->killed(getOwner());
        }
    }
}


void NpcController::rewardExp(go::Entity& from)
{
    if (! from.isValid() || ! from.isPlayer()) {
        return;
    }

    CreatureLevel fromLevel = static_cast<go::Creature&>(from).getCreatureLevel();
    go::Npc& owner = getOwnerAs<go::Npc>();
    const ExpPoint rewardBaseExp = owner.getRewardExp(fromLevel);
    bool shouldReward = true;
    go::Partyable* partyable = from.queryPartyable();
    if (partyable) {
        PartyPtr party = partyable->getParty();
        if (party.get() != nullptr) {
            party->rewardExp(rewardBaseExp, owner.getCurrentWorldMap()->getMapCode(), owner.getPosition());
            shouldReward = false;
        }
    }

    if (shouldReward) {
        RewardCallback* rewardCallbck = from.getController().queryRewardCallback();
        if (rewardCallbck) {
            rewardCallbck->expRewarded(rewardBaseExp);
        }
    }
}


void NpcController::processWorldEvent(go::Entity* killEntity)
{
    WorldEventMissionCallback* worldEventMissionCallback = 
        getOwner().queryWorldEventable()->getWorldEventMissionCallback();
    if (! worldEventMissionCallback) {
        return;
    }
    go::Creature& owner = getOwnerAs<go::Creature>();
    worldEventMissionCallback->killed(killEntity, owner.getCreatureLevel(), getOwner().getGameObjectInfo());    
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
