#pragma once

#include "CastState.h"
#include <gideon/cs/shared/data/SkillInfo.h>


namespace gideon { namespace zoneserver {

class SkillCasterState : public CastState
{
public:
	virtual ~SkillCasterState() {}

public:
    virtual bool canCastSkill(bool isCheckMez) const = 0;
    virtual bool isActivateConcentrationSkill() const = 0;

    virtual bool shouldStopDuringCasting(SkillCode skillCode) const {
        skillCode;
        return false;
    }

public:
	virtual void concentrationSkillActivated(SkillCode skillCode, bool isMovingSkill) = 0;
    virtual void concentrationSkillReleased() = 0;

};

}} //namespace gideon { namespace zoneserver {
