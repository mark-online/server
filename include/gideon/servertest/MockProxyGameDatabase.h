#pragma once

#include <gideon/Common.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/servertest/MockConnection.h>
#include <sne/test/CallCounter.h>

namespace gideon { namespace servertest {

/**
* @class MockProxyGameDatabase
*
* 테스트 용 GameDatabase.
*/
class GIDEON_SERVER_API MockProxyGameDatabase :
    public serverbase::ProxyGameDatabase,
    public sne::test::CallCounter
{
    typedef sne::core::HashMap<Nickname, ObjectId> NicknameMap;
public:
    MockProxyGameDatabase();

    void setValidAccount(AccountId accountId,
        const UserId& userId, const std::string& password);

    ObjectId getLastCreatedCharacterId() const {
        return lastCreatedCharacterId_;
    }

	const FullUserInfo getFullUserInfo(AccountId accountId) const;

    virtual void registGameMailCallback(serverbase::GameMailCallback& gameMailCallback);
    virtual void registDBQueryPlayerCallback(serverbase::DBQueryPlayerCallback& playerCallback);
	virtual void registBuddyCallback(serverbase::CommunityBuddyCallback& buddyCallback);

public:
    static UserId makeUserId(AccountId accountId);
    static Nickname makeNickname(AccountId accountId);

public:
    virtual sne::base::Future::Ref getProperties();
    virtual sne::base::Future::Ref getServerSpec(
        const std::string& serverName, const std::string& suffix);
    virtual sne::base::Future::Ref getMaxInventoryId(ZoneId zoneId);

    virtual sne::base::Future::Ref getShardInfo(ShardId shardId);

    virtual sne::base::Future::Ref getZoneInfo(const std::string& name);
    virtual sne::base::Future::Ref getZoneInfoList(ShardId shardId);

    virtual sne::base::Future::Ref getWorldTime(ZoneId zoneId);
    virtual bool asyncUpdateWorldTime(ZoneId zoneId, WorldTime worldTime);

    virtual sne::base::Future::Ref asyncAuthenticate(
        const UserId& userId, const UserPassword& password);
    virtual sne::base::Future::Ref asyncGetFullUserInfo(const AccountInfo& accountInfo);
    virtual bool asyncUpdateLoginAt(AccountId accountId);

    virtual sne::base::Future::Ref asyncCreateCharacter(const CreateCharacterInfo& createCharacterInfo,
        const CreateCharacterEquipments& createCharacterEquipments, ZoneId zoneId, const ObjectPosition& position);
    virtual sne::base::Future::Ref asyncDeleteCharacter(AccountId accountId,
        ObjectId characterId);
    virtual sne::base::Future::Ref asyncCheckDuplicateNickname(AccountId accountId,
        const Nickname& nickname);
    virtual void asyncSaveCharacterStats(AccountId accountId, ObjectId characterId,
        const DBCharacterStats& saveInfo);
    virtual void asyncSaveSelectCharacterTitle(AccountId accountId, ObjectId characterId,
        CharacterTitleCode titleCode);

    virtual sne::base::Future::Ref asyncLoadCharacterProperties(AccountId accountId,
        ObjectId characterId);
    virtual void asyncSaveCharacterProperties(AccountId accountId, ObjectId characterId,
        const std::string& config, const std::string& prefs);

    virtual void asyncMoveInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId, SlotId slotId);
    virtual void asyncSwitchInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId1, ObjectId itemId2);
    virtual void asyncAddInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, const ItemInfo& itemInfo);
    
    virtual void asyncChangeEquipItemInfo(AccountId accountId, ObjectId characterId, 
        ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount);
    virtual void asyncEnchantEquipItem(AccountId accountId, ObjectId characterId, 
        ObjectId itemId, SocketSlotId id, EquipSocketInfo& socketInfo);
    virtual void asyncUnenchantEquipItem(AccountId accountId, ObjectId characterId, 
        ObjectId itemId, SocketSlotId id);

    virtual void asyncRemoveInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId);
    virtual void asyncUpdateInventoryItemCount(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId1, uint8_t itemCount);
    virtual void asyncAddQuestItem(AccountId accountId, ObjectId characterId,
        const QuestItemInfo& questItemInfo);
    virtual void asyncRemoveQuestItem(AccountId accountId, ObjectId characterId,
        ObjectId questItemId);
    virtual void asyncUpdateQuestItemUsableCount(AccountId accountId, ObjectId characterId,
        ObjectId questItemId, uint8_t usableCount);
	virtual void asyncUpdateQuestItemCount(AccountId accountId, ObjectId characterId,
		ObjectId questItemId, uint8_t stackCount);

    virtual void asyncEquipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, EquipPart equipPart);
    virtual void asyncUnequipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, EquipPart unequipPart);
    virtual void asyncReplaceInventoryWithEquipItem(AccountId accountId,
        ObjectId characterId, ObjectId unequipItemId, EquipPart unequipPart,
        ObjectId equipItemId, EquipPart equipPart);

    virtual void asyncEquipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, AccessoryIndex equipPart);
    virtual void asyncUnequipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart);
    virtual void asyncReplaceInventoryWithAccessoryItem(AccountId accountId,
        ObjectId characterId, ObjectId unequipItemId, AccessoryIndex unequipPart,
        ObjectId equipItemId, AccessoryIndex equipPart);

    virtual void asyncChangeCharacterState(AccountId accountId,
        ObjectId characterId, CreatureStateType state);

    virtual void asyncSaveActionBar(AccountId accountId, ObjectId characterId, 
        ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code);

    virtual void asyncLearnSkill(AccountId accountId,
        ObjectId characterId, SkillCode currentSkillCode, SkillCode learnSkillCode);
    virtual void asyncRemoveSkill(AccountId accountId, ObjectId characterId, 
        SkillCode skillCode);
    virtual void asyncRemoveAllSkill(AccountId accountId, ObjectId characterId);

	virtual void asyncAcceptedQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode);
    virtual void asyncAcceptRepeatQuest(AccountId accountId, ObjectId characterId, 
        QuestCode questCode, sec_t repeatAcceptTime);
	virtual void asyncCancelQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode, bool isRepeatQuest);
	virtual void asyncCompleteQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode, bool isRepeatQuest);
	virtual void asyncUpdateQuestMission(AccountId accountId, ObjectId characterId, 
		QuestCode questCode, QuestMissionCode missionCode, const QuestGoalInfo& goalInfo,
        bool isRepeatQuest);
	virtual void asyncRemoveCompleteQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode);

    /// = 커뮤니티
    virtual sne::base::Future::Ref getMaxGuildId();
    virtual sne::base::Future::Ref getGuildInfos();

    virtual void asyncCreateGuild(const BaseGuildInfo& guildInfo);
    virtual void asyncAddGuildMember(AccountId accountId, GuildId guildId, const GuildMemberInfo& memberInfo);
    virtual void asyncRemoveGuildMember(ObjectId characterId);
    virtual void asyncRemoveGuild(GuildId guildId);
    virtual void asyncAddGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId, GuildRelatioshipType type);
    virtual void asyncRemoveGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId);
	virtual void asyncChangeGuildMemberPosition(ObjectId characterId, GuildMemberPosition position);
    virtual void asyncAddGuildApplicant(ObjectId characterId, GuildId guildId);
    virtual void asyncRemoveGuildApplicant(ObjectId characterId);
    virtual void asyncModifyGuildIntroduction(GuildId guildId, const GuildIntroduction& introduction);
    virtual void asyncModifyGuildNotice(GuildId guildId, const GuildNotice& notice);
    virtual void asyncAddGuildRank(GuildId guildId, const GuildRankInfo& rankInfo, uint8_t vaultCount);
    virtual void asyncAddGuildBankRights(GuildId guildId, GuildRankId guildRankId, VaultId vaultId, const GuildBankVaultRightInfo& bankRankInfo);
    virtual void asyncDeleteGuildRank(GuildId guildId, GuildRankId rankId);
    virtual void asyncSwapGuildRank(GuildId guildId, GuildRankId rankId1, GuildRankId rankId2);
    virtual void asyncUpdateGuildRankName(GuildId guildId, GuildRankId rankId, const GuildRankName& rankName);
    virtual void asyncUpdateGuildRankRights(GuildId guildId, GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay);
    virtual void asyncUpdateGuildBankRights(GuildId guildId, GuildRankId rankId,
        VaultId vaultId, uint32_t rights, uint32_t itemWithdrawalPerDay);
    virtual void asyncUpdateGuildRank(GuildId guildId, ObjectId playerId, GuildRankId rankId);

    virtual void asyncRemoveAllGuildSkills(GuildId guildId);
    virtual void asyncRemoveGuildSkills(GuildId guildId, const SkillCodes& skillCodes);
    virtual void asyncAddGuildSkill(GuildId guildId, SkillCode skillCode);
    virtual void asyncUpdateGuildExpInfo(GuildId guildId, const GuildLevelInfo& guildLevelInfo);

    virtual void asyncMoveGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId, SlotId slotId);
    virtual void asyncSwitchGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId1, SlotId slotId1,
        ObjectId itemId2, SlotId slotId2);
    virtual void asyncAddGuildInventoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo);

    virtual void asyncRemoveGuildInventoryItem(GuildId guildId, ObjectId itemId);
    virtual void asyncUpdateGuildInventoryItemCount(GuildId guildId, ObjectId itemId, uint8_t itemCount);
    virtual void asyncUpdateGuildGameMoney(GuildId guildId, GameMoney gameMoney);
    virtual void asyncUpdateGuildMemberWithdraw(ObjectId playerId, GameMoney dayWithdraw, sec_t resetTime);
    virtual void asyncUpdateGuildVaultName(GuildId guildId, VaultId vaultId, const VaultName& name);
    virtual void asyncAddGuildVault(GuildId guildId, const VaultInfo& vaultInfo);

    virtual void asyncAddGuildEventLog(GuildId guildId, const GuildEventLogInfo& guildEventLog);
    virtual void asyncAddGuildBankEventLog(GuildId guildId, const GuildBankEventLogInfo& guildBankEventLog);
    virtual void asyncAddGuildGameMoneyEventLog(GuildId guildId, const GuildGameMoneyEventLogInfo& guildGameMoneyEventLog);

    virtual void asyncSyncMail(ObjectId characterId);
    virtual void asyncSendMail(const Nickname& reveiverNickname, const MailInfo& maillInfo);
    virtual void asyncSendMail(ObjectId receiverId, const MailInfo& maillInfo);
    virtual void asyncAquireItemsInMail(MailId mailId);
    virtual void asyncDeleteMail(MailId mailId);
    virtual void asyncReadMail(MailId mailId);
    virtual void asyncQueryHasNotMail(ObjectId characterId);

    virtual sne::base::Future::Ref getAuctionInfos(ZoneId zoneId);
    virtual void asyncCreateAuction(ZoneId zoneId, NpcCode npcCode, const FullAuctionInfo& auctionInfo);
    virtual void asyncDeleteAuction(ZoneId zoneId, AuctionId auctionId);
    virtual void asyncUpdateBid(ZoneId zoneId, AuctionId auctionId, ObjectId buyerId, GameMoney currentBidMoney);

    virtual void asyncUpdateArenaRecord(AccountId accountId, ObjectId characterId, ArenaModeType arenaMode,
        uint32_t resultScore, ArenaResultType resultType);
    virtual void asyncUpdateArenaPoint(AccountId accountId, ObjectId characterId, ArenaPoint arenaPoint);
    virtual void asyncUpdateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime);

    virtual sne::base::Future::Ref loadBuildingInfos(ZoneId zoneId);
    virtual void asyncMoveBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, ObjectId itemId, SlotId slotId);
    virtual void asyncSwitchBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, ObjectId itemId1, ObjectId itemId2,
        SlotId slotId1, SlotId slotId2);
    virtual void asyncAddBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, const ItemInfo& itemInfo);
    virtual void asyncAddBuildingInventoryEquipItem(ObjectId buildingId,
        InvenType invenType, const ItemInfo& itemInfo, const EquipItemInfo& equipItemInfo);
    virtual void asyncRemoveBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, ObjectId itemId);
    virtual void asyncUpdateBuildingInventoryItemCount(ObjectId buildingId,
        InvenType invenType, ObjectId itemId1, uint8_t itemCount);
    virtual void asyncRemoveAllBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType);
    
    virtual void asyncAddBuildingGuard(ObjectId buildingId, const BuildingGuardInfo& guardInfo);
    virtual void asyncRemoveBuildingGuard(ObjectId guardId);


    virtual void asyncCreateBuilding(const BuildingInfo& buildingInfo);
    virtual void asyncDeleteBuilding(ObjectId buildingId);
    virtual void asyncUpdateBuildingState(ObjectId buildingId, BuildingStateType state,
        sec_t startBuildTime, sec_t expireTime, HitPoint currentHitPoint);
    virtual void asyncUpdateBuildingOwner(ObjectId buildingId, 
        BuildingOwnerType ownerType, ObjectId characterId, GuildId guildId);

    virtual void asyncAddBindRecall(AccountId accountId, ObjectId characterId, 
        const BindRecallInfo& bindRecallInfo);
    virtual void asyncRemoveBindRecall(AccountId accountId, ObjectId characterId, 
        ObjectId linkId);

    virtual void asyncAddSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
        uint8_t count, sec_t completeTime);
    virtual void asyncRemoveSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode);
    virtual void asyncUpdateSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
        uint8_t count, sec_t completeTime);

    virtual void asyncReleaseBeginnerProtection(AccountId accountId, ObjectId characterId);
    virtual void asyncAddCooldownInfos(AccountId accountId, ObjectId characterId, const CooltimeInfos& infos);
    virtual void asyncAddRemainEffects(AccountId accountId, ObjectId characterId, const RemainEffectInfos& infos);

    virtual void asyncUpdateCharacterInventoryInfo(AccountId accountId, ObjectId characterId, InvenType invenType,
        bool isCashSlot, uint8_t count);

	virtual void asyncCreateVehicle(AccountId accountId, ObjectId characterId, const VehicleInfo& info);
	virtual void asyncCreateGlider(AccountId accountId, ObjectId characterId, const GliderInfo& info);
	virtual void asyncDeleteVehicle(AccountId accountId, ObjectId characterId, ObjectId id);
	virtual void asyncDeleteGlider(AccountId accountId, ObjectId characterId, ObjectId id);
	virtual void asyncSelectVehicle(AccountId accountId, ObjectId characterId, ObjectId id);
	virtual void asyncSelectGlider(AccountId accountId, ObjectId characterId, ObjectId id);
	virtual void asyncUpdateGliderDurability(AccountId accountId, ObjectId characterId, ObjectId id, uint32_t currentValue);

	virtual void asyncGetBuddies(AccountId accountId, ObjectId characterId);
	virtual void asyncAddBuddy(ObjectId characterId1, ObjectId characterId2);
	virtual void asyncRemoveBuddy(ObjectId characterId1, ObjectId characterId2);
    virtual void asyncAddBlock(ObjectId ownerId, ObjectId characterId);
    virtual void asyncRemoveBlock(ObjectId ownerId, ObjectId characterId);

    virtual void asyncQueryAchievements(AccountId accountId, ObjectId characterId);
    virtual void asyncUpdateProcessAchievement(AccountId accountId, ObjectId characterId, AchievementCode code, const ProcessAchievementInfo& missionInfo);
    virtual void asyncCompleteAchievement(AccountId accountId, ObjectId characterId, AchievementCode code, AchievementPoint point);

    virtual void asyncQueryCharacterTitles(AccountId accountId, ObjectId characterId);
    virtual void asyncAddCharacterTitle(AccountId accountId, ObjectId characterId, CharacterTitleCode titleCode);

private:
    virtual bool open(sne::server::ConfigReader& configReader);
    virtual void close();
    virtual void ping() {}

private:
    MockConnection connection_;

    AccountId verifiedAccountId_;
    UserId verifiedUserId_;
    std::string verifiedPassword_;

    AccountId lastCreatedAccountId_;
    ObjectId lastCreatedCharacterId_;
    FullCharacterInfo lastCreatedCharacterInfo_;
    serverbase::GameMailCallback* gameMailCallback_;
	serverbase::CommunityBuddyCallback* buddyCallback_;
    serverbase::DBQueryPlayerCallback* playerCallback_;
    NicknameMap nicknameMap_;    
};

}} // namespace gideon { namespace servertest {
