#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/SkillInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class SkillCallback
 */
class SkillCallback
{
public:
    virtual ~SkillCallback() {}

public:
    virtual void activateConcentrationCancelled(const GameObjectInfo& sourceInfo,
        SkillCode skillCode) = 0;
	virtual void activateConcentrationCompleted(const GameObjectInfo& sourceInfo,
		SkillCode skillCode) = 0;
    virtual void passiveActivated(const GameObjectInfo& creatureInfo, SkillCode skillCode) = 0;
    virtual void skillLearned(SkillCode oldSkillCode, SkillCode newSkillCode) = 0;
    virtual void skillRemoved(SkillCode skillCode) = 0;
    virtual void skillAllRemoved() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
