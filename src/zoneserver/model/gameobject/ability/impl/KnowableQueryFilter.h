#pragma once

#include "../Knowable.h"
#include "../../Entity.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/SkillEffectTable.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class EffectInfoableEntityQueryFilter
 * 스킬 방향성 필터
 */
class EffectInfoableEntityQueryFilter : public Knowable::QueryFilter
{
public:
    EffectInfoableEntityQueryFilter(const go::Entity& entity,
        const datatable::CheckEffectInfo& checkEffectInfo) :
        entity_(entity),
        checkEffectInfo_(checkEffectInfo) {}

    virtual bool isOk(const go::Entity& target, const Position& dirTarget) const {
        if (! isAllowedDirection(dirTarget)) {
            return false;
        }
        if (checkEffectInfo_.exceptTarget_ == etSelf) {            
            if (target.isSame(entity_)) {
                return false;
            }
        }
        return true;
    }

private:
    bool isAllowedDirection(const Position& dirTarget) const {
        if (checkEffectInfo_.effectRangeDirection_ == erdNone) {
            return true;
        }

        const bool isForward =
            isInForward(asVector2(dirTarget), entity_.getPosition().heading_);

        if (checkEffectInfo_.effectRangeDirection_ == erdForward) {
            if (isForward) {
                return true;
            }
        }
        else if (checkEffectInfo_.effectRangeDirection_ == erdBackward) {
            if (! isForward) {
                return true;
            }
        }
        else {
            assert(false);
        }
        return false;
    }

private:
    const go::Entity& entity_;
    const datatable::CheckEffectInfo& checkEffectInfo_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
