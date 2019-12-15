#pragma once

#include <gideon/cs/shared/data/SkillInfo.h>


namespace gideon { namespace zoneserver {

/***
 * @class CastState
 ***/
class CastState
{
public:
	virtual ~CastState() {}

public:
    virtual bool canCasting() const = 0;
	virtual DataCode getCastCode() const = 0;
    virtual bool isCastingAndNotMovingCast() const = 0;

	virtual void releaseNotSkillCastingByAttack() = 0;

	virtual void castStarted(DataCode dataCode, bool isMovingCasting) = 0;
	virtual void castReleased(bool isCancelRelease) = 0;
};

}} //namespace gideon { namespace zoneserver {
