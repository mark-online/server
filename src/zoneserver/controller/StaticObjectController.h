#pragma once

#include "EntityController.h"
#include "callback/EffectCallback.h"
#include "callback/StaticObjectEffectCallback.h"

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class StaticObjectController
 * Controller for StaticObject
 */
class ZoneServer_Export StaticObjectController : public EntityController,
    public EffectCallback,
    public StaticObjectEffectCallback
{
    typedef EntityController Parent;

private:
    EffectCallback* queryEffectCallback() { return this; }
    StaticObjectEffectCallback* queryStaticObjectEffectCallback() { return this; }

protected:
    // = EntityController overriding
    virtual void spawned(WorldMap& worldMap);
    virtual void despawned(WorldMap& worldMap);

private:
    // = EffectCallback override
    virtual void applyEffect(go::Entity& from,
        const SkillEffectResult& skillEffectResult);
    virtual void hitEffect(go::Entity& /*from*/, DataCode /*dataCode*/) {}
    virtual void cancelEffect(EffectStackCategory /*category*/) {}

    virtual void effectApplied(go::Entity& to, go::Entity& from,
        const SkillEffectResult& skillEffectResult);
    virtual void effectHit(go::Entity& /*to*/, go::Entity& /*from*/, DataCode /*dataCode*/) {}
protected:
    // = StaticObjectEffectCallback override
    virtual void damaged(HitPoint currentPoint, bool observerNotify, bool exceptSelf);
    virtual void repaired(HitPoint currentPoint, bool observerNotify, bool exceptSelf);
    virtual void redueced(ManaPoint currentPoint, bool observerNotify, bool exceptSelf);
    virtual void refilled(ManaPoint currentPoint, bool observerNotify, bool exceptSelf);

private:
    void hpChanged(HitPoint currentPoint, bool observerNotify, bool exceptSelf);
    void mpChanged(ManaPoint currentPoint, bool observerNotify, bool exceptSelf);

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
