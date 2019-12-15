#pragma once

#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/StatusInfo.h>
#include <gideon/cs/shared/data/EnvironmentInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class MoveState
 **/
class MoveState
{
public:
    bool canMove() const {
        return (! isRooted()) && (! isControlAbsorbed());
    }

public:
    /// 발이 묶였는가? (이동 불가)
	virtual bool isRooted(const Position* position = nullptr) const = 0;

    virtual bool isRunning() const = 0;
	
    /// 이동 제어를 빼앗겼는가? (변이 등에 의해)
    virtual bool isControlAbsorbed() const = 0;

    /// 
    virtual bool isInLava() const = 0;

    virtual HitPoint getFallDamage() const = 0;
    
public:
	virtual void moved(bool turn = false) = 0;

    virtual void run() = 0;

    virtual void walked() = 0;

    virtual void fallen() = 0;

    virtual void landed() = 0;

    virtual void environmentEntered(EnvironmentType type) = 0;
    virtual void environmentLeft() = 0;
};

}} // namespace gideon { namespace zoneserver {
