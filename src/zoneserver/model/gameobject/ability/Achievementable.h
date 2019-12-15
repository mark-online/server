#pragma once

#include <gideon/cs/shared/data/AchievementInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {
class AchievementRepository;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class Achievementable
 * 업적을 달성할수 있다
 */
class Achievementable
{
public:
    virtual ~Achievementable() {}
	
public:
    virtual void initializeAchievement(ErrorCode errorCode, const ProcessAchievementInfoMap& processInfoMap,
        const CompleteAchievementInfoMap& completeInfoMap) = 0;
    virtual void updateAchievement(AchievementMissionType missionType, Entity* target = nullptr) = 0;
    virtual void updateAchievement(AchievementMissionType missionType, uint32_t param1, uint32_t param2) = 0;
	
};

}}} // namespace gideon { namespace zoneserver { namespace go {
