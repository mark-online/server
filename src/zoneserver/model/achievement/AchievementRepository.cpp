#include "ZoneServerPCH.h"
#include "AchievementRepository.h"
#include "AchievementMission.h"
#include "../gameobject/Player.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/AchievementCallback.h"
#include "../../controller/callback/CharacterTitleCallback.h"
#include "../../helper/InventoryHelper.h"
#include "../../service/item/ItemIdGenerator.h"
#include "../../service/mail/MailService.h"
#include <gideon/cs/datatable/AchievementTable.h>

namespace gideon { namespace zoneserver {


AchievementRepository::AchievementRepository(go::Player& owner, const ProcessAchievementInfoMap& processAchievementInfoMap,
    const CompleteAchievementInfoMap& completeAchievementInfoMap) :
    owner_(owner),
    processAchievementInfoMap_(processAchievementInfoMap),
    completeAchievementInfoMap_(completeAchievementInfoMap)
{
    missionMap_.emplace(acmtLevel, new AchievementLevelUpMission(*this, owner));
    missionMap_.emplace(acmtAddBuddy, new AchievementBuddyMission(*this, owner));
    missionMap_.emplace(acmtInventory, new AchievementInventoryMission(*this, owner));
    missionMap_.emplace(acmtBankGameMoney, new AchievementGameMoneyMission(*this, owner));
    missionMap_.emplace(acmtChao, new AchievementChaoMission(*this, owner));
    missionMap_.emplace(acmtTreasure, new AchievementTreasureMission(*this, owner));
    missionMap_.emplace(acmtEquip, new AchievementEquipMission(*this, owner));
    missionMap_.emplace(acmtStore, new AchievementStoreMission(*this, owner));
    missionMap_.emplace(acmtExecution, new AchievementExecutionMission(*this, owner));
    missionMap_.emplace(acmtHunter, new AchievementHunterMission(*this, owner));
    missionMap_.emplace(acmtQuest, new AchievementQuestMission(*this, owner));

    missionMap_.emplace(acmtPvPDeath, new AchievementPVPDeathMission(*this, owner));
    missionMap_.emplace(acmtPvPKill, new AchievementPVPKillMission(*this, owner));
    missionMap_.emplace(acmtPvPWin, new AchievementPvPWinMission(*this, owner));

}


AchievementRepository::~AchievementRepository()
{
    processAchievementInfoMap_.clear();
    completeAchievementInfoMap_.clear();

    for (MissionMap::value_type value : missionMap_) {
        delete value.second;
    }
}


void AchievementRepository::update(AchievementMissionType missionType, go::Entity* target)
{
    MissionMap::iterator pos = missionMap_.find(missionType);
    if (pos != missionMap_.end()) {
        (*pos).second->update(missionType, target);
    }
    
    updateCompleteAchievement(missionType);
}


void AchievementRepository::update(AchievementMissionType missionType, uint64_t param1, uint64_t param2)
{
    MissionMap::iterator pos = missionMap_.find(missionType);
    if (pos != missionMap_.end()) {
        (*pos).second->update(missionType, param1, param2);
    }

    updateCompleteAchievement(missionType);
}


bool AchievementRepository::isCompleted(AchievementCode code) const
{
    return completeAchievementInfoMap_.find(code) != completeAchievementInfoMap_.end();
}


bool AchievementRepository::isProcessed(AchievementCode code) const
{
    return processAchievementInfoMap_.find(code) != processAchievementInfoMap_.end();
} 


ProcessAchievementInfo* AchievementRepository::getProcessMissionInfo(AchievementCode code)
{
    ProcessAchievementInfoMap::iterator pos = processAchievementInfoMap_.find(code);
    if (pos != processAchievementInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void AchievementRepository::addProcessAchievementInfo(AchievementCode code, uint64_t param1, uint64_t param2)
{
    sec_t now = getTime();
    processAchievementInfoMap_.emplace(code, ProcessAchievementInfo(param1, param2, now));
}


void AchievementRepository::updateCompleteAchievement(AchievementMissionType missionType)
{
    const datatable::AchievementTable::Achievements& achievements = ACHIEVEMENT_TABLE->getAchievements(missionType);
    for (const gdt::achievement_t* achievement : achievements) {
        const AchievementCode achievementCode = achievement->achievement_code();
        if (isCompleted(achievementCode) || ! isProcessed(achievementCode)) {
            continue;
        }
        MissionMap::iterator pos = missionMap_.find(missionType);
        if (pos == missionMap_.end()) {
            continue;
        }
        AchievementMission* mission = (*pos).second;
        if (mission->isComplete(achievementCode)) {
            completeAchievement(*achievement);
        }         
    }
}


void AchievementRepository::completeAchievement(const gdt::achievement_t& achievement)
{
    // 보상 처리및 완료
    const AchievementCode achievementCode = achievement.achievement_code();
    const sec_t now = getTime();
    processAchievementInfoMap_.erase(achievementCode);
    completeAchievementInfoMap_.insert(CompleteAchievementInfoMap::value_type(achievementCode,
        CompleteAchievementInfo(achievementCode, now)));
    owner_.upAchievementPoint(achievement.reward_achievement_point());
    owner_.getController().queryAchievementCallback()->achievementCompleted(achievementCode, owner_.getAchievementPoint(), now);

    DataCode rewardCode1 = achievement.reward_code_1();
    if (isValidDataCode(rewardCode1)) {
        reward(achievementCode, rewardCode1, achievement.reward_value_1());
    }
    DataCode rewardCode2 = achievement.reward_code_2();
    if (isValidDataCode(rewardCode2)) {
        reward(achievementCode, rewardCode2, achievement.reward_value_2());
    }
}


void AchievementRepository::reward(AchievementCode achievementCode, DataCode dataCode, uint64_t value)
{
    CodeType ct = getCodeType(dataCode);
    if (isCharacterTitleType(ct)) {
        owner_.getController().queryCharacterTitleCallback()->characterTitleAdded(dataCode);
    }
    else if (isItemType((ct))) {
        InventoryInfo inventoryInfo;
        AddItemInfo addItemInfo = createAddItemInfoByBaseItemInfo(BaseItemInfo(dataCode, static_cast<uint8_t>(value)));
        ItemInfo itemInfo(addItemInfo.itemCode_, addItemInfo.count_,
            ITEM_ID_GENERATOR->generateItemId(), 1, addItemInfo.expireTime_);

        if (addItemInfo.isAccessory()) {
            itemInfo.accessoryItemInfo_ = addItemInfo.accessoryItemInfo_;
        }
        else if (addItemInfo.isEquipment()) {
            itemInfo.equipItemInfo_ = addItemInfo.equipItemInfo_;
        }
        inventoryInfo.addItem(itemInfo);

        MAIL_SERVICE->sendMailFromAchievement(owner_.getObjectId(), achievementCode, inventoryInfo);
    }
}

}} // namespace gideon { namespace zoneserver {