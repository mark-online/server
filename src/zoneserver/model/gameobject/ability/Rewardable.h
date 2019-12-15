#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/LevelInfo.h>

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class Rewardable
 * 보상을을 줄수 있다
 */
class Rewardable
{
public:
	virtual ExpPoint getRewardExpPoint() const = 0;
    // return
    virtual void expReward(bool& isLevelUp, bool& isMajorLevelUp, ExpPoint exp) = 0;

	virtual ErrorCode addRewardSkill(SkillCode skillCode) = 0;

    virtual ErrorCode addCharacterTitle(CharacterTitleCode characterTitleCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
