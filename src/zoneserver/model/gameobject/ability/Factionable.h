#pragma once

#include <gideon/cs/shared/data/FactionInfo.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class Factionable
 * 파벌(faction)을 인식한다
 */
class Factionable
{
public:
    virtual ~Factionable() {}

public:
    virtual void setFaction(FactionCode factionCode) = 0;

public:
    virtual bool isFriendlyTo(const Entity& target) const = 0;
    virtual bool isHostileTo(const Entity& target) const = 0;

    virtual FactionCode getFactionCode() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
