#pragma once

#include <gideon/server/data/CreateCharacterInfo.h>
#include <gideon/server/data/CharacterStats.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/QuestInfo.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace databaseproxyserver {

/**
 * @class AccountCache
 *
 * 사용자 계정과 관련된 정보 Cache
 */
class AccountCache : public boost::noncopyable
{
public:
    virtual ~AccountCache() {}

    virtual ErrorCode initialize(AccountId accountId) = 0;

    virtual void finalize() = 0;

    virtual void removeOldCompleteRepeatQuest() = 0;

    virtual ErrorCode createCharacter(FullCharacterInfo& characterInfo,
        const CreateCharacterInfo& createCharacterInfo, const CreateCharacterEquipments& createCharacterEquipments,
		ZoneId zoneId, const ObjectPosition& position) = 0;

    virtual ErrorCode deleteCharacter(GuildId& guildId, ObjectId characterId) = 0;

    virtual void saveCharacterStats(ObjectId characterId,
        const DBCharacterStats& saveInfo) = 0;
    virtual void saveSelectCharacterTitle(ObjectId characterId, CharacterTitleCode titileCode) = 0;

    virtual ErrorCode getCharacterProperties(std::string& config, std::string& prefs,
        ObjectId characterId) = 0;
    virtual void saveCharacterProperties(ObjectId characterId,
        const std::string& config, const std::string& prefs) = 0;

    virtual ObjectIdSet getCharacterIds() const = 0;

public:
    virtual void moveInventoryItem(ObjectId characterId,
        ObjectId itemId, SlotId slotId, InvenType invenType) = 0;
    virtual void switchInventoryItem(ObjectId characterId,
        ObjectId itemId1, ObjectId itemId2, InvenType invenType) = 0;
    virtual void addInventoryItem(ObjectId characterId, const ItemInfo& itemInfo, InvenType invenType) = 0;
    virtual void changeEquipItemInfo(ObjectId characterId, ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount) = 0;
    virtual void enchantEquipItem(ObjectId characterId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo) = 0;
	virtual void unenchantEquipItem(ObjectId characterId, ObjectId itemId, SocketSlotId slotId) = 0;

    virtual void removeInventoryItem(ObjectId characterId,ObjectId itemId, InvenType invenType) = 0;
    virtual void updateInventoryItemCount(ObjectId characterId,
        ObjectId itemId, uint8_t itemCount, InvenType invenType) = 0;
    virtual void addQuestItem(ObjectId characterId, const QuestItemInfo& questItemInfo) = 0;	
    virtual void removeQuestItem(ObjectId characterId, ObjectId itemId) = 0;
    virtual void updateQuestItemUsableCount(ObjectId characterId, ObjectId itemId, uint8_t usableCount) = 0;
	virtual void updateQuestItemCount(ObjectId characterId, ObjectId itemId, uint8_t usableCount) = 0;

    virtual void equipItem(ObjectId characterId,
        ObjectId itemId, EquipPart equipPart) = 0;
    virtual void unequipItem(ObjectId characterId,
        ObjectId itemId, SlotId slotId, EquipPart unequipPart) = 0;
    virtual void replaceInventoryWithEquipItem(ObjectId characterId,
        ObjectId unequipItemId, EquipPart unequipPart, ObjectId equipItemId,
        EquipPart equipPart) = 0;

    virtual void equipAccessoryItem(ObjectId characterId,
        ObjectId itemId, AccessoryIndex equipPart) = 0;
    virtual void unequipAccessoryItem(ObjectId characterId,
        ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart) = 0;
    virtual void replaceInventoryWithAccessoryItem(ObjectId characterId,
        ObjectId unequipItemId, AccessoryIndex unequipPart, ObjectId equipItemId,
        AccessoryIndex equipPart) = 0;

    virtual void changeCharacterState(ObjectId characterId,
        CreatureStateType state) = 0;

    virtual void saveActionBar(ObjectId characterId, 
        ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code) = 0;

