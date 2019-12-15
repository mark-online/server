#pragma once

#include "EffectScriptCommand.h"
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace go {

class StaticObject;

/**
 * @class StaticObjectEffectScriptCommand
 */
class StaticObjectEffectScriptCommand : public EffectScriptCommand
{
public:
    StaticObjectEffectScriptCommand(StaticObject& owner) :
        owner_(owner) {}

    virtual ~StaticObjectEffectScriptCommand() {}

    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove) = 0;

protected:
    StaticObject& owner_;
};


/**
 * @class StaticObjectLifePointsStatsEffectCommand
 */
class StaticObjectLifePointsStatsEffectCommand : public StaticObjectEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<StaticObjectLifePointsStatsEffectCommand>
{
public:
    StaticObjectLifePointsStatsEffectCommand(StaticObject& owner) :
        StaticObjectEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);    
};

}}} // namespace gideon { namespace zoneserver { namespace go {