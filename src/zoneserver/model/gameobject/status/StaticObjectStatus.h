#pragma once

#include "../../../zoneserver_export.h"
#include <gideon/cs/shared/data/StaticObjectStatusInfo.h>
#include <gideon/cs/shared/data/UnionEntityInfo.h>

namespace gideon { namespace zoneserver { namespace go {


class StaticObject;

/**
 * @class StaticObjectStatus
 */
class StaticObjectStatus : public boost::noncopyable
{
public:
    StaticObjectStatus(go::StaticObject& owner, StaticObjectStatusInfo& staticObjectStatusInfo);
    virtual ~StaticObjectStatus() {}

public:
    HitPoints getHitPoints() const;

    bool isMinHp() const;

    bool reduceHp(HitPoint hp);

    void fillHp(HitPoint hp);

    const StaticObjectStatusInfo& getStaticObjectStatusInfo() const {
        return staticObjectStatusInfo_;
    }

protected:
    bool reduceHp_i(HitPoint hp);
    bool fillHp_i(HitPoint hp);
    
    void pointChanged(PointType pointType, uint32_t currentPoint, bool isDamage);

private:
    go::StaticObject& owner_;
    StaticObjectStatusInfo& staticObjectStatusInfo_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
