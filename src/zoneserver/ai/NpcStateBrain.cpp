#include "ZoneServerPCH.h"
#include "NpcStateBrain.h"
#include "state/npc/NpcBrainStateFactory.h"
#include "state/BrainStateMachine.h"
#include "state/GlobalBrainState.h"
#include "event/npc/NpcBrainEventFactory.h"
#include "aggro/AggroList.h"
#include "evt/EventTrigger.h"
#include "../model/gameobject/Npc.h"
#include "../model/gameobject/StaticObject.h"
#include "../model/gameobject/status/CreatureStatus.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/ability/Chaoable.h"
#include "../model/gameobject/ability/Factionable.h"
#include "../model/gameobject/ability/Buildable.h"
#include "../model/gameobject/ability/Guildable.h"
#include "../model/gameobject/ability/Invadable.h"
#include "../model/gameobject/ability/WorldEventable.h"
#include "../model/state/CreatureState.h"
#include "../model/state/CombatState.h"
#include "../model/state/MoveState.h"
#include "../controller/callback/EntityStatusCallback.h"
#include "../controller/callback/NpcTalkingCallback.h"
#include "../controller/EntityController.h"
#include "../controller/npc/NpcMoveController.h"
#include "../world/World.h"
#include "../world/WorldMap.h"
#include "../service/skill/NpcSkillList.h"
#include "../service/movement/MovementManager.h"
#include "../service/formation/NpcFormation.h"
#include "../service/guild/GuildService.h"
#include "../service/guild/Guild.h"
#include "../service/world_event/callback/WorldEventMissionCallback.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/NpcTalkingTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace ai {

namespace {

// TODO: read from config
const GameTime watchMovementCheckInterval = 500;
const GameTime watchPlayerWorldEventRegionJoinableCheckInterval = 2000;
const GameTime talkToCheckInterval = 3 * 1000;
const GameTime talkToCooldownTime = 3 * 60 * 1000;


/**
 * @class HandleEventTask
 */
class HandleEventTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<HandleEventTask>
{
public:
    HandleEventTask(NpcStateBrain& brain, BrainEvent& event) :
        brain_(brain),
        event_(event) {}

private:
    virtual void run() {
        brain_.handleEvent(event_);
    }

private:
    NpcStateBrain& brain_;
    BrainEvent& event_;
};


/**
 * @class EnumerateCommandForCallLink
 * 가까이 있고 전투 상태가 아닌 친구에게만 요청한다
 */
class EnumerateCommandForCallLink : public go::Knowable::EnumerateCommand
{
public:
    EnumerateCommandForCallLink(const go::Npc& caller, const go::Entity& target,
        uint8_t aiLinkMaxCount, float32_t aiLinkReactiveDistance) :
        caller_(caller),
        target_(target),
        callerPosition_(caller.getPosition()),
        callerObjectType_(caller.getObjectType()),
        aiLinkMaxCount_(aiLinkMaxCount),
        aiLinkReactiveDistanceSq_(aiLinkReactiveDistance * aiLinkReactiveDistance),
        aiLinkCount_(0) {}

private:
    virtual bool execute(go::Entity& entity) {
        if (entity.isSame(caller_)) {
            return false;
        }

        go::Factionable* callerFactionable = const_cast<go::Npc&>(caller_).queryFactionable();
        if (! callerFactionable->isFriendlyTo(entity)) {
            return false;
        }

        if (entity.getSquaredLength(callerPosition_) > aiLinkReactiveDistanceSq_) {
            return false;
        }

        go::Thinkable* thinkable = entity.queryThinkable();
        if (! thinkable) {
            return false;
        }

        AggroList* aggroList = thinkable->getBrain().getAggroList();
        if (! aggroList) {
            return false;
        }

        if (! aggroList->addThreatByEvent(target_, ai::eventThreated)) {
            return false;
        }

        ++aiLinkCount_;
        return aiLinkCount_ >= aiLinkMaxCount_;
    }

private:
    const go::Npc& caller_;
    const go::Entity& target_;
    const Position callerPosition_;
    const ObjectType callerObjectType_;
    const uint8_t aiLinkMaxCount_;
    const float32_t aiLinkReactiveDistanceSq_;
    uint8_t aiLinkCount_;
};


/**
 * @class EnumerateCommandForWatchMovement
 */
class EnumerateCommandForWatchMovement : public go::Knowable::EnumerateCommand
{
public:
    EnumerateCommandForWatchMovement(NpcStateBrain& brain) :
        brain_(brain) {}

private:
    virtual bool execute(go::Entity& entity) {
        return brain_.perceiveEntity(entity);
    }

private:
    NpcStateBrain& brain_;
};


/**
 * @class EnumerateCommandForWatchPlayerWorldEventRegionJoinable
 */
class EnumerateCommandForWatchPlayerWorldEventRegionJoinable : public go::Knowable::EnumerateCommand
{
public:
    EnumerateCommandForWatchPlayerWorldEventRegionJoinable(NpcStateBrain& brain) :
        brain_(brain) {}

private:
    virtual bool execute(go::Entity& entity) {
        return brain_.perceivePlayerWorldEventRegionJoinable(entity);
    }

private:
    NpcStateBrain& brain_;
};


/**
 * @class EnumerateCommandForTalkTo
 */
class EnumerateCommandForTalkTo : public go::Knowable::EnumerateCommand
{
public:
    EnumerateCommandForTalkTo(NpcStateBrain& brain) :
        brain_(brain) {}

private:
    virtual bool execute(go::Entity& entity) {
        return brain_.talkTo(entity);
    }

private:
    NpcStateBrain& brain_;
};


/// 처리 가능한 상태 목록
const NpcBrainStateId stateIds[] = {
    stateIdle, stateActive, stateAttacking, stateThinking, stateEvading,
    stateResting, stateDialog, stateFleeing, stateSeekingAssist
};

/// 디폴트(entry) 상태
const NpcBrainStateId entryStateId = stateIdle;


/// 처리 가능한 이벤트 목록
const NpcBrainEventId globalEventIds[] = {
    eventActive, eventNothingToDo, eventAttacked, eventTired, eventReturned,
    eventFeared, eventSeekAssist
    // eventAggressive, eventThreated, eventDialogRequested
};

} // namespace {

// = NpcStateBrain

NpcStateBrain::NpcStateBrain(go::Entity& owner, std::unique_ptr<evt::EventTrigger> eventTrigger) :
    StateBrain(owner, std::move(eventTrigger)),
    stateFactory_(std::make_unique<NpcBrainStateFactory>()),
    aggroList_(std::make_unique<AggroList>(owner)),
    npcTemplate_(static_cast<const go::Npc&>(getOwner()).getNpcTemplate()),
    reactiveType_(artUnknown),
    reactiveDistanceSq_(0.0f)
{
    reactiveType_ = npcTemplate_.getReactiveType();
    const float32_t reactiveDistance = npcTemplate_.getReactiveDistance();
    if (reactiveDistance > 0.00001f) {
        reactiveDistanceSq_ = (reactiveDistance * reactiveDistance);
    }
}


NpcStateBrain::~NpcStateBrain()
{
}


void NpcStateBrain::callLink()
{
    go::Npc& owner = static_cast<go::Npc&>(getOwner());

    const go::Entity* target = getAggroList()->selectVictim(owner);
    if (! target) {
        return;
    }

    if (! npcTemplate_.canCallLink()) {
        return;
    }

    const uint8_t aiLinkMaxCount = npcTemplate_.getAiLinkCount();
    if (aiLinkMaxCount <= 0) {
        return;
    }

    EnumerateCommandForCallLink enumerateCommand(owner, *target, aiLinkMaxCount,
        npcTemplate_.getAiLinkReactiveDistance());
    (void)owner.queryKnowable()->enumerate(enumerateCommand);
}


void NpcStateBrain::callAssist(go::Entity& assist)
{
    go::Npc& owner = static_cast<go::Npc&>(getOwner());

    go::Entity* target = getAggroList()->selectVictim(owner);
    if (! target) {
        return;
    }

    go::Liveable* liveable = assist.queryLiveable();
    if ((! liveable) || liveable->getCreatureStatus().isDied()) {
        return;
    }

    CombatState* combatState = assist.queryCombatState();
    if ((! combatState) || combatState->isCombating()) {
        return;
    }

    go::Thinkable* thinkable = assist.queryThinkable();
    if (! thinkable) {
        return;
    }

    AggroList* aggroList = thinkable->getBrain().getAggroList();
    if (! aggroList) {
        return;
    }

    (void)aggroList->addThreatByEvent(*target, ai::eventThreated);
}


bool NpcStateBrain::watchMovement(GameTime diff)
{
    if (! canWatchMovement(diff)) {
        return false;
    }

    EnumerateCommandForWatchMovement enumerateCommand(*this);
    return getOwner().queryKnowable()->enumerate(enumerateCommand);
}


