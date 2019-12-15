#include "ZoneServerPCH.h"
#include "NpcStatus.h"
#include "../Npc.h"

namespace gideon { namespace zoneserver { namespace go {

// = CreatureStatus overriding

bool NpcStatus::reduceHp_i(HitPoint& applyPoint, HitPoint hp, bool canDie)
{
    const bool hpChanged = CreatureStatus::reduceHp_i(applyPoint, hp, canDie);

    npcInfo_.currentPoints_.hp_ = getCurrentPoints().hp_;
    return hpChanged;
}


bool NpcStatus::reduceMp_i(ManaPoint& /*applyPoint*/, ManaPoint /*mp*/)
{
    // NPC는 MP/CP를 사용하지 않는다
    return false;
}


bool NpcStatus::fillHp_i(HitPoint& applyPoint, HitPoint hp, bool force)
{
    const bool hpChanged = CreatureStatus::fillHp_i(applyPoint, hp, force);

    npcInfo_.currentPoints_.hp_ = getCurrentPoints().hp_;
    return hpChanged;
}


bool NpcStatus::fillMp_i(ManaPoint& /*applyPoint*/, ManaPoint mp)
{
    mp; // NPC는 MP/CP를 사용하지 않는다
    return false;
}


void NpcStatus::restorePoints_i()
{
    CreatureStatus::restorePoints_i();

    npcInfo_.currentPoints_ = getCurrentPoints();
}


void NpcStatus::setCurrentPoint(const Points& points)
{
    npcInfo_.currentPoints_ = points;
}


void NpcStatus::upBaseStatus(BaseStatusIndex index, int32_t value)
{
    // TODO skill
    index, value;
}


void NpcStatus::downBaseStatus(BaseStatusIndex index, int32_t value)
{
    // TODO skill
    index, value;
}


void NpcStatus::updateAllStatus()
{
    // TODO skill
}

}}} // namespace gideon { namespace zoneserver { namespace go {