    virtual void learnSkill(ObjectId characterId, 
        SkillCode currentSkillCode, SkillCode learnSkillCode) = 0;
    virtual void removeSkill(ObjectId characterId, SkillCode skillCode) = 0;
    virtual void removeAllSkill(ObjectId characterId) = 0;

	virtual void acceptQuest(ObjectId characterId, QuestCode questCode) = 0;
    virtual void acceptRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptRepeatTime) = 0;
	virtual void cancelQuest(ObjectId characterId, QuestCode questCode, bool isRepeatQuest) = 0;
	virtual void completeQuest(ObjectId characterId, QuestCode questCode, bool isRepeatQuest) = 0;
	virtual void updateQuestMission(ObjectId characterId, QuestCode questCode,
		QuestMissionCode missionCode, const QuestGoalInfo& goalInfo, bool isRepeatQuest) = 0;
	virtual void removeCompleteQuest(ObjectId characterId, QuestCode questCode) = 0;

    virtual void setPlayerGuildInfo(GuildId guildId, const GuildMemberInfo& memberInfo,
        bool dbSave = false) = 0;
    virtual void leaveGuild(ObjectId characterId) = 0;

    virtual void updateArenaRecord(ObjectId characterId, ArenaModeType arenaMode,
        uint32_t resultScore, ArenaResultType resultType) = 0;
    virtual void updateArenaPoint(ObjectId characterId, ArenaPoint arenaPoint) = 0;
    virtual void updateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime) = 0;

    virtual void deleteAccountExpireItems() = 0;
    
    virtual void addBindRecallInfo(ObjectId characterId, const BindRecallInfo& bindRecallInfo) = 0;
    virtual void removeBindRecallInfo(ObjectId characterId, ObjectId linkId) = 0;

    virtual void releaseBeginnerProtection(ObjectId characterId) = 0;

    virtual void addCooldownInfos(ObjectId characterId, const CooltimeInfos& infos) = 0;
    virtual void addRemainEffects(ObjectId characterId, const RemainEffectInfos& infos) = 0;

    virtual void updateCharacterInventoryInfo(ObjectId characterId, InvenType invenType, bool isCashSlot, uint8_t count) = 0;

	virtual void createVehicle(ObjectId characterId, const VehicleInfo& info) = 0;
	virtual void createGlider(ObjectId characterId, const GliderInfo& info) = 0;
	virtual void deleteVehicle(ObjectId characterId, ObjectId id) = 0;
	virtual void deleteGlider(ObjectId characterId, ObjectId id) = 0;
	virtual void selectVehicle(ObjectId characterId, ObjectId id) = 0;
	virtual void selectGlider(ObjectId characterId, ObjectId id) = 0;
	virtual void updateGliderDurability(ObjectId characterId, ObjectId id, uint32_t currentValue) = 0;

    virtual void addCharacterTitle(ObjectId characterId, CharacterTitleCode titleCode) = 0;
    virtual void updateProcessAchievement(ObjectId characterId, AchievementCode code, const ProcessAchievementInfo& missionInfo) = 0;
    virtual void completeAchievement(ObjectId characterId, AchievementCode code, AchievementPoint point) = 0;

    virtual ErrorCode queryAchievements(ProcessAchievementInfoMap& processInfoMap, CompleteAchievementInfoMap& completeInfoMap, ObjectId characterId) = 0;
    virtual ErrorCode queryCharacterTitles(CharacterTitleCodeSet& titleCodeSet, ObjectId characterId) = 0;

public:
    virtual FullUserInfo getFullUserInfo() const = 0;

    virtual size_t getCharacterCount() const = 0;

public:
    /// cache hitted
    virtual void touched() = 0;

    /// 캐시를 회수해야 하는가?
    virtual bool shouldCollect(uint16_t oldCacheMinAccessPeriod) const = 0;
};

}} // namespace gideon { namespace databaseproxyserver {
