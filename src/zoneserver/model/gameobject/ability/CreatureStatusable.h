#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/StatusInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class CreatureStatusable
 */
class CreatureStatusable
{
public:
    virtual const CreatureStatusInfo& getCurrentCreatureStatusInfo() const = 0;
    virtual const CreatureStatusInfo& getCreatureStatusInfo() const = 0;
    virtual CreatureStatusInfo& getCurrentCreatureStatusInfo() = 0;
    virtual CreatureStatusInfo& getCreatureStatusInfo() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
