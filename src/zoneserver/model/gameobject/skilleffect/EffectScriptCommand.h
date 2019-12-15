#pragma once

#include <gideon/server/data/ServerSkillEffect.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class EffectScriptCommand
 */
class EffectScriptCommand
{
public:
    EffectScriptCommand() {}

    virtual ~EffectScriptCommand() {}

    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace go {
