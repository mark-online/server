#include "ZoneServerPCH.h"
#include "Actions.h"
#include <sne/base/utility/Logger.h>
#include "../Brain.h"
#include "../aggro/AggroList.h"
#include "../event/npc/NpcBrainEventDef.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/Device.h"
#include "../../model/gameobject/ability/Thinkable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Formable.h"
#include "../../model/gameobject/ability/Moveable.h"
#include "../../model/state/MoveState.h"
#include "../../controller/EntityController.h"
#include "../../world/WorldMap.h"
#include "../../world/region/SpawnMapRegion.h"
#include "../../service/movement/MovementManager.h"
#include "../../service/teleport/TeleportService.h"
#include "../../service/spawn/SpawnService.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

// = Action

bool Action::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! isValid(static_cast<ActionType>(actionType))) {
        SNE_LOG_ERROR("Action::initialize(%d) - invalid action_type(%d)",
            eventType, actionType);
        return false;
    }

    actionType_ = static_cast<ActionType>(actionType);
    param1_ = param1;
    param2_ = param2;
    param3_ = param3;
    param4_ = param4;
    param5_ = param5;
    return true;
}

// = ActionThreat

bool ActionThreat::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    const Condition condition = getCondition();
    if (! isValid(condition)) {
        SNE_LOG_ERROR("ActionThreat::initialize(%d,%d) - invalid condition(%d)",
            eventType, actionType, condition);
        return false;
    }

    switch (condition) {
    //case atcValue:
    case atcPercent:
        if (std::abs(getValue()) > 100) {
            SNE_LOG_ERROR("ActionThreat::initialize(%d,%d) - invalid value(%d)",
                eventType, actionType, getValue());
            return false;
        }
    }

    const DataCode targetCode = getTargetEntityCode();
    if (! isAllowedEntityCode(targetCode)) {
        SNE_LOG_ERROR("ActionThreat::initialize(%d,%d) - invalid entity_code(%d)",
            eventType, actionType, targetCode);
    }
    return true;
}


void ActionThreat::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* target) const
{
    go::Thinkable* thinkable = owner.queryThinkable();
    if (! thinkable) {
        assert(false && "invalid Event AI setting");
        return;
    }

    AggroList* aggroList = thinkable->getBrain().getAggroList();
    if (! aggroList) {
        return;
    }

    go::Knowable* knowable = owner.queryKnowable();
    if (! knowable) {
        assert(false && "invalid Event AI setting");
    }

    if (target != nullptr) {
        if (target->getEntityCode() != getTargetEntityCode()) {
            target = nullptr;
        }
    }

    if (! target) {
        target = knowable->getNearestEntityBy(getTargetEntityCode());
        if (! target) {
            return;
        }
    }

    const bool ignoreFaction = true;
    aggroList->addThreat(*target, eventThreated, getValue(),
        (getCondition() == atcPercent), ignoreFaction);
}

// = ActionFlee

bool ActionFlee::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    return true;
}


void ActionFlee::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    go::Thinkable* thinkable = owner.queryThinkable();
    if (! thinkable) {
        assert(false && "invalid Event AI setting");
        return;
    }

    MoveState* moveState = owner.queryMoveState();
    assert(moveState != nullptr);
    if (moveState->isRooted()) {
        return;
    }

    go::Moveable* moveable = owner.queryMoveable(); assert(moveable != nullptr);
    moveable->getMovementManager().setTimedFleeParameter(getDuration());

    ai::Stateable* stateable = thinkable->getBrain().queryStateable();
    if (stateable != nullptr) {
        stateable->asyncHandleEvent(eventFeared);
    }
}

// = ActionFleeForAssist

bool ActionFleeForAssist::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    return true;
}


void ActionFleeForAssist::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    go::Thinkable* thinkable = owner.queryThinkable();
    if (! thinkable) {
        assert(false && "invalid Event AI setting");
        return;
    }

    MoveState* moveState = owner.queryMoveState();
    assert(moveState != nullptr);
    if (moveState->isRooted()) {
        return;
    }

    ai::Stateable* stateable = thinkable->getBrain().queryStateable();
    if (stateable != nullptr) {
        stateable->asyncHandleEvent(eventSeekAssist);
    }
}

// = ActionSetPhase

bool ActionSetPhase::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    if (! isValid(getPhase())) {
        SNE_LOG_ERROR("ActionSetPhase::initialize(%d,%d) - invalid event_phase(%d)",
           eventType, actionType, getPhase());
    }

    return true;
}


void ActionSetPhase::process(EntityActionCallback* callback, go::Entity& /*owner*/, go::Entity* /*target*/) const
{
    assert(callback != nullptr);
    if (callback != nullptr) {
        callback->setPhase(getPhase());
    }
}

// = ActionIncreasePhase

bool ActionIncreasePhase::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    if (! isValidIncDec(getIncreasement())) {
        SNE_LOG_ERROR("ActionIncreasePhase::initialize(%d,%d) - invalid increasement(%d)",
            eventType, actionType, getIncreasement());
    }

    if (! isValidIncDec(getDecreasement())) {
        SNE_LOG_ERROR("ActionIncreasePhase::initialize(%d,%d) - invalid decreasement(%d)",
            eventType, actionType, getDecreasement());
    }

    return true;
}


void ActionIncreasePhase::process(EntityActionCallback* callback, go::Entity& /*owner*/, go::Entity* /*target*/) const
{
    assert(callback != nullptr);
    if (callback != nullptr) {
        callback->increasePhase(getIncreasement());
        callback->decreasePhase(getDecreasement());
    }
}

// = ActionRandomPhaseRange

bool ActionRandomPhaseRange::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    if ((getMinPhase() >= evpMax) || (getMaxPhase() >= evpMax)) {
        SNE_LOG_ERROR("ActionRandomPhaseRange::initialize(%d,%d) - invalid min/max_phase(%d,%d)",
            eventType, actionType, getMinPhase(), getMaxPhase());
    }

    if (getMinPhase() > getMaxPhase()) {
        SNE_LOG_ERROR("ActionRandomPhaseRange::initialize(%d,%d) - min_phase(%d) > max_phase(%d)",
            eventType, actionType, getMinPhase(), getMaxPhase());
    }

    return true;
}


void ActionRandomPhaseRange::process(EntityActionCallback* callback, go::Entity& /*owner*/, go::Entity* /*target*/) const
{
    assert(callback != nullptr);
    if (callback != nullptr) {
        const EventPhase phase = EventPhase(esut::random(getMinPhase(), getMaxPhase()));
        callback->setPhase(phase);
    }
}

// = ActionPlayAction

bool ActionPlayAction::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    return true;
}


void ActionPlayAction::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    owner.playAction(getActionCode());
}

// = ActionSummonNpc

bool ActionSummonNpc::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    if (! isNpcType(getCodeType(getNpcCode()))) {
        SNE_LOG_ERROR("ActionSummonNpc::initialize(%d,%d) - invalid npc_code(%d)",
            eventType, actionType, getNpcCode());
        return false;            
    }

    if (! isValid(getSpawnType())) {
        SNE_LOG_ERROR("ActionSummonNpc::initialize(%d,%d) - invalid spawn_type(%d)",
            eventType, actionType, getSpawnType());
        return false;            
    }

    return true;
}


void ActionSummonNpc::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    SpawnMapRegion& globalSpawnMapRegion =
        static_cast<SpawnMapRegion&>(worldMap->getGlobalMapRegion());
    (void)globalSpawnMapRegion.summonNpc(
        owner, getNpcCode(), getSpawnDelay(), getLifeTime(), getSpawnType(), getKeepAwayDistance());
}

// = ActionFormUp

bool ActionFormUp::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    return true;
}


void ActionFormUp::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    go::Formable* formable = owner.queryFormable();
    if (! formable) {
        assert(false);
        return;
    }

    if (formable->hasForm()) {
        formable->formUp();
    }
}

// = ActionTransform

bool ActionTransform::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    const Condition condition = getCondition();
    if (! isValid(condition)) {
        SNE_LOG_ERROR("ActionTransform::initialize(%d,%d) - invalid condition(%d)",
            eventType, actionType, condition);
        return false;
    }

    switch (condition) {
    case atcNpc:
    case atcMonster:
        if (! isValidNpcCode(getNpcCode())) {
            SNE_LOG_ERROR("ActionTransform::initialize(%d,%d) - invalid npc_code(%d)",
                eventType, actionType, getNpcCode());
            return false;
        }
    }
    return true;
}


void ActionTransform::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    go::Knowable* knowable = owner.queryKnowable();
    if (! knowable) {
        assert(false && "ActionTransform::process() - owner is not knowable");
        return;
    }

    go::Entity* target = nullptr;
    switch (getCondition()) {
    case atcNpc:
        target = knowable->getNearestNpc();
        break;
    case atcMonster:
        target = knowable->getNearestMonster();
        break;
    default:
        assert(false);
    }
    if (! target) {
        return;
    }

    const datatable::NpcTemplate& npcTemplate = static_cast<go::Npc&>(owner).getNpcTemplate();

    if (owner.getLength(target->getPosition()) > npcTemplate.getReactiveDistance()) {
        return;
    }

    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        return;
    }

    const msec_t despawnDelay = 0;
    (void)SPAWN_SERVICE->scheduleDespawn(*target, despawnDelay);

    const msec_t spawnDelay = 0;
    SpawnMapRegion& spawnMapRegion = static_cast<SpawnMapRegion&>(target->getMapRegion());
    (void)spawnMapRegion.summonNpc(*target, getNpcCode(), spawnDelay, getDuration(),
        sptTransform, 0.0f);
}

