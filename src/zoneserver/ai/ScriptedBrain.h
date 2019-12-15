#pragma once

#include "Brain.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class ScriptedBrain
 * 스크립트 기반의 AI
 */
class ScriptedBrain :
    public Brain,
    private Scriptable
{
public:
    ScriptedBrain(go::Entity& owner);
    virtual ~ScriptedBrain();

protected:
    bool isActivated() const {
        return isActivated_;
    }

protected:
    // = Brain overriding
    virtual void initialize() {}
    virtual void finalize() {}

    virtual void activate();
    virtual void deactivate();

    //virtual void analyze(GameTime diff);

    virtual void spawned() {}
    virtual void despawned() {}
    virtual void attacked(const go::Entity& /*attacker*/) {}
    virtual void died() {}
    virtual void combatStarted() {}
    virtual void combatStopped() {}
    virtual void movedInLineOfSight(go::Entity& /*entity*/) {}
    virtual void wanderPaused() {}
    virtual void pathNodeArrived() {}

    virtual Scriptable* queryScriptable() { return this; }
    virtual const Scriptable* queryScriptable() const { return this; }

protected:
    // = Scriptable overriding

private:
    bool isActivated_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
