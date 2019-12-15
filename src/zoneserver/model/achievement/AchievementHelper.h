#pragma once

#include <gideon/cs/shared/data/AchievementInfo.h>

namespace gideon { namespace zoneserver {

/***
* @class AchievementHelper
***/
class AchievementHelper
{
public:
    virtual bool isCompleted(AchievementCode code) const = 0;
    virtual bool isProcessed(AchievementCode code) const = 0;

    virtual ProcessAchievementInfo* getProcessMissionInfo(AchievementCode code) = 0;
    
    virtual void addProcessAchievementInfo(AchievementCode code, 
         uint64_t param1, uint64_t param2) = 0;
};

}} // namespace gideon { namespace zoneserver {