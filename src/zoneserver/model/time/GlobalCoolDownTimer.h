#pragma once

#include <gideon/cs/shared/data/Time.h>
#include <array>

namespace gideon { namespace zoneserver {


/***
 * class GlobalCoolDownTimer
 ***/
class GlobalCoolDownTimer
{
    typedef std::mutex LockType;
    enum {timeCount = 5};
public:
    GlobalCoolDownTimer();
    ~GlobalCoolDownTimer();

    void setNextGlobalCooldownTime(uint32_t index, msec_t nextCooldownTime);
    void cancelCooldown(uint32_t index);
	void cancelPreCooldown();
    void reset();

public:
    msec_t getNextGlobalCooldownTime(uint32_t index) const;

private:
    void cancelCooldown_i(uint32_t index) {
        assert(index < timeCount);
        times_[index] = 0;
    }

private:
    typedef std::array<GameTime, timeCount> GlobalCoolTimes;
    GlobalCoolTimes times_;
	uint32_t preIndex_;
	mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {
