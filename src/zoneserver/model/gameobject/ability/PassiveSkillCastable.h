#pragma once

#include <gideon/server/data/ServerSkillEffect.h>

namespace gideon { namespace zoneserver { namespace go {

class PassiveSkillManager;

/**
 * @PassiveSkillCastable
 */
class PassiveSkillCastable
{
public:
    virtual void notifyChangeCondition(PassiveCheckCondition condition) = 0;
    virtual PassiveSkillManager& getPassiveSkillManager() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {