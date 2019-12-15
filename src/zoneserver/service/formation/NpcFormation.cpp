#include "ZoneServerPCH.h"
#include "NpcFormation.h"
#include "../../model/gameobject/ability/Liveable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Thinkable.h"
#include "../../model/gameobject/ability/Moveable.h"
#include "../../model/gameobject/ability/Marchable.h"
#include "../../model/gameobject/ability/WorldEventable.h"
#include "../../model/gameobject/status/CreatureStatus.h"
#include "../../model/state/CombatState.h"
#include "../../model/state/CreatureState.h"
#include "../../ai/Brain.h"
#include "../../ai/aggro/AggroList.h"
#include "../../ai/event/npc/NpcBrainEventDef.h"
#include "../../world/region/SpawnMapRegion.h"
#include "../../world/WorldMap.h"
#include "../../service/movement/MovementManager.h"
#include "../spawn/SpawnService.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/NpcFormationTable.h>

namespace gideon { namespace zoneserver {

bool NpcFormation::hasForm(const go::Entity& entity)
{
    return NPC_FORMATION_TABLE->getNpcFormation(entity.getEntityCode()) != nullptr;
}


bool NpcFormation::formUp(NpcFormationRefPtr thisFormation)
{
    std::lock_guard<LockType> lock(lock_);

    WorldMap* worldMap = leader_.getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return false;
    }
    SpawnMapRegion& globalSpawnMapRegion =
        static_cast<SpawnMapRegion&>(worldMap->getGlobalMapRegion());

    const datatable::NpcFormationTable::Members* members =
        NPC_FORMATION_TABLE->getNpcFormation(leader_.getEntityCode());
    if (! members) {
        assert(false && "fix tables!");
        return false;
    }

    for (const gdt::npc_formation_t* formationInfo : *members) {
        const NpcCode memberNpcCode = formationInfo->member_npc_code();
        assert(isValidNpcCode(memberNpcCode) && "fix table!");
        if (memberNpcCode == formationInfo->leader_npc_code()) {
            members_[&leader_] = formationInfo;
        }
        else {
            const float32_t maxDistance = formationInfo->max_distance() / 100.0f; // cm -> m
            go::Entity* member = nullptr;
            const ErrorCode errorCode =
                globalSpawnMapRegion.summonFormationMember(member, memberNpcCode, leader_, maxDistance);
            if (isSucceeded(errorCode)) {
                member->queryFormable()->setFormation(thisFormation);
                members_[member] = formationInfo;
                go::WorldEventable* worldEventable = leader_.queryWorldEventable();
                if (worldEventable) {
                    WorldEventMissionCallback* callback = worldEventable->getWorldEventMissionCallback();
                    if (callback) {
                        member->queryWorldEventable()->setWorldEventMissionCallback(*callback, false, 0.0f);
                    }
                }
            }
            else {
                // TODO: 어떻게 해야 하나?
            }
        }
    }

    // TODO: member들의 위치 보정???

    return true;
}


void NpcFormation::dismiss()
{
    Members members; 
    {
        std::lock_guard<LockType> lock(lock_);

        members = members_;
        members_.clear();
    }

    for (Members::value_type& value : members) {
        go::Entity* member = value.first;
        if (isLeader(*member)) {
            continue;
        }

        if (! isMemberAlive(*member)) {
            continue;
        }

        if (! member->queryCreatureState()->isDied()) {
            member->queryCreatureState()->died();
        }

        (void)SPAWN_SERVICE->scheduleDespawn(*member, 0);
    }
}


void NpcFormation::leave(go::Entity& member)
{
    std::lock_guard<LockType> lock(lock_);

    members_.erase(&member);
}


void NpcFormation::leaderMoved(const Position& position)
{
    const ObjectPosition leaderPosition = leader_.getPosition();
    const float32_t pathAngle = getYaw(position, leaderPosition);

    go::Marchable* leaderMarchable = leader_.queryMarchable(); assert(leaderMarchable != nullptr);
    const bool isMarching = leaderMarchable->isMarching();

    {
        std::lock_guard<LockType> lock(lock_);

        for (Members::value_type& value : members_) {
            const gdt::npc_formation_t* formationInfo = value.second;
            go::Entity* member = value.first;
            if (isLeader(*member)) {
                continue;
            }
            if (!isMemberAlive(*member)) {
                continue;
            }

            const float32_t moveAngle = pathAngle + toRadians(formationInfo->angle());
            const Heading moveHeading = toHeading(toDegrees(moveAngle));
            const Vector2 direction = getDirection(moveHeading);
            const float32_t maxDistance = formationInfo->max_distance() / 100.0f; // cm -> m

            ObjectPosition destination(position, moveHeading);
            destination.x_ += direction.x * maxDistance;
            destination.y_ += direction.y * maxDistance;
            //destination.z_ = ;

            member->setHomePosition(destination);

            if (member->getSquaredLength(destination) < square(maxDistance)) {
                continue;
            }

            CombatState* combatState = member->queryCombatState();
            if ((!combatState) || (!combatState->isCombating())) {
                if (isMarching) {
                    go::Marchable* marchable = member->queryMarchable(); assert(marchable != nullptr);
                    marchable->marchStarted();
                }
                member->queryMoveable()->getMovementManager().setPointMovement(destination);
            }
        }
    }
}


void NpcFormation::memberAttacked(const go::Entity& actor, const go::Entity& target)
{
    std::lock_guard<LockType> lock(lock_);

    const gdt::npc_formation_t* memberFormationInfo = getFormationInfo(actor);
    if (! memberFormationInfo) {
        return;
    }

    const ReactType reactType = ReactType(memberFormationInfo->react_type());
    if (reactType == rtNothing) {
        return;
    }

    if ((reactType == rtLeaderOnly) && (! isLeader(actor))) {
        return;
    }

    for (Members::value_type& value : members_) {
        go::Entity* member = value.first;
        if (member->isSame(actor)) {
            continue;
        }
        if (! isMemberAlive(*member)) {
            continue;
        }
        CombatState* combatState = member->queryCombatState();
        if ((combatState != nullptr) && combatState->isCombating()) {
            continue;
        }

        go::Thinkable* thinkable = member->queryThinkable(); assert(thinkable != nullptr);
        ai::AggroList* aggroList = thinkable->getBrain().getAggroList(); assert(aggroList != nullptr);
        (void)aggroList->addThreatByEvent(target, ai::eventThreated);
    }
}


void NpcFormation::memberTiredToAttack(const go::Entity& actor)
{
    std::lock_guard<LockType> lock(lock_);

    const gdt::npc_formation_t* memberFormationInfo = getFormationInfo(actor);
    if (! memberFormationInfo) {
        return;
    }

    const ReactType reactType = ReactType(memberFormationInfo->react_type());
    if (reactType == rtNothing) {
        return;
    }

    if ((reactType == rtLeaderOnly) && (! isLeader(actor))) {
        return;
    }

    for (Members::value_type& value : members_) {
        go::Entity* member = value.first;
        if (member->isSame(actor)) {
            continue;
        }
        if (! isMemberAlive(*member)) {
            continue;
        }
        CombatState* combatState = member->queryCombatState();
        if (! combatState) {
            continue;
        }
        if (! combatState->isCombating()) {
            continue;
        }

        go::Thinkable* thinkable = member->queryThinkable(); assert(thinkable != nullptr);
        ai::Stateable* stateable = thinkable->getBrain().queryStateable();
        if (stateable != nullptr) {
            stateable->asyncHandleEvent(ai::eventTired);
        }
    }
}


void NpcFormation::marchStarted()
{
    std::lock_guard<LockType> lock(lock_);

    for (Members::value_type& value : members_) {
        go::Entity* member = value.first;
        if (isLeader(*member)) {
            continue;
        }
        if (! isMemberAlive(*member)) {
            continue;
        }
        CombatState* combatState = member->queryCombatState();
        if ((combatState != nullptr) && combatState->isCombating()) {
            continue;
        }

        go::Marchable* marchable = member->queryMarchable(); assert(marchable != nullptr);
        marchable->marchStarted();
    }
}


void NpcFormation::marchStopped()
{
    std::lock_guard<LockType> lock(lock_);

    for (Members::value_type& value : members_) {
        go::Entity* member = value.first;
        if (isLeader(*member)) {
            continue;
        }
        if (! isMemberAlive(*member)) {
            continue;
        }
        CombatState* combatState = member->queryCombatState();
        if ((combatState != nullptr) && combatState->isCombating()) {
            continue;
        }

        go::Marchable* marchable = member->queryMarchable(); assert(marchable != nullptr);
        marchable->marchStopped();
    }
}


bool NpcFormation::isMember(const go::Entity& entity) const
{
    return members_.find(const_cast<go::Entity*>(&entity)) != members_.end();
}


bool NpcFormation::isMemberAlive(go::Entity& member) const
{
    if (! leader_.queryKnowable()->getEntity(member.getGameObjectInfo())) {
        return false;
    }

    if (member.queryLiveable()->getCreatureStatus().isDied()) {
        return false;
    }
    return true;
}


const gdt::npc_formation_t* NpcFormation::getFormationInfo(const go::Entity& member) const
{
    const Members::const_iterator pos = members_.find(const_cast<go::Entity*>(&member));
    if (pos != members_.end()) {
        return (*pos).second;
    }
    return nullptr;
}

}} // namespace gideon { namespace zoneserver {