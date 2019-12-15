#pragma once

#include "../Castable.h"
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class CastableImpl
 * 시전할수 있다
 */
class CastableImpl : public Castable
{
public:    
    CastableImpl(Entity& owner);
    virtual ~CastableImpl();
    
private:
    virtual bool isCasting() const; 

    virtual void startCasting(DataCode dataCode, bool canMovingCastable,
        sne::base::RunnablePtr task, GameTime interval);
    virtual void cancelCasting();
	virtual void releaseCastState(bool isCancelRelase);

	virtual void activateConcentrationSkill(sne::base::RunnablePtr task, GameTime interval);
	virtual void cancelConcentrationSkill();

private:
    Entity& owner_;
    sne::base::Future::WeakRef castingTask_;
	sne::base::Future::WeakRef activeConcentrationTask_;
};


}}} // namespace gideon { namespace zoneserver { namespace go {