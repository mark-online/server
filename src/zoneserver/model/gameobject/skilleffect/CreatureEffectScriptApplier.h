#pragma once

#include "EffectScriptApplier.h"

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class CreatureEffectScriptApplier
 * active skill
 */
class CreatureEffectScriptApplier :
    public EffectScriptApplier
{
public:
    CreatureEffectScriptApplier(go::Entity& owner);
    ~CreatureEffectScriptApplier();

    const DebuffBuffEffectInfoSet& getDebuffBuffEffectInfoSet() const;

private:
    // = EffectScriptApplier overriding
    virtual void initialzeEffectCommand(); 

    virtual void excuteEffect(ServerEffectPtr skillEffect);

    virtual void notifyEffectAdded(const DebuffBuffEffectInfo& info, bool isNotify);
    virtual void notifyEffectRemoved(DataCode dataCode, bool isCaster);

private:
    virtual bool canExecuteEffect() const;

private:
    void died();

private:
    void rewardExp(const GameObjectInfo& skillCasterInfo);

private:
    DebuffBuffEffectInfoSet debuffBuffEffectInfoSet_;
};


}}} // namespace gideon { namespace zoneserver { namespace go {