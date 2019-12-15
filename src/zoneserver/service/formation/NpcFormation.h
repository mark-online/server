#pragma once

#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Formable.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gdt {
class npc_formation_t;
} // namespace gdt {

namespace gideon { namespace zoneserver {

/**
 * @class NpcFormation
 * NPC 대형
 */
class NpcFormation :
    public sne::core::ThreadSafeMemoryPoolMixin<NpcFormation>
{
    enum ReactType {
        rtNothing = 0,
        rtLeaderOnly = 1,
        rtAlways
    };

    typedef sne::core::HashMap<go::Entity*, const gdt::npc_formation_t*> Members;

    typedef std::mutex LockType;

public:
    static bool hasForm(const go::Entity& entity);

public:
    NpcFormation(go::Entity& leader) :
        leader_(leader) {}

    bool formUp(NpcFormationRefPtr thisFormation);
    void dismiss();

    void leave(go::Entity& member);

public:
    void leaderMoved(const Position& position);

    void memberAttacked(const go::Entity& actor, const go::Entity& target);

    void memberTiredToAttack(const go::Entity& actor);

    void marchStarted();
    void marchStopped();

public:
    go::Entity& getLeader() {
        return leader_;
    }

    bool isLeader(const go::Entity& entity) const {
        return leader_.isSame(entity);
    }

    bool shouldFollowLeader(const go::Entity& entity) const {
        return ! isLeader(entity);
    }

private:
    bool isMember(const go::Entity& entity) const;
    bool isMemberAlive(go::Entity& member) const;

    const gdt::npc_formation_t* getFormationInfo(const go::Entity& member) const;

private:
    go::Entity& leader_;
    Members members_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {