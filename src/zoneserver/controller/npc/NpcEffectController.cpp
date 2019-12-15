#include "ZoneServerPCH.h"
#include "NpcEffectController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Thinkable.h"
#include "../../ai/Brain.h"
#include "../../ai/aggro/AggroList.h"

namespace gideon { namespace zoneserver { namespace gc {

// = EffectCallback overriding

void NpcEffectController::effectApplied(go::Entity& to, go::Entity& from,
	const SkillEffectResult& skillEffectResult)
{
    to, from, skillEffectResult;
    //if (to.isSame(getOwner()) && from.isEffectInfoable()) {
    //    go::Thinkable* thinkable = getOwner().queryThinkable();
    //    if (thinkable != nullptr) {
    //        ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
    //        if (aggroList != nullptr) {
    //            aggroList->addEffectInfo(from, skillEffectResult);
    //        }
    //    }
    //}
}


void NpcEffectController::effectHit(go::Entity& /*to*/, go::Entity& /*from*/, DataCode /*dataCode*/)
{

}

}}} // namespace gideon { namespace zoneserver { namespace gc {