// = ActionTeleport

bool ActionTeleport::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    const DataCode destinationCode = getDestinationCode();
    if ((! isValidNpcCode(destinationCode)) && (! isValidRegionCode(destinationCode))) {
        SNE_LOG_ERROR("ActionTeleport::initialize(%d,%d) - invalid destination_code(%d)",
            eventType, actionType, destinationCode);
        return false;
    }

    return true;
}


void ActionTeleport::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    WorldPosition destination = owner.getWorldPosition();
    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    const DataCode destinationCode = getDestinationCode();
    if (isValidNpcCode(destinationCode)) {
        const datatable::PositionSpawnTable* positionSpawnTable =
            worldMap->getPositionSpawnTable();
        if (! positionSpawnTable) {
            assert(false);
            return;
        }
        const PositionSpawnTemplate* spawnTemplate = positionSpawnTable->getPositionSpawnTemplate(
            owner.getMapRegion().getRegionCode(), destinationCode);
        if (! spawnTemplate) {
            assert(false);
            return;
        }
        const Vector2 direction = getDirection(spawnTemplate->position_.heading_);

        static_cast<ObjectPosition&>(destination) = spawnTemplate->position_;
        destination.x_ += (direction.x * 2.0f);
        destination.y_ += (direction.y * 2.0f);
        destination.heading_ = fixHeading((int)spawnTemplate->position_.heading_ + toHeading(180));
    }
    else if (isValidRegionCode(destinationCode)) {
        const MapRegion* mapRegion = worldMap->getMapRegion(destinationCode);
        if (! mapRegion) {
            assert(false);
            return;
        }
        static_cast<ObjectPosition&>(destination) = mapRegion->getRandomPosition();
    }

    (void)TELEPORT_SERVICE->teleportPosition(destination, owner);
}

// = ActionMoveThruPath

bool ActionMoveThruPath::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    //const EntityPathCode pathCode = getEntityPathCode();
    //if (! isValidEntityPathCode(pathCode)) {
    //    SNE_LOG_ERROR("ActionMoveThruPath::initialize(%d,%d) - invalid entity_path_code(%d)",
    //        eventType, actionType, pathCode);
    //    return false;
    //}

    return true;
}


void ActionMoveThruPath::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    WorldMap* worldMap = owner.getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    go::Moveable* moveable = owner.queryMoveable();
    if (! moveable) {
        SNE_LOG_ERROR("ActionMoveThruPath::process() - owner cannot move");
        assert(false);
        return;
    }

    const gdt::entity_path_t* entityPath = nullptr;
    const EntityPathCode pathCode = getEntityPathCode();
    if (isValidEntityPathCode(pathCode)) {
        entityPath = worldMap->getEntityPath(pathCode);
    }
    else {
        entityPath = worldMap->getNearestEntityPath(owner);
    }
    if (! entityPath) {
        SNE_LOG_ERROR("ActionMoveThruPath::process() - invalid entity_path_code(%d)");
        assert(false);
        return;
    }
    owner.setPath(entityPath);

    assert(owner.hasPath());
    moveable->getMovementManager().setPathMovement();
}

// = ActionActivateDevice

bool ActionActivateDevice::initialize(EventType eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (! Action::initialize(eventType, actionType, param1, param2, param3, param4, param5)) {
        return false;
    }

    if (! isValidDeviceCode(getDeviceCode())) {
        SNE_LOG_ERROR("ActionTeleport::initialize(%d,%d) - invalid device_code(%d)",
            eventType, actionType, getDeviceCode());
        return false;
    }

    return true;
}


void ActionActivateDevice::process(EntityActionCallback* /*callback*/, go::Entity& owner, go::Entity* /*target*/) const
{
    if (! owner.isCreature()) {
        assert(false);
        return;
    }

    MapRegion& mapRegion = owner.getMapRegion();

    go::Device* device =
        static_cast<go::Device*>(mapRegion.getFirstEntity(otDevice, getDeviceCode()));
    if (! device) {
        assert(false);
        return;
    }

    //ActionActivateDevice* noContThis = const_cast<ActionActivateDevice*>(this);
    (void)device->activate(static_cast<go::Creature&>(owner));
}

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