bool NpcStateBrain::watchPlayerWorldEventRegionJoinable(GameTime diff)
{
    if (! canWatchPlayerWorldEventRegionJoinable(diff)) {
        return false;
    }

    EnumerateCommandForWatchPlayerWorldEventRegionJoinable enumerateCommand(*this);
    return getOwner().queryKnowable()->enumeratePlayers(enumerateCommand);
}


bool NpcStateBrain::talkTo(GameTime diff)
{
    if (! canTalkTo(diff)) {
        return false;
    }

    EnumerateCommandForTalkTo enumerateCommand(*this);
    return getOwner().queryKnowable()->enumerate(enumerateCommand);
}


void NpcStateBrain::handleEvent(BrainEvent& event)
{
    sne::core::fsm::State<Brain>* currentState = nullptr;
    sne::core::fsm::State<Brain>* globalState = nullptr;
    {
        std::unique_lock<go::Entity::LockType> lock(getOwner().getLock());

        currentState = getStateMachine().getCurrentState();
        globalState = getStateMachine().getGlobalState();
    }

    if (currentState != nullptr) {
        if (currentState->handleEvent(*this, event)) {
            return;
        }
    }

    if (globalState != nullptr) {
        if (globalState->handleEvent(*this, event)) {
            return;
        }
    }
}


bool NpcStateBrain::perceiveEntity(go::Entity& entity)
{
    if (! entity.isValid()) {
        return false;
    }

    go::Npc& owner = static_cast<go::Npc&>(getOwner());

    if (entity.getGameObjectInfo() == owner.getGameObjectInfo()) {
        return false;
    }

    if (! canInteract(entity.getPosition())) {
        return false;
    }

    if (entity.isCreature()) {
        CreatureLevel npcLevel = npcTemplate_.getLevel();
        CreatureLevel targetLevel = static_cast<go::Creature&>(entity).getCreatureLevel();
        if (npcLevel <= targetLevel && WORLD->getGlobalWorldMap().getAggressiveLevelDiff() < toCreatureLevel(targetLevel - npcLevel)) {
            return false;
        }
    }

    if (! shouldReact(entity)) {
        return false;
    }    

    if (isAggressive(reactiveType_)) {
        CombatState* combatState = owner.queryCombatState();
        assert(combatState != nullptr);
        if (! combatState->canPerceiveEntity()) {
            return false;
        }

        if (! getAggroList()->addThreatByEvent(entity, eventAggressive)) {
            return false;
        }

        gc::EntityStatusCallback* entityStatusCallback =
            entity.getController().queryEntityStatusCallback();
        if (entityStatusCallback != nullptr) {
            entityStatusCallback->entityThreated(owner.getGameObjectInfo());
            owner.getNpcMoveController().headTo(entity);
        }
    }
    else if (gideon::shouldFlee(reactiveType_)) {
        if (owner.queryMoveState()->isRooted()) {
            return false;
        }

        if (! getAggroList()->addThreatByEvent(entity, eventUnknown)) {
            return false;
        }

        const msec_t fleeDuration = esut::random(1000, 4000); // TODO: read from config.
        go::Moveable* moveable = owner.queryMoveable();
        moveable->getMovementManager().setTimedFleeParameter(fleeDuration);

        asyncHandleEvent(eventFeared);
    }

    return true;
}


bool NpcStateBrain::perceivePlayerWorldEventRegionJoinable(go::Entity& entity)
{
    if (! entity.isPlayer()) {
        return false;
    }

    if (entity.getGameObjectInfo() == getOwner().getGameObjectInfo()) {
        return false;
    }

    go::WorldEventable* worldEventable = getOwner().queryWorldEventable();
    if (worldEventable && worldEventable->hasMissionJoinAbility()) {
        WorldEventMissionCallback* callback = worldEventable->getWorldEventMissionCallback();
        if (callback != nullptr) {
            callback->joinMission(getOwner(), entity);
        }
    }        

    return false;
}


