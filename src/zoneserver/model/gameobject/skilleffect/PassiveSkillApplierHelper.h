#pragma once

#include <gideon/cs/shared/data/SkillInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class PassiveSkillApplierHelper
 */
class PassiveSkillApplierHelper
{
public:
    virtual bool applyEffect(EffectStackCategory category) = 0;
	virtual void deapplyEffect(EffectStackCategory category) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {