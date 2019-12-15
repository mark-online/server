#pragma once


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class StaticObjectEffectCallback
 */
class StaticObjectEffectCallback
{
public:
    virtual ~StaticObjectEffectCallback() {}
public:

    virtual void damaged(HitPoint currentPoint, bool observerNotify, bool exceptSelf = true) = 0;
    virtual void repaired(HitPoint currentPoint, bool observerNotify, bool exceptSelf = true) = 0;
    virtual void redueced(ManaPoint currentPoint, bool observerNotify, bool exceptSelf = true) = 0;
    virtual void refilled(ManaPoint currentPoint, bool observerNotify, bool exceptSelf = true) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace gc {