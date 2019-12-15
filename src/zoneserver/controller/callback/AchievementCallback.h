#pragma once

#include <gideon/cs/shared/data/AchievementInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class AchievementCallback
 */
class AchievementCallback
{
public:
    virtual ~AchievementCallback() {}

public:
    virtual void achievementUpdated(AchievementCode code, const ProcessAchievementInfo& missionInfo) = 0;
    virtual void achievementCompleted(AchievementCode code, AchievementPoint point, sec_t now) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
