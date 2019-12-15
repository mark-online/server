#pragma once

#include <gideon/cs/shared/data/CooldownInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CooldownCallback
 */
class CooldownCallback
{
public:
    virtual ~CooldownCallback() {}

    virtual void cooldownInfosAdded(const CooltimeInfos& cooltimeInfos) = 0;
    virtual void remainEffectsAdded(const RemainEffectInfos& effectInfos) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
