#include "ZoneServerPCH.h"
#include "StaticObjectEffectScriptApplier.h"
#include "../StaticObject.h"
#include "../skilleffect/StaticObjectEffectScriptCommand.h"
#include "../status/StaticObjectStatus.h"

namespace gideon { namespace zoneserver { namespace go {

StaticObjectEffectScriptApplier::StaticObjectEffectScriptApplier(Entity& owner) :
    EffectScriptApplier(owner)
{
}


void StaticObjectEffectScriptApplier::initialzeEffectCommand()
{
    go::StaticObject& owner = getOwnerAs<go::StaticObject>();
    commandMap_.insert(EffectSciptCommandMap::value_type(
        ssetLifeStats, new StaticObjectLifePointsStatsEffectCommand(owner)));
}



void StaticObjectEffectScriptApplier::excuteEffect(ServerEffectPtr effectPtr)
{
    for (int i = 0; i < eanCount; ++i) {
        if (! effectPtr->effects_[i].shouldExcute_ || effectPtr->effects_[i].isRemove_ || ! isValid(effectPtr->effects_[i].script_)) {
            continue;
        }        
        EffectScriptCommand* command = getEffectInfoCommand(effectPtr->effects_[i].script_);
        if (command) {
            command->execute(effectPtr, toEffectApplyNum(i), false);
            effectPtr->effects_[i].shouldExcute_ = false;
            if (! canExecuteEffect()) {
                effectPtr->effects_[i].isRemove_ = true;
                return;            
            }
            
        }
    }
}


bool StaticObjectEffectScriptApplier::canExecuteEffect() const
{    
    if (getOwnerAs<go::StaticObject>().getStaticObjectStatus().isMinHp()) {
        if (getOwnerAs<go::StaticObject>().canDestory()) {
            return false;
        }
    }
	   
    return true;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
