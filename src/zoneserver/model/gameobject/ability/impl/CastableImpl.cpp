#include "ZoneServerPCH.h"
#include "CastableImpl.h"
#include "../SkillCastable.h"
#include "../ItemCastable.h"
#include "../CastGameTimeable.h"
#include "../../Entity.h"
#include "../../../state/CastState.h"
#include "../../../../controller/EntityController.h"

namespace gideon { namespace zoneserver { namespace go {

CastableImpl::CastableImpl(Entity& owner) :
    owner_(owner)
{
}
 

CastableImpl::~CastableImpl()
{
}


bool CastableImpl::isCasting() const
{
    sne::base::Future::Ref castingTask = castingTask_.lock();
    if (castingTask.get()) {
		if (! (castingTask->isCancelled() && castingTask->isDone())) {
			return true;
		}
    }

	sne::base::Future::Ref activeConcentrationTask = activeConcentrationTask_.lock();
	if (activeConcentrationTask.get()) {
		if (! (activeConcentrationTask->isCancelled() && activeConcentrationTask->isDone())) {
			return true;
		}
	}

	return false;
}


void CastableImpl::startCasting(DataCode dataCode, bool canMovingCastable,
    sne::base::RunnablePtr task, GameTime interval)
{
    CastState* castState = owner_.queryCastState();
    if (castState) {
        castState->castStarted(dataCode, canMovingCastable);
    }
    castingTask_ = TASK_SCHEDULER->schedule(std::move(task), interval);
}


void CastableImpl::cancelCasting()
{
    releaseCastState(true);
    sne::base::Future::Ref castingTask = castingTask_.lock();
    if (castingTask.get() != nullptr) {
        castingTask->cancel();
        castingTask_.reset();
    }
	cancelConcentrationSkill();
	go::CastGameTimeable* castable = owner_.queryCastGameTimeable();
	if (castable) {
		castable->cancelPreCooldown();
	}
}


void CastableImpl::releaseCastState(bool isCancelRelase)
{
	CastState* castState = owner_.queryCastState();
	if (castState) {
		castState->castReleased(isCancelRelase);
	}
}


void CastableImpl::activateConcentrationSkill(sne::base::RunnablePtr task, GameTime interval)
{
	activeConcentrationTask_ = TASK_SCHEDULER->schedule(std::move(task), 0, interval);
}


void CastableImpl::cancelConcentrationSkill()
{
	sne::base::Future::Ref activeConcentrationTask = activeConcentrationTask_.lock();
	if (! activeConcentrationTask.get()) {
		return;
	}

	if (activeConcentrationTask->isCancelled()) {
		return;
	}

	activeConcentrationTask->cancel();
	activeConcentrationTask_.reset();
}


}}} // namespace gideon { namespace zoneserver { namespace go {
