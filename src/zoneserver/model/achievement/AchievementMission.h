#pragma once

#include <gideon/cs/shared/data/AchievementInfo.h>
#include <gideon/cs/shared/data/ItemInfo.h>

namespace gdt {
class achievement_t;
}

namespace gideon { namespace zoneserver { namespace go {
class Entity;
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go

namespace gideon { namespace zoneserver {

class AchievementHelper;

/***
* @class AchievementMission
***/
class AchievementMission
{
public:
    AchievementMission(AchievementHelper& helper, go::Player& player);

    void update(AchievementMissionType missionType, go::Entity* target = nullptr);
    void update(AchievementMissionType missionType, uint64_t param1, uint64_t param2);

    bool isComplete(AchievementCode achievementCode) const;

protected:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, uint64_t param1, uint64_t param2) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const = 0;

protected:
    virtual void updateMission(AchievementCode achievementCode, go::Entity* target);

protected:
    void updateMission_i(AchievementCode achievementCode, uint64_t param1, uint64_t param2, bool isAdd);

protected:
    AchievementHelper& helper_;
    go::Player& owner_;
    uint64_t param1_;
    uint64_t param2_;
};


/***
* @class AchievementBuddyMission
***/
class AchievementBuddyMission : public AchievementMission
{
public:
    AchievementBuddyMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    virtual void updateMission(AchievementCode achievementCode, go::Entity* target);
};

/***
* @class AchievementInventoryMission
***/
class AchievementInventoryMission : public AchievementMission
{
public:
    AchievementInventoryMission(AchievementHelper& helper, go::Player& player) :
        AchievementMission(helper, player) {}

private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;
};

/***
* @class AchievementGameMoneyMission
***/
class AchievementGameMoneyMission : public AchievementMission
{
public:
    AchievementGameMoneyMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    virtual void updateMission(AchievementCode achievementCode, go::Entity* target);
};


/***
* @class AchievementChaoMission
***/
class AchievementChaoMission : public AchievementMission
{
public:
    AchievementChaoMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    virtual void updateMission(AchievementCode achievementCode, go::Entity* target);
};


/***
* @class AchievementEquipMission
***/
class AchievementEquipMission : public AchievementMission
{
public:
    AchievementEquipMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    virtual void updateMission(AchievementCode achievementCode, go::Entity* target);

private:
    mutable ItemGrade itemGrade_;
};


/***
* @class AchievementStoreMission
***/
class AchievementStoreMission : public AchievementMission
{
public:
    AchievementStoreMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;
};


/***
* @class AchievementExecutionMission
***/
class AchievementExecutionMission : public AchievementMission
{
public:
    AchievementExecutionMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;
};

/***
* @class AchievementLevelUpMission
***/
class AchievementLevelUpMission : public AchievementMission
{
public:
    AchievementLevelUpMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    virtual void updateMission(AchievementCode achievementCode, go::Entity* target);
};


/***
* @class AchievementTreasureMission
***/
class AchievementTreasureMission : public AchievementMission
{
public:
    AchievementTreasureMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}
private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;
};


/***
* @class AchievementHunterMission
***/
class AchievementHunterMission : public AchievementMission
{
public:
    AchievementHunterMission(AchievementHelper& helper, go::Player& player) :
        AchievementMission(helper, player) {}
private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;
};

/***
* @class AchievementQuestMission
***/
class AchievementQuestMission : public AchievementMission
{
public:
    AchievementQuestMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}
private:
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    void updateMission(AchievementCode achievementCode, go::Entity* target);
};


/***
* @class AchievementPVPKillMission
***/
class AchievementPVPKillMission : public AchievementMission
{
public:
    AchievementPVPKillMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    void updateMission(AchievementCode achievementCode, go::Entity* target);
};


/***
* @class AchievementPVPDeathMission
***/
class AchievementPVPDeathMission : public AchievementMission
{
public:
    AchievementPVPDeathMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    void updateMission(AchievementCode achievementCode, go::Entity* target);
};



/***
* @class AchievementPvPWinMission
***/
class AchievementPvPWinMission : public AchievementMission
{
public:
    AchievementPvPWinMission(AchievementHelper& helper, go::Player& player) :
      AchievementMission(helper, player) {}

private:
    virtual bool shouldSkipMission(const gdt::achievement_t& mission, go::Entity* target) const;
    virtual bool checkComplete(const ProcessAchievementInfo& missionInfo, const gdt::achievement_t& missionTemplate) const;

private:
    void updateMission(AchievementCode achievementCode, go::Entity* target);
};

}} // namespace gideon { namespace zoneserver {