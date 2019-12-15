#include "ZoneServerPCH.h"
#include "AchievementMission.h"
#include "AchievementHelper.h"
#include "../item/EquipInventory.h"
#include "../quest/QuestRepository.h"
#include "../gameobject/Player.h"
#include "../gameobject/Npc.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/AchievementCallback.h"
#include "../../world/WorldMap.h"
#include <gideon/cs/datatable/AchievementTable.h>


namespace gideon { namespace zoneserver {

AchievementMission::AchievementMission(AchievementHelper& helper, go::Player& player) :
    helper_(helper),
    owner_(player),
    param1_(0),
    param2_(0)
{
}


void AchievementMission::update(AchievementMissionType achievementType, go::Entity* target)
{
    assert(! (achievementType == acmtAddBuddy || achievementType == acmtStore));
    const datatable::AchievementTable::Achievements& achievements = 
        ACHIEVEMENT_TABLE->getAchievements(achievementType);
    for (const gdt::achievement_t* achievement : achievements) {
        const AchievementCode achievementCode = achievement->achievement_code();
        if (helper_.isCompleted(achievementCode)) {
            continue;
        }

        if (shouldSkipMission(*achievement, target)) {
            continue;
        }
        
        updateMission(achievementCode, target);
    }
}


void AchievementMission::update(AchievementMissionType achievementType, uint64_t param1, uint64_t param2)
{
    assert(achievementType == acmtAddBuddy || achievementType == acmtStore);
    const datatable::AchievementTable::Achievements& achievements = 
        ACHIEVEMENT_TABLE->getAchievements(achievementType);
    for (const gdt::achievement_t* achievement : achievements) {
        const AchievementCode achievementCode = achievement->achievement_code();
        if (helper_.isCompleted(achievementCode)) {
            continue;
        }

        if (shouldSkipMission(*achievement, param1, param2)) {
            continue;
        }

        param1_ = param1;
        param2_ = param2;

        updateMission(achievementCode, nullptr);
    }
}


bool AchievementMission::isComplete(AchievementCode achievementCode) const
{
    ProcessAchievementInfo* achievementInfo = helper_.getProcessMissionInfo(achievementCode);
    if (! achievementInfo) {
        return false;
    }

    const gdt::achievement_t* achievementTemplate = 
        ACHIEVEMENT_TABLE->getAchievement(achievementCode);
    if (! achievementTemplate) {
        return false;
    }
    return checkComplete(*achievementInfo, *achievementTemplate);
}


bool AchievementMission::shouldSkipMission(const gdt::achievement_t& /*achievement*/, go::Entity* /*target*/) const
{
    return false;
}


bool AchievementMission::shouldSkipMission(const gdt::achievement_t& /*achievement*/, uint64_t /*param1*/, uint64_t /*param2*/) const
{
    return false;
}



void AchievementMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    updateMission_i(achievementCode, 1, 0, true);
}


void AchievementMission::updateMission_i(AchievementCode achievementCode, uint64_t param1, uint64_t param2, bool isAdd)
{
    ProcessAchievementInfo* achievementInfo = helper_.getProcessMissionInfo(achievementCode);
    if (achievementInfo) {
        if (isAdd) {
            achievementInfo->param1_ += param1;
        }
        else {
            if (achievementInfo->param1_ == param1) {
                return;
            }
            achievementInfo->param1_ = param1;
        }
        achievementInfo->param2_ = param2;
        achievementInfo->lastUpdateTime_ = getTime();
    }
    else {
        helper_.addProcessAchievementInfo(achievementCode, param1, param2);
    }
    achievementInfo = helper_.getProcessMissionInfo(achievementCode);
    assert(achievementInfo);
    owner_.getController().queryAchievementCallback()->achievementUpdated(achievementCode, *achievementInfo);  
}


// = AchievementBuddyMission
bool AchievementBuddyMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_1() <= achievementInfo.param1_;
}


void AchievementBuddyMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    updateMission_i(achievementCode, param1_, 0, false);
}


// = AchievementEquipMission
bool AchievementInventoryMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* /*target*/) const
{
    if (1 == achievement.param_1()) {

    }
    else if (2 == achievement.param_1()) {

    }
    assert(false);
    return true;
}


bool AchievementInventoryMission::checkComplete(const ProcessAchievementInfo& /*achievementInfo*/, const gdt::achievement_t& /*achievementTemplate*/) const
{
    return true;
}


// = AchievementGameMoneyMission
bool AchievementGameMoneyMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_1() <= achievementInfo.param1_;
}


void AchievementGameMoneyMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    updateMission_i(achievementCode, static_cast<uint64_t>(owner_.queryMoneyable()->getBankGameMoney()), 0, false);     
}


// = AchievementChaoMission
bool AchievementChaoMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_1() <= achievementInfo.param1_;
}


void AchievementChaoMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    updateMission_i(achievementCode, static_cast<uint64_t>(owner_.queryChaoable()->getChaotic()), 0, false);    
}


// = AchievementEquipMission
bool AchievementEquipMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* /*target*/) const
{
    itemGrade_ = toItemGrade(achievement.param_1());
    if (! isValid(itemGrade_)) {
        return false;
    }
    return owner_.queryInventoryable()->getEquipInventory().getEquipCountByItemGrade(itemGrade_) == 0;
}


bool AchievementEquipMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_2() <= achievementInfo.param1_;
}


void AchievementEquipMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    const uint64_t equipCount = owner_.queryInventoryable()->getEquipInventory().getEquipCountByItemGrade(itemGrade_);
    updateMission_i(achievementCode, equipCount, 0, false);    
}


// = AchievementStoreMission
bool AchievementStoreMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_3() <= achievementInfo.param1_;
}


// = AchievementExecutionMission
bool AchievementExecutionMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_1() <= achievementInfo.param1_;
}


// = AchievementLevelUpMission
bool AchievementLevelUpMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* /*target*/) const
{    
    CreatureLevel currentLevel = static_cast<go::Creature&>(owner_).getCreatureLevel();
    return toCreatureLevel(achievement.param_1()) > currentLevel;
}

bool AchievementLevelUpMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_2() <= achievementInfo.param1_;
}


void AchievementLevelUpMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    CreatureLevel currentLevel = static_cast<go::Creature&>(owner_).getCreatureLevel();
    updateMission_i(achievementCode, currentLevel, 0, false);        
}


// = AchievementTreasureMission
bool AchievementTreasureMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* target) const
{    
    const TreasureCode treasureCode = achievement.param_1();
    return isValidTreasureCode(treasureCode) && treasureCode != target->getEntityCode();
}


bool AchievementTreasureMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_1() <= achievementInfo.param1_;
}


// = AchievementHunterMission
bool AchievementHunterMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* target) const
{
    assert(target);

    const NpcType npcType = toNpcType(achievement.param_1());
    const NpcType victimeNpcType = getNpcType(target->getEntityCode());
    if (isMonster(npcType) && isMonster(victimeNpcType)) {
        return false;
    }

    if (isValid(npcType) && npcType != victimeNpcType) {
        return true;
    }

    const NpcCode npcCode = achievement.param_2();
    if (isValidNpcCode(npcCode) && npcCode != target->getEntityCode()) {
        return true;
    }
    return false;
}


bool AchievementHunterMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_3() <= achievementInfo.param1_;
}

// = AchievementQuestMission
bool AchievementQuestMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_1() <= achievementInfo.param1_;
}


void AchievementQuestMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    updateMission_i(achievementCode, static_cast<uint64_t>(owner_.queryQuestable()->getQuestRepository().getCompleteQuestCount()), 0, false);    
}


// = AchievementPVPKillMission
bool AchievementPVPKillMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* target) const
{
    MapCode mapCode = static_cast<MapCode>(achievement.param_1());
    MapCode ownerMapCode = owner_.getCurrentWorldMap()->getMapCode();

    ProcessAchievementInfo* achievementInfo = helper_.getProcessMissionInfo(achievement.achievement_code());
    if (achievementInfo && achievementInfo->param2_ == target->getObjectId()) {
        return true;
    }

    if (! isValidMapCode(mapCode)) {
        // 현재는 투기장만
        if (isArenaMap(getMapType(ownerMapCode))) {
            return true;
        }      
        return false;
    }

    return mapCode != ownerMapCode;    
}


bool AchievementPVPKillMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_4() <= achievementInfo.param1_;
}


void AchievementPVPKillMission::updateMission(AchievementCode achievementCode, go::Entity* target)
{
    const gdt::achievement_t* achievement = 
        ACHIEVEMENT_TABLE->getAchievement(achievementCode);
    if (! achievement) {
        return;
    }

    if (target->isPlayer()) {
        const datatable::AchievementTable::Achievements& achievements = 
            ACHIEVEMENT_TABLE->getAchievements(acmtPvPDeath);
        for (const gdt::achievement_t* deathAchievement : achievements) {
            const AchievementCode deathAchievementCode = deathAchievement->achievement_code();
            if (helper_.isProcessed(deathAchievementCode)) {
                const bool isConsecutively = deathAchievement->param_2() != 0;
                if (isConsecutively) {
                    updateMission_i(deathAchievementCode, 0, 0, false);
                }                
            }
        }
    }

    const bool isDayMission = achievement->param_1() != 0;
    if (isDayMission) {
        ProcessAchievementInfo* achievementInfo = helper_.getProcessMissionInfo(achievementCode);
        const sec_t nowStartTime = getDayStartSec(getTime());
        sec_t lastUpdateStartTime = getTime();
        if (achievementInfo) {
            lastUpdateStartTime = getDayStartSec(achievementInfo->lastUpdateTime_);
        }  
        updateMission_i(achievementCode, 1, target->getObjectId(), lastUpdateStartTime == nowStartTime);                   
    }
    else {
        updateMission_i(achievementCode, 1, target->getObjectId(), true);
    }    
}

// = AchievementPVPDeathMission
bool AchievementPVPDeathMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* target) const
{
    MapCode mapCode = static_cast<MapCode>(achievement.param_1());
    MapCode ownerMapCode = owner_.getCurrentWorldMap()->getMapCode();

    ProcessAchievementInfo* achievementInfo = helper_.getProcessMissionInfo(achievement.achievement_code());
    if (achievementInfo && achievementInfo->param2_ == target->getObjectId()) {
        return true;
    }

    if (! isValidMapCode(mapCode)) {
        // 현재는 투기장만
        if (isArenaMap(getMapType(ownerMapCode))) {
            return true;
        }      
        return false;
    }

    return mapCode != ownerMapCode;    
}


bool AchievementPVPDeathMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_4() <= achievementInfo.param1_;
}


void AchievementPVPDeathMission::updateMission(AchievementCode achievementCode, go::Entity* target)
{
    const gdt::achievement_t* achievement = 
        ACHIEVEMENT_TABLE->getAchievement(achievementCode);
    if (! achievement) {
        return;
    }

    if (target->isPlayer()) {
        const datatable::AchievementTable::Achievements& achievements = 
            ACHIEVEMENT_TABLE->getAchievements(acmtPvPKill);
        for (const gdt::achievement_t* killAchievement : achievements) {
            const AchievementCode killAchievementCode = killAchievement->achievement_code();
            if (helper_.isProcessed(killAchievementCode)) {
                const bool isConsecutively = killAchievement->param_2() != 0;
                if (isConsecutively) {
                    updateMission_i(killAchievementCode, 0, 0, false);
                }
            }
        }
    }

    const bool isDayMission = achievement->param_1() != 0;
    if (isDayMission) {
        ProcessAchievementInfo* achievementInfo = helper_.getProcessMissionInfo(achievementCode);
        const sec_t nowStartTime = getDayStartSec(getTime());
        sec_t lastUpdateStartTime = getTime();
        if (achievementInfo) {
            lastUpdateStartTime = getDayStartSec(achievementInfo->lastUpdateTime_);
        }     
        updateMission_i(achievementCode, 1, target->getObjectId(), lastUpdateStartTime == nowStartTime);                   
    }
    else {
        updateMission_i(achievementCode, 1, target->getObjectId(), true);
    }    
}



// = AchievementPvPWinMission
bool AchievementPvPWinMission::shouldSkipMission(const gdt::achievement_t& achievement, go::Entity* /*target*/) const
{
    MapCode mapCode = static_cast<MapCode>(achievement.param_1());
    MapCode ownerMapCode = owner_.getCurrentWorldMap()->getMapCode();

    if (! isValidMapCode(mapCode)) {
        // 현재는 투기장만
        if (isArenaMap(getMapType(ownerMapCode))) {
            return true;
        }
        return false;
    }

    return mapCode != ownerMapCode;    
}


bool AchievementPvPWinMission::checkComplete(const ProcessAchievementInfo& achievementInfo, const gdt::achievement_t& achievementTemplate) const
{
    return achievementTemplate.param_4() <= achievementInfo.param1_;
}


void AchievementPvPWinMission::updateMission(AchievementCode achievementCode, go::Entity* /*target*/)
{
    const gdt::achievement_t* achievement = 
        ACHIEVEMENT_TABLE->getAchievement(achievementCode);
    if (! achievement) {
        return;
    }

    const bool isDayMission = achievement->param_1() != 0;
    if (isDayMission) {
        ProcessAchievementInfo* achievementInfo = helper_.getProcessMissionInfo(achievementCode);
        const sec_t nowStartTime = getDayStartSec(getTime());
        sec_t lastUpdateStartTime = getTime();
        if (achievementInfo) {
            lastUpdateStartTime = getDayStartSec(achievementInfo->lastUpdateTime_);
        }     
        updateMission_i(achievementCode, 1, 0, lastUpdateStartTime == nowStartTime);                   
    }
    else {
        updateMission_i(achievementCode, 1, 0, true);
    }        
}

}} // namespace gideon { namespace zoneserver {