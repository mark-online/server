#pragma once

#include "AchievementHelper.h"
#include <gideon/cs/shared/data/AchievementInfo.h>

namespace gdt {
class achievement_t;
}

namespace gideon { namespace zoneserver { namespace go {
class Entity;
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

class AchievementMission;

/**
 * @class AchievementRepository
 */
class AchievementRepository : public AchievementHelper
{
    typedef std::mutex LockType;
    typedef sne::core::Map<AchievementMissionType, AchievementMission*> MissionMap;
public:
    AchievementRepository(go::Player& owner, const ProcessAchievementInfoMap& processAchievementInfoMap,
        const CompleteAchievementInfoMap& completeAchievementInfoMap);
    ~AchievementRepository();


    void update(AchievementMissionType missionType, go::Entity* target = nullptr);
    void update(AchievementMissionType missionType, uint64_t param1, uint64_t param2);

private:
    // AchievementHelper overriding
    virtual bool isCompleted(AchievementCode code) const;
    virtual bool isProcessed(AchievementCode code) const;

    virtual ProcessAchievementInfo* getProcessMissionInfo(AchievementCode code);
    virtual void addProcessAchievementInfo(AchievementCode code, uint64_t param1, uint64_t param2);

private:
    void updateCompleteAchievement(AchievementMissionType missionType);

    void completeAchievement(const gdt::achievement_t& achievement);
    void reward(AchievementCode achievementCode, DataCode dataCode, uint64_t value);

private:
    LockType lock_;
    go::Player & owner_;
    ProcessAchievementInfoMap processAchievementInfoMap_;
    CompleteAchievementInfoMap completeAchievementInfoMap_;
    MissionMap missionMap_;

};

}} // namespace gideon { namespace zoneserver {