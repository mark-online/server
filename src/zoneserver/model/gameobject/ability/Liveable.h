#pragma once

#include <memory>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Rate.h>

namespace gideon { namespace zoneserver { namespace go {

class CreatureStatus;

/**
 * @class Liveable
 * 살아갈 수 있다(말장난?;;)
 */
class Liveable
{
public:
    virtual ~Liveable() {}

public:
    virtual std::unique_ptr<CreatureStatus> createCreatureStatus() = 0;

public:
    virtual CreatureStatus& getCreatureStatus() = 0;

    virtual const CreatureStatus& getCreatureStatus() const = 0;

	virtual ErrorCode revive(bool skipTimeCheck = false) = 0;

    virtual bool reviveByEffect(HitPoint& refillPoint, permil_t perRefillHp) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
