#include "ZoneServerPCH.h"
#include "StaticObjectEffectScriptCommand.h"
#include "../StaticObject.h"
#include "../status/StaticObjectStatus.h"

namespace gideon { namespace zoneserver { namespace go {


void StaticObjectLifePointsStatsEffectCommand::execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool /*isRemove*/)
{
    ServerEffectInfo::Effect& effect = effectPtr->effects_[num];

    switch (effect.script_) {
    case estHpDown:
        if (! owner_.getStaticObjectStatus().isMinHp()) {
            owner_.getStaticObjectStatus().reduceHp(toHitPoint(effect.value_));
        }
        break;
    }
  
}

}}} // namespace gideon { namespace zoneserver { namespace go {