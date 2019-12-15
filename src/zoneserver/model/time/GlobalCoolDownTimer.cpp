#include "ZoneServerPCH.h"
#include "GlobalCoolDownTimer.h"
#include "../../service/time/GameTimer.h"


namespace gideon { namespace zoneserver {

GlobalCoolDownTimer::GlobalCoolDownTimer() :
	preIndex_(0)
{
    times_.fill(0);
}


GlobalCoolDownTimer::~GlobalCoolDownTimer()
{
}


msec_t GlobalCoolDownTimer::getNextGlobalCooldownTime(uint32_t index) const
{
    assert(index < timeCount);

    std::lock_guard<LockType> lock(lock_);

    return times_[index];
}


void GlobalCoolDownTimer::setNextGlobalCooldownTime(uint32_t index, GameTime globalCoolTime)
{    
    assert(index < timeCount);

    GameTime now = GAME_TIMER->msec();

    std::lock_guard<LockType> lock(lock_);

    times_[index] = now + globalCoolTime;
	preIndex_ = index;
}


void GlobalCoolDownTimer::cancelCooldown(uint32_t index)
{
    std::lock_guard<LockType> lock(lock_);

    cancelCooldown_i(index);
}


void GlobalCoolDownTimer::cancelPreCooldown()
{
    std::lock_guard<LockType> lock(lock_);

	cancelCooldown_i(preIndex_);
}


void GlobalCoolDownTimer::reset()
{
    std::lock_guard<LockType> lock(lock_);

    times_.fill(0);
    preIndex_ = 0;
}

}} // namespace gideon { namespace zoneserver {