bool NpcStateBrain::talkTo(go::Entity& entity)
{
    if (! entity.isValid()) {
        return false;
    }

    gc::NpcTalkingCallback* talkingCallback = entity.getController().queryNpcTalkingCallback();
    if (! talkingCallback) {
        return false;
    }

    if (! canInteract(entity.getPosition())) {
        return false;
    }

    if (! entity.isPlayer()) {
        return false;
    }

    go::Npc& owner = static_cast<go::Npc&>(getOwner());
    const datatable::NpcTalkingList* npcTalkingList = owner.getNpcTalkingList();
    if ((! npcTalkingList) || npcTalkingList->empty()) {
        return false;
    }

    CombatState* combatState = owner.queryCombatState(); assert(combatState != nullptr);
    const bool isCombating = combatState->isCombating();
    if (isCombating) {
        go::Entity* target = aggroList_->getMostHated();
        if ((target != nullptr) && (target->isSame(entity))) {
            return false;
        }
    }

    const int targetLevel = static_cast<go::Creature&>(entity).getCreatureLevel();
    const int thisLevel = static_cast<go::Creature&>(owner).getCreatureLevel();
    go::Factionable* factionable = owner.queryFactionable(); assert(factionable != nullptr);
    go::Chaoable* chaoable = entity.queryChaoable(); assert(chaoable != nullptr);

    const datatable::NpcTalkingTemplate* selectedTalkingTemplate = nullptr;
    for (const datatable::NpcTalkingTemplate* talkingTemplate : *npcTalkingList) {
        // peace_or_combat
        switch (talkingTemplate->getState()) {
        case datatable::ntsCombat:
            if (! isCombating) {
                continue;
            }
            break;
        case datatable::ntsPeace:
            if (isCombating) {
                continue;
            }
            break;
        default:
            assert(false && "invalid peace_or_combat");
            continue;
        }

        // level_diff
        switch (talkingTemplate->getLevelDiff()) {
        case datatable::ntlIgnore:
            break;
        case datatable::ntlLow:
            if (targetLevel <= thisLevel) {
                continue;
            }
            break;
        case datatable::ntlHigh:
            if (targetLevel > thisLevel) {
                continue;
            }
            break;
        default:
            assert(false && "invalid level_diff");
            continue;
        }

        // relation
        switch (talkingTemplate->getRelation()) {
        case datatable::ntrIgnore:
            break;
        case datatable::ntrHostile:
            if (! factionable->isHostileTo(entity)) {
                continue;
            }
            break;
        case datatable::ntrChao:
            if ((! chaoable) || (! chaoable->isChao())) {
                continue;
            }
            break;
        default:
            assert(false && "invalid relation");
            continue;
        }

        // chance
        if (talkingTemplate->getInfo().chance() < uint8_t(esut::random(0, 99))) {
            continue;
        }

        selectedTalkingTemplate = talkingTemplate;
        break;
    }

    if (! selectedTalkingTemplate) {
        return false;
    }

    GameTime& lastTalkToTime = talkToPlayerMap_[entity.getObjectId()];
    if ((lastTalkToTime > 0) && (GAME_TIMER->getElapsedTime(lastTalkToTime) < talkToCooldownTime)) {
        return false;
    }

    lastTalkToTime = GAME_TIMER->msec();
    talkingCallback->talkedFrom(owner.getGameObjectInfo(),
        selectedTalkingTemplate->getInfo().npc_talking_code());
    return true;
}


bool NpcStateBrain::canWatchMovement(GameTime diff) const
{
    if (! npcTemplate_.shouldReactiveByDistance()) {
        return false;
    }

    CreatureState* state = const_cast<go::Entity&>(getOwner()).queryCreatureState();
    if (state && state->isSpawnProtection()) {
        return false;
    }

    nextWatchMovementTime_.update(diff);
    if (! nextWatchMovementTime_.isPassed()) {
        return false;
    }

    nextWatchMovementTime_.reset(watchMovementCheckInterval);
    return true;
}


bool NpcStateBrain::canWatchPlayerWorldEventRegionJoinable(GameTime diff) const
{
    go::WorldEventable* worldEventable = const_cast<go::Entity&>(getOwner()).queryWorldEventable();
    if ((! worldEventable) || (! worldEventable->hasMissionJoinAbility())) {
        return false;
    }

    nextWatchPlayerWorldEventRegionJoinableTime_.update(diff);
    if (! nextWatchPlayerWorldEventRegionJoinableTime_.isPassed()) {
        return false;
    }

    nextWatchPlayerWorldEventRegionJoinableTime_.reset(watchPlayerWorldEventRegionJoinableCheckInterval);
    return true;
}


bool NpcStateBrain::canTalkTo(GameTime diff) const
{
    const go::Npc& owner = static_cast<const go::Npc&>(getOwner());
    const datatable::NpcTalkingList* npcTalkingList = owner.getNpcTalkingList();
    if ((! npcTalkingList) || npcTalkingList->empty()) {
        return false;
    }

    nextTalkToTime_.update(diff);
    if (! nextTalkToTime_.isPassed()) {
        return false;
    }

    nextTalkToTime_.reset(talkToCheckInterval);
    return true;
}


bool NpcStateBrain::canInteract(const ObjectPosition& position) const
{
    const ObjectPosition ownerPosition = getOwner().getPosition();
    const Vector2 ownerHeading = getDirection(ownerPosition.heading_);

    const Vector2 dirTarget = (asVector2(position) - asVector2(ownerPosition));
    const float32_t distanceSq = squaredLength(dirTarget);

    float32_t reactiveDistanceSq = reactiveDistanceSq_;
    if (! isInForward(normalizeTo(dirTarget, sqrtf(distanceSq)), ownerHeading)) {
        reactiveDistanceSq *= 0.3f; // 30%
    }

    if (distanceSq > reactiveDistanceSq) {
        return false;
    }

    assert(npcTemplate_.shouldReactiveByDistance());
    return true;
}


bool NpcStateBrain::shouldReact(go::Entity& entity) const
{
    if (! isProactive(reactiveType_)) {
        return false;
    }

    go::Liveable* liveable = entity.queryLiveable();
    if ((liveable != nullptr) && (liveable->getCreatureStatus().isDied())) {
        return false;
    }

    go::Npc& owner = static_cast<go::Npc&>(const_cast<NpcStateBrain*>(this)->getOwner());

    const GameObjectInfo summonerInfo = owner.querySummonable()->getSummoner();
    if (summonerInfo.isBuilding()) {
        return shouldGuardBuildingAgainst(entity);
    }

    if (entity.queryFactionable() != nullptr) {
        go::Factionable* factionable = owner.queryFactionable();
        if (! factionable->isHostileTo(entity)) {
            // TODO: 카오인 플레이어가 우호 관계라면 공격하지 않는다?
            if (factionable->isFriendlyTo(entity)) {
                return false;
            }

            if (! isAggressiveToChao(reactiveType_)) {
                return false;
            }
            go::Chaoable* chaoable = entity.queryChaoable();
            if ((! chaoable) || (! chaoable->isChao())) {
                return false;
            }
        }
    }

    if (! isAggressive(reactiveType_)) {
        return true;
    }

    if (entity.isCreature()) {
        go::Creature& creature = static_cast<go::Creature&>(entity);
        CreatureState* state = creature.queryCreatureState();
        if (state != nullptr) {
            if ((state->isHidden()) || (state->isSpawnProtection())) {
                return false;
            }
        }

        if (creature.isPlayer()) {
            if (! owner.queryInvadable()->isInvader()) {
                const int targetLevel = creature.getCreatureLevel();
                const int ownerLevel = creature.getCreatureLevel();
                if (! WORLD->isAggressive(ownerLevel, targetLevel)) {
                    return false;
                }
            }
        }
        else if (creature.isNpcOrMonster()) {
            if (isCritter(getNpcType(creature.getEntityCode()))) {
                const MonsterGrade mg = owner.getNpcTemplate().getMonsterGrade();
                if (isEliteMonster(mg) || isBossMonster(mg)) {
                    return false;
                }
            }
        }
    }
    else if (entity.isStaticObject()) {
        // TODO: ability로 대체
        // TODO: Anchor
        //go::StaticObject& so = static_cast<go::StaticObject&>(entity);
        //if ((! so.canDestory()) || so.isMinHp()) {
        //    return false;
        //}

        if (entity.isBuilding()) {
            if (! owner.queryInvadable()->isInvader()) {
                return false;
            }
        }
        else {
            return false;
        }
    }

    return true;
}


bool NpcStateBrain::shouldGuardBuildingAgainst(go::Entity& entity) const
{
    go::Npc& owner = static_cast<go::Npc&>(const_cast<NpcStateBrain*>(this)->getOwner());

    const GameObjectInfo summonerInfo = owner.querySummonable()->getSummoner();
    if (! summonerInfo.isBuilding()) {
        return false;
    }

    go::Guildable* targetGuilable = entity.queryGuildable();
    if (! targetGuilable) {
        return false;
    }

    go::Entity* summoner = owner.queryKnowable()->getEntity(summonerInfo);
    if (! summoner) {
        return false;
    }

    go::Buildable* buildable = summoner->queryBuildable(); assert(buildable != nullptr);

    const GuildId ownerGuildId = buildable->getOwnerGuildId();
    if (! isValidGuildId(ownerGuildId)) {
        return false;
    }
    GuildPtr ownerGuild = GUILD_SERVICE->getGuildByGuildId(ownerGuildId);
    if (! ownerGuild.get()) {
        return false;
    }

    const GuildId targetGuildId = targetGuilable->getGuildId();
    if (! isValidGuildId(targetGuildId)) {
        return false;
    }

    const GuildRelationshipInfo* relationshipInfo = ownerGuild->getRelaytionship(targetGuildId);
    if (! relationshipInfo) {
        return false;
    }

    return relationshipInfo->relationship_ == grtHostility;
}

// = Brain overriding

void NpcStateBrain::initialize()
{
    for (int i = 0; i < _countof(stateIds); ++i) {
        const NpcBrainStateId stateId = stateIds[i];
        BrainState* state = stateFactory_->getBrainState(stateId);
        getStateMachine().registerBrainState(stateId, *state);
    }

    globalState_ = stateFactory_->createGlobalBrainState();
    getStateMachine().setGlobalState(*globalState_);

    for (int i = 0; i < _countof(globalEventIds); ++i) {
        const NpcBrainEventId eventId = globalEventIds[i];
        globalState_->registerBrainEvent(
            *NPC_BRAIN_EVENT_FACTORY->getBrainEvent(eventId));
    }
}


void NpcStateBrain::finalize()
{
}


void NpcStateBrain::attacked(const go::Entity& attacker)
{
    asyncHandleEvent(eventAttacked);

    go::Formable* formable = getOwner().queryFormable(); assert(formable != nullptr);
    NpcFormationRefPtr formation = formable->getFormation();
    if (formation.get() != nullptr) {
        formation->memberAttacked(getOwner(), attacker);
    }

    notifyToSummons(attacker);

    notifyToSummoner(attacker, eventThreated);
}


void NpcStateBrain::died()
{
    aggroList_->clear();

    StateBrain::died();
}


void NpcStateBrain::pathNodeArrived()
{
    StateBrain::pathNodeArrived();

    if (getOwner().isLastPathNode(getOwner().getCurrentPathIndex())) {
        WorldEventMissionCallback* missionCallback = getOwner().queryWorldEventable()->getWorldEventMissionCallback();
        if (missionCallback) {
            //missionCallback->
        }
    }
}


bool NpcStateBrain::canStrikeBack() const
{
    if (! gideon::canStrikeBack(reactiveType_)) {
        return false;
    }

    const go::Npc& owner = static_cast<const go::Npc&>(getOwner());
    if (! owner.getSkillList().hasSkill()) {
        return false;
    }

    return true;
}


bool NpcStateBrain::shouldFlee() const
{
    return gideon::shouldFlee(reactiveType_);
}

// = Stateable overriding

void NpcStateBrain::asyncHandleEvent(int eventId)
{
    const go::Creature& owner = static_cast<const go::Creature&>(getOwner());
    if (owner.getCreatureStatus().isDied()) {
        return;
    }

    BrainEvent* event =
        NPC_BRAIN_EVENT_FACTORY->getBrainEvent(toNpcBrainEventId(eventId));
    if (! event) {
        return;
    }

    TASK_SCHEDULER->schedule(std::make_unique<HandleEventTask>(*this, *event));
}


void NpcStateBrain::handleEvent(int eventId)
{
    const go::Creature& owner = static_cast<const go::Creature&>(getOwner());
    if (owner.getCreatureStatus().isDied()) {
        return;
    }

    BrainEvent* event =
        NPC_BRAIN_EVENT_FACTORY->getBrainEvent(toNpcBrainEventId(eventId));
    if (! event) {
        return;
    }

    handleEvent(*event);
}

bool NpcStateBrain::isInState(int stateId) const
{
    const BrainState* state = const_cast<NpcStateBrain*>(this)->getState(stateId);
    return state == const_cast<NpcStateBrain*>(this)->getStateMachine().getCurrentState();
}


BrainState& NpcStateBrain::getEntryState()
{
    BrainState* state = getState(entryStateId);
    assert(state != nullptr);
    return *state;
}


BrainState* NpcStateBrain::getState(int stateId)
{
    return stateFactory_->getBrainState(toNpcBrainStateId(stateId));
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
