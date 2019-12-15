#include "ZoneServerPCH.h"
#include "StaticObjectStatus.h"
#include "../StaticObject.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/StaticObjectEffectCallback.h"

namespace gideon { namespace zoneserver { namespace go {

StaticObjectStatus::StaticObjectStatus(go::StaticObject& owner, StaticObjectStatusInfo& staticObjectStatusInfo) :
    owner_(owner),
    staticObjectStatusInfo_(staticObjectStatusInfo)
{
}


bool StaticObjectStatus::isMinHp() const
{
    return staticObjectStatusInfo_.hitPoints_.isMin();
}


HitPoints StaticObjectStatus::getHitPoints() const
{
    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

    return staticObjectStatusInfo_.hitPoints_;
}


bool StaticObjectStatus::reduceHp(HitPoint hp)
{
    HitPoint currentHitPoint = hpMin;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        const bool hpChanged = reduceHp_i(hp);
        if (! hpChanged) {
            return false;
        }
        currentHitPoint = staticObjectStatusInfo_.hitPoints_.hp_;
    }
       
    pointChanged(ptHp, currentHitPoint, true);

    return true;
}


void StaticObjectStatus::fillHp(HitPoint hp)
{
    HitPoint currentHitPoint = hpMin;
    {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        const bool hpChanged = fillHp_i(hp);
        if (! hpChanged) {
            return;
        }
        currentHitPoint = staticObjectStatusInfo_.hitPoints_.hp_;

    }

    pointChanged(ptHp, currentHitPoint, false);
}


bool StaticObjectStatus::reduceHp_i(HitPoint hp)
{
    HitPoint prevHp = staticObjectStatusInfo_.hitPoints_.hp_;
    staticObjectStatusInfo_.hitPoints_.reduce(hp);

    return staticObjectStatusInfo_.hitPoints_.hp_ != prevHp;
}


bool StaticObjectStatus::fillHp_i(HitPoint hp)
{
    HitPoint prevHp = staticObjectStatusInfo_.hitPoints_.hp_;
    staticObjectStatusInfo_.hitPoints_.refill(hp);

    return staticObjectStatusInfo_.hitPoints_.hp_ != prevHp;
}


void StaticObjectStatus::pointChanged(PointType pointType, uint32_t currentPoint, bool isDamage)
{
    gc::StaticObjectEffectCallback* callback = owner_.getController().queryStaticObjectEffectCallback();
    if (! callback) {
        return;
    }
    if (isDamage) {
        if (ptHp == pointType) {
            callback->damaged(toHitPoint(currentPoint), true, true);
        }
    }
    else {
        if (ptHp == pointType) {
            callback->repaired(toHitPoint(currentPoint), true, true);
        }
    }
    
}


}}} // namespace gideon { namespace zoneserver { namespace go {
