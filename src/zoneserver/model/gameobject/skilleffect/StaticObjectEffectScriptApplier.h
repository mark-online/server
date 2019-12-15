#pragma once

#include "EffectScriptApplier.h"

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class StaticObjectEffectScriptApplier
 * active skill
 */
class StaticObjectEffectScriptApplier : public EffectScriptApplier
{
public:
    StaticObjectEffectScriptApplier(Entity& owner);

private: // = EffectScriptApplier overriding
    virtual void initialzeEffectCommand() override; 

    virtual void excuteEffect(ServerEffectPtr skillEffect) override;

    virtual void notifyEffectAdded(const DebuffBuffEffectInfo& /*Info*/, bool /*isNotify*/) override {}
    virtual void notifyEffectRemoved(DataCode /*dataCode*/, bool /*isCaster*/) override {}

private:
    virtual bool canExecuteEffect() const;
};


}}} // namespace gideon { namespace zoneserver { namespace go {
