#pragma once

#include "../AccountCache.h"

namespace gideon { namespace databaseproxyserver {

/**
 * @class AccountCacheImpl
 *
 * AccountCache 실제 구현
 */
class AccountCacheImpl : public AccountCache
{
    typedef std::mutex LockType;

    typedef sne::core::HashMap<ObjectId, CreatureStatusInfo> CreatureStatusInfoMap;
    typedef sne::core::HashMap<ObjectId, ProcessAchievementInfoMap> AccountProcessAchievementInfoMap;
    typedef sne::core::HashMap<ObjectId, CompleteAchievementInfoMap> AccountCompleteAchievementInfoMap;
    typedef sne::core::HashMap<ObjectId, CharacterTitleCodeSet> AccountCharacterTitleCodeSet;

    struct Property
    {
        std::string config_;
        std::string prefs_;

        explicit Property(const std::string& config = "", const std::string& prefs = "") :
            config_(config),
            prefs_(prefs) {}
    };
    typedef sne::core::HashMap<ObjectId, Property> PropertyMap;

public:
    AccountCacheImpl();
    virtual ~AccountCacheImpl();

private:
    virtual ErrorCode initialize(AccountId daccountId);
    virtual void finalize();
    
    virtual void removeOldCompleteRepeatQuest();

    virtual ErrorCode createCharacter(FullCharacterInfo& characterInfo,
        const CreateCharacterInfo& createCharacterInfo, const CreateCharacterEquipments& createCharacterEquipments,
		ZoneId zoneId, const ObjectPosition& position);

    virtual ErrorCode deleteCharacter(GuildId& guildId, ObjectId characterId);

    virtual void saveCharacterStats(ObjectId characterId,
        const DBCharacterStats& saveInfo);
    virtual void saveSelectCharacterTitle(ObjectId characterId, CharacterTitleCode titileCode);

    virtual ErrorCode getCharacterProperties(std::string& config, std::string& prefs,
        ObjectId characterId);
    virtual void saveCharacterProperties(ObjectId characterId,
        const std::string& config, const std::string& prefs);

    virtual ObjectIdSet getCharacterIds() const;

    virtual void moveInventoryItem(ObjectId characterId,
        ObjectId itemId, SlotId slotId, InvenType invenType);
    virtual void switchInventoryItem(ObjectId characterId,
        ObjectId itemId1, ObjectId itemId2, InvenType invenType);
    virtual void addInventoryItem(ObjectId characterId, const ItemInfo& itemInfo, InvenType invenType);
    virtual void changeEquipItemInfo(ObjectId characterId, ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount);
    virtual void enchantEquipItem(ObjectId characterId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo);
    virtual void unenchantEquipItem(ObjectId characterId, ObjectId itemId, SocketSlotId slotId);
    
    virtual void removeInventoryItem(ObjectId characterId,ObjectId itemId, InvenType invenType);
    virtual void updateInventoryItemCount(ObjectId characterId,
        ObjectId itemId, uint8_t itemCount, InvenType invenType);
    virtual void addQuestItem(ObjectId characterId, const QuestItemInfo& questItemInfo);	
    virtual void removeQuestItem(ObjectId characterId, ObjectId itemId);
    virtual void updateQuestItemUsableCount(ObjectId characterId, ObjectId itemId, uint8_t usableCount);
	virtual void updateQuestItemCount(ObjectId characterId, ObjectId itemId, uint8_t usableCount);

    virtual void equipItem(ObjectId characterId,
        ObjectId itemId, EquipPart equipPart);
    virtual void unequipItem(ObjectId characterId,
        ObjectId itemId, SlotId slotId, EquipPart unequipPart);
    virtual void replaceInventoryWithEquipItem(ObjectId characterId,
        ObjectId unequipItemId, EquipPart unequipPart, ObjectId equipItemId,
        EquipPart equipPart);

    virtual void equipAccessoryItem(ObjectId characterId,
        ObjectId itemId, AccessoryIndex equipPart);
    virtual void unequipAccessoryItem(ObjectId characterId,
        ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart);
    virtual void replaceInventoryWithAccessoryItem(ObjectId characterId,
        ObjectId unequipItemId, AccessoryIndex unequipPart, ObjectId equipItemId,
        AccessoryIndex equipPart);

    virtual void changeCharacterState(ObjectId characterId,
        CreatureStateType state);

    virtual void saveActionBar(ObjectId characterId, 
        ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code);

    virtual void learnSkill(ObjectId characterId, 
        SkillCode currentSkillCode, SkillCode learnSkillCode);
    virtual void removeSkill(ObjectId characterId, SkillCode skillCode);
    virtual void removeAllSkill(ObjectId characterId);

	virtual void acceptQuest(ObjectId characterId, QuestCode questCode);
    virtual void acceptRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptRepeatTime);
	virtual void cancelQuest(ObjectId characterId, QuestCode questCode, bool isRepeatQuest);
	virtual void completeQuest(ObjectId characterId, QuestCode questCode, bool isRepeatQuest);
	virtual void updateQuestMission(ObjectId characterId, QuestCode questCode,
		QuestMissionCode missionCode, const QuestGoalInfo& goalInfo, bool isRepeatQuest);
	virtual void removeCompleteQuest(ObjectId characterId, QuestCode questCode);

    virtual void setPlayerGuildInfo(GuildId guildId, const GuildMemberInfo& memberInfo, bool dbSave);
    virtual void leaveGuild(ObjectId characterId);

private:
    virtual FullUserInfo getFullUserInfo() const;    

    virtual size_t getCharacterCount() const;

    void cancelQuest(ObjectId characterId, QuestCode questCode);
    void completeQuest(ObjectId characterId, QuestCode questCode);
    void updateQuestMission(ObjectId characterId, QuestCode questCode,
        QuestMissionCode missionCode, const QuestGoalInfo& goalInfo);

    void cancelRepeatQuest(ObjectId characterId, QuestCode questCode);
    void completeRepeatQuest(ObjectId characterId, QuestCode questCode);
    void updateRepeatQuestMission(ObjectId characterId, QuestCode questCode,
        QuestMissionCode missionCode, const QuestGoalInfo& goalInfo); 

    virtual void updateArenaRecord(ObjectId characterId, ArenaModeType arenaMode,
        uint32_t resultScore, ArenaResultType resultType);
    virtual void updateArenaPoint(ObjectId characterId, ArenaPoint arenaPoint);
    virtual void updateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime);    

    virtual void addBindRecallInfo(ObjectId characterId, const BindRecallInfo& bindRecallInfo);
    virtual void removeBindRecallInfo(ObjectId characterId, ObjectId linkId);

    virtual void releaseBeginnerProtection(ObjectId characterId);
    virtual void addCooldownInfos(ObjectId characterId, const CooltimeInfos& infos);
    virtual void addRemainEffects(ObjectId characterId, const RemainEffectInfos& infos);

    virtual void updateCharacterInventoryInfo(ObjectId characterId, InvenType invenType, bool isCashSlot, uint8_t count);

	virtual void createVehicle(ObjectId characterId, const VehicleInfo& info);
	virtual void createGlider(ObjectId characterId, const GliderInfo& info);
	virtual void deleteVehicle(ObjectId characterId, ObjectId id);
	virtual void deleteGlider(ObjectId characterId, ObjectId id);
	virtual void selectVehicle(ObjectId characterId, ObjectId id);
	virtual void selectGlider(ObjectId characterId, ObjectId id);
	virtual void updateGliderDurability(ObjectId characterId, ObjectId id, uint32_t currentValue);

    virtual void addCharacterTitle(ObjectId characterId, CharacterTitleCode titleCode);
    virtual void updateProcessAchievement(ObjectId characterId, AchievementCode code, const ProcessAchievementInfo& missionInfo);
    virtual void completeAchievement(ObjectId characterId, AchievementCode code, AchievementPoint point);

    virtual ErrorCode queryAchievements(ProcessAchievementInfoMap& processInfoMap, CompleteAchievementInfoMap& completeInfoMap, ObjectId characterId);
    virtual ErrorCode queryCharacterTitles(CharacterTitleCodeSet& titleCodeSet, ObjectId characterId);

private:
    virtual void touched() {
        //std::unique_lock<std::mutex> lock(lock_);
        accessedAt_ = time(0);
    }

    virtual bool shouldCollect(uint16_t oldCacheMinAccessPeriod) const {
        return (time(0) - accessedAt_) >= oldCacheMinAccessPeriod;
    }

    virtual void deleteAccountExpireItems();
    void deleteCharacterExpireItems(FullCharacterInfo& character);
    
private:
    void reset();

private:
    FullUserInfo userInfo_; ///< 사용자 정보
    AccountProcessAchievementInfoMap accountProcessAchievementInfoMap_;
    AccountCompleteAchievementInfoMap accountCompleteAchievementInfoMap_;
    AccountCharacterTitleCodeSet accountCharacterTitleCodeSet_;
    
    PropertyMap propertyMap_;

    mutable time_t accessedAt_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace databaseproxyserver {
