#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/SkillInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PassiveSkillCallback
 */
class PassiveSkillCallback
{
public:
    virtual ~PassiveSkillCallback() {}

    virtual void activatePassive(SkillCode skillCode) = 0;

public:
    virtual void passiveActivated(const GameObjectInfo& creatureInfo, SkillCode skillCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
