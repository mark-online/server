#include "ZoneServerPCH.h"
#include "Events.h"
#include "../../model/gameobject/ability/Factionable.h"
#include "../../model/gameobject/ability/Liveable.h"
#include "../../model/gameobject/status/CreatureStatus.h"
#include "../../model/state/CombatState.h"
#include "../../model/state/CreatureState.h"
#include "../../world/WorldMap.h"
#include <gideon/cs/datatable/template/event_trigger_table.hxx>
#include <gideon/cs/datatable/template/entity_path_table.hxx>
#include <gideon/3d/3d.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

// = Event

bool Event::initialize(const gdt::evt_t& eventAiTemplate)
{
    const DataCode entityCode = eventAiTemplate.entity_code();
    if (! isAllowedEntityCode(entityCode)) {
        return false;
    }

    eventType_ = toEventType(eventAiTemplate.event_type());
    entityCode_ = entityCode;
    eventFlags_ = toEventFlags(eventAiTemplate.event_flags());
    eventPhaseMask_ = EventPhaseBits(eventAiTemplate.event_phase_mask());
    eventChance_ = eventAiTemplate.event_chance();
    param1_ = int(eventAiTemplate.event_param1());
    param2_ = int(eventAiTemplate.event_param2());
    param3_ = int(eventAiTemplate.event_param3());
    param4_ = int(eventAiTemplate.event_param4());

    if (! isValid(eventPhaseMask_)) {
        SNE_LOG_ERROR("Event::initialize(%d) - invalid event_phase_mask(%d)",
            eventType_, eventPhaseMask_);
        return false;
    }

    if (eventChance_ <= 0) {
        SNE_LOG_ERROR("Event::initialize(%d) - invalid event_chance(%d)",
            eventType_, eventChance_);
        return false;
    }
    else if (eventChance_ > 100) {
        SNE_LOG_WARNING("Event::initialize(%d) - too big event_chance(%d)",
            eventType_, eventChance_);
        return false;
    }

    return true;
}


void Event::setAction(std::unique_ptr<Action> action, int index)
{
    actions_[index] = std::move(action);
}


void Event::processActions(go::Entity& owner, EntityActionCallback* callback, go::Entity* target) const
{
    for (auto& action : actions_) {
        if (action != nullptr) {
            action->process(callback, owner, target);
        }
    }
}

// = EventTimerInCombat

bool EventTimerInCombat::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    if (getInitialMin() > getInitialMax()) {
        SNE_LOG_ERROR("EventTimerInCombat::initialize(%d) - initial_min(%d) > initial_max(%d)",
            getEventType(), getInitialMin(), getInitialMax());
        return false;
    }

    if (getRepeatMin() > getRepeatMax()) {
        SNE_LOG_ERROR("EventTimerInCombat::initialize(%d) - repeat_min(%d) > repeat_max(%d)",
            getEventType(), getRepeatMin(), getRepeatMax());
        return false;
    }

    return true;
}


bool EventTimerInCombat::process(go::Entity& owner, go::Entity* /*target*/) const
{
    CombatState* combatState = owner.queryCombatState();
    if ((! combatState) || (! combatState->isCombating())) {
        return false;
    }
    return true;
}


bool EventTimerInCombat::updateRepeatTimer(EventCallback& callback) const
{
    return callback.updateRepeatTimer(getRepeatMin(), getRepeatMax());
}

// = EventTimerInPeace

bool EventTimerInPeace::process(go::Entity& owner, go::Entity* /*target*/) const
{
    CombatState* combatState = owner.queryCombatState();
    if ((combatState != nullptr) && combatState->isCombating()) {
        return false;
    }
    return true;
}

// = EventLosInPeace

bool EventLosInPeace::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    if (getRepeatMin() > getRepeatMax()) {
        SNE_LOG_ERROR("EventLosInPeace::initialize(%d) - repeat_min(%d) > repeat_max(%d)",
            getEventType(), getRepeatMin(), getRepeatMax());
        return false;
    }

    return true;
}


bool EventLosInPeace::process(go::Entity& owner, go::Entity* target) const
{
    if (! target) {
        assert(false);
        return false;
    }
    if (! target->isValid()) {
        return false;
    }

    if (owner.isSame(*target)) {
        return false;
    }

    CombatState* combatState = owner.queryCombatState();
    if ((combatState != nullptr) && combatState->isCombating()) {
        return false;
    }

    const float32_t distanceSq = owner.getSquaredLength(target->getPosition());
    if (distanceSq > square(static_cast<float32_t>(getMaxDistance()))) {
        return false;
    }

    go::Factionable* ownerFactionable = owner.queryFactionable();
    if (! ownerFactionable) {
        assert(false);
        return false;
    }

    if (isHostileOnly()) {
        if (! ownerFactionable->isHostileTo(*target)) {
            return false;
        }
    }
    else {
        if (ownerFactionable->isHostileTo(*target)) {
            return false;
        }
    }

    if (owner.isCreature()) {
        CreatureState* state = owner.queryCreatureState();
        if (state != nullptr) {
            if (state->isHidden()) {
                return false;
            }
        }
    }

    return true;
}


bool EventLosInPeace::updateRepeatTimer(EventCallback& callback) const
{
    return callback.updateRepeatTimer(getRepeatMin(), getRepeatMax());
}

// = EventHp

bool EventHp::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    if (getMaxHpPct() > 100) {
        SNE_LOG_WARNING("EventHp::initialize(%d) - too big max_hp_pct(%d)",
            getEventType(), getMaxHpPct());
        return false;
    }

    if (getMaxHpPct() <= getMinHpPct()) {
        SNE_LOG_WARNING("EventHp::initialize(%d) - max_hp_pct(%d) <= min_hp_pct(%d)",
            getEventType(), getMaxHpPct(), getMinHpPct());
        return false;
    }

    if (isRepeatable()) {
        if (getRepeatMin() > getRepeatMax()) {
            SNE_LOG_ERROR("EventHp::initialize(%d) - repeat_min(%d) > repeat_max(%d)",
                getEventType(), getRepeatMin(), getRepeatMax());
            return false;
        }
    }

    return true;
}


bool EventHp::process(go::Entity& owner, go::Entity* /*target*/) const
{
    CombatState* combatState = owner.queryCombatState();
    if ((combatState != nullptr) && (! combatState->isCombating())) {
        return false;
    }

    // TODO: static object도 지원하게끔 수정
    go::Liveable* liveable = owner.queryLiveable();
    if (! liveable) {
        return false;
    }
    const uint8_t hpRatio = liveable->getCreatureStatus().getHitPointRatio();
    if ((hpRatio > getMaxHpPct()) || (hpRatio < getMinHpPct())) {
        return false;
    }

    return true;
}


bool EventHp::updateRepeatTimer(EventCallback& callback) const
{
    return callback.updateRepeatTimer(getRepeatMin(), getRepeatMax());
}

// = EventSpawned

bool EventSpawned::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    const Condition condition = getCondition();
    if (! isValid(condition)) {
        SNE_LOG_ERROR("EventSpawned::initialize(%d) - Invalid condition(%d)",
            getEventType(), condition);
        return false;
    }

    switch (condition) {
    case escMap:
        if (! isValidMapCode(getMapCode())) {
            return false;
        }
    }

    if (! gideon::isValid(getSpawnType())) {
        SNE_LOG_ERROR("EventSpawned::initialize(%d) - Invalid spawn_type(%d)",
            getEventType(), getSpawnType());
        return false;
    }

    return true;
}


bool EventSpawned::process(go::Entity& owner, go::Entity* /*target*/) const
{
    switch (getCondition()) {
    case escAlways:
        break;
    case escMap:
        {
            const WorldMap* worldMap = owner.getCurrentWorldMap();
            if ((! worldMap) || (worldMap->getMapCode() != getMapCode())) {
                return false;
            }
        }
        break;
    default:
        return false;
    }

    if (owner.getSpawnTemplate().spawnType_ != getSpawnType()) {
        return false;
    }

    return true;
}

// = EventDespawned

bool EventDespawned::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    return true;
}


bool EventDespawned::process(go::Entity& /*owner*/, go::Entity* /*target*/) const
{
    return true;
}

// = EventDied

bool EventDied::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    return true;
}


bool EventDied::process(go::Entity& /*owner*/, go::Entity* /*target*/) const
{
    return true;
}

// = EventWanderPaused

bool EventWanderPaused::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    //if (isRepeatable()) {
    //    if (getRepeatMin() > getRepeatMax()) {
    //        SNE_LOG_ERROR("EventWanderPaused::initialize(%d) - repeat_min(%d) > repeat_max(%d)",
    //            getEventType(), getRepeatMin(), getRepeatMax());
    //        return false;
    //    }
    //}

    return true;
}


bool EventWanderPaused::process(go::Entity& owner, go::Entity* /*target*/) const
{
    CombatState* combatState = owner.queryCombatState();
    if ((combatState != nullptr) && combatState->isCombating()) {
        return false;
    }
    return true;
}

// = EventPathNodeArrived

bool EventPathNodeArrived::initialize(const gdt::evt_t& eventAiTemplate)
{
    if (! Event::initialize(eventAiTemplate)) {
        return false;
    }

    if (! isValid(getCondition())) {
        SNE_LOG_ERROR("EventPathNodeArrived::initialize(%d) - Invalid condition(%d)",
            getEventType(), getCondition());
        return false;
    }

    //if (isRepeatable()) {
    //    if (getRepeatMin() > getRepeatMax()) {
    //        SNE_LOG_ERROR("EventWanderPaused::initialize(%d) - repeat_min(%d) > repeat_max(%d)",
    //            getEventType(), getRepeatMin(), getRepeatMax());
    //        return false;
    //    }
    //}

    return true;
}


bool EventPathNodeArrived::process(go::Entity& owner, go::Entity* /*target*/) const
{
    const gdt::entity_path_t* entityPath = owner.getPath();
    if (! entityPath) {
        assert(false && "fix event_trigger_table!");
        return false;
    }
    if (getEntityPathCode() != entityPath->entity_path_code()) {
        return false;
    }
    if (getNodeIndex() != owner.getCurrentPathIndex()) {
        return false;
    }

    const Condition condition = getCondition();
    if (condition == epnaForward) {
        if (! owner.isPathForward()) {
            return false;
        }
    }
    else if (condition == epnaBackward) {
        if (owner.isPathForward()) {
            return false;
        }
    }

    CombatState* combatState = owner.queryCombatState();
    if ((combatState != nullptr) && combatState->isCombating()) {
        return false;
    }

    return true;
}

// = EventDeviceActivated

bool EventDeviceActivated::process(go::Entity& owner, go::Entity* /*target*/) const
{
    if (owner.getObjectType() != otDevice) {
        assert(false);
        return false;
    }
    return true;
}

// = EventItemUsed

bool EventItemUsed::process(go::Entity& owner, go::Entity* /*target*/) const
{
    if (owner.getObjectType() != otPc) {
        assert(false);
        return false;
    }
    return true;
}

// = EventActivationMissionCompleted

bool EventActivationMissionCompleted::process(go::Entity& owner, go::Entity* /*target*/) const
{
    if (owner.getObjectType() != otPc) {
        assert(false);
        return false;
    }
    return true;
}

// = EventKillMissionCompleted

bool EventKillMissionCompleted::process(go::Entity& owner, go::Entity* /*target*/) const
{
    if (owner.getObjectType() != otPc) {
        assert(false);
        return false;
    }
    return true;
}

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
