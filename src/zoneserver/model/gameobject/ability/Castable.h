#pragma once

#include <gideon/cs/shared/data/CastInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class CastingType
 */
enum CastingType
{
    castingSkill,
};

/**
 * @class Castable
 * 시전할수 있다
 */
class Castable
{
public:    
    virtual ~Castable() {}

    virtual bool isCasting() const = 0;

    virtual void startCasting(DataCode dataCode, bool canMovingCastable,
        sne::base::RunnablePtr task, GameTime interval) = 0;
    virtual void cancelCasting() = 0;
	virtual void releaseCastState(bool isCancelRelease) = 0;

	virtual void activateConcentrationSkill(sne::base::RunnablePtr task, GameTime interval) = 0;
	virtual void cancelConcentrationSkill() = 0;

};


}}} // namespace gideon { namespace zoneserver { namespace go {