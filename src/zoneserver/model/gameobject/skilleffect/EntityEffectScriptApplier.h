#pragma once

#include "EffectScriptApplier.h"

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class EntityEffectScriptApplier
 * active skill
 */
class EntityEffectScriptApplier :
    public EffectScriptApplier
{
public:
    EntityEffectScriptApplier(go::Entity& owner) :
        EffectScriptApplier(owner) {}
    ~EntityEffectScriptApplier() {}

private:
    // = EffectScriptApplier overriding
    virtual void initialzeEffectCommand() {} 
    virtual void applySkill(Entity& /*from*/, const SkillEffectResult& /*skillEffectResult*/) {}
    virtual void excuteEffect(ServerEffectPtr /*skillEffect*/) {}

    virtual void notifyEffectAdded(const DebuffBuffEffectInfo& /*info*/, bool /*isNotify*/) {}
    virtual void notifyEffectRemoved(DataCode /*dataCode*/, bool /*isCaster*/) {}

private:
    virtual bool canExecuteEffect() const {return false;}
};


}}} // namespace gideon { namespace zoneserver { namespace go {