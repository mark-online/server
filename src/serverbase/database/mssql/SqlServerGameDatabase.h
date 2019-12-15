#pragma once

#include <gideon/serverbase/database/GameDatabase.h>
#include <sne/database/ado/AdoConnection.h>
#include <sne/database/DatabaseFactory.h>
#include "SqlServerGameDatabaseCommands.h"

namespace gideon { namespace serverbase {

/**
 * @class SqlServerGameDatabase
 *
 * MS SQL Server에 특화된 GameDatabase 클래스.
 */
class SqlServerGameDatabase : public GameDatabase
{
public:
    SqlServerGameDatabase();

private:
    virtual bool open(sne::server::ConfigReader& configReader);
    virtual void close();
    virtual void ping() {
        getAdoDatabase().ping();
    }

private:
	virtual void beginTransaction();
	virtual void commitTransaction();
	virtual void rollbackTransaction();

    virtual bool getProperties(sne::server::Properties& properties);
    virtual bool getServerSpec(sne::server::ServerSpec& spec,
        const std::string& serverName, const std::string& suffix);

    virtual bool getShardInfo(FullShardInfo& shardInfo, ShardId shardId);

    virtual bool getZoneInfo(ZoneInfo& zoneInfo, const std::string& name);
    virtual bool getZoneInfoList(ZoneInfos& zoneInfos, ShardId shardId);

    virtual bool getWorldTime(WorldTime& worldTime, ZoneId zoneId);

    virtual bool getRecentLoginUserList(AccountIds& userList, uint16_t maxCount);

	virtual bool getFullUserInfo(FullUserInfo& userInfo, AccountId accountId);

    virtual ErrorCode createCharacter(ObjectId& characterId,
        AccountId accountId, const Nickname& nickname, EquipType selectEquipType,
        CharacterClass characterClass, SexType sexType, const CreateCharacterEquipments& createCharacterEquipments,
        const CharacterAppearance& appearance, ZoneId zoneId, const ObjectPosition& position);
    virtual ErrorCode deleteCharacter(GuildId& guildId, ObjectId characterId);
    virtual ErrorCode checkDuplicateNickname(const Nickname& nickname);

    virtual bool saveCharacterStats(AccountId accountId, ObjectId characterId,
        const DBCharacterStats& saveInfo);

    virtual bool loadCharacterProperties(std::string& config, std::string& prefs, ObjectId characterId);
    virtual bool saveCharacterProperties(AccountId accountId, ObjectId characterId,
        const std::string& config, const std::string& prefs);

    virtual bool getCharacterInfo(FullCharacterInfo& characterInfo, ObjectId characterId);
    virtual bool updateCharacterPoints(ObjectId characterId, const Points& points);
    virtual bool saveSelectCharacterTitle(ObjectId characterId, const CharacterTitleCode titleCode);

    virtual bool getMaxInventoryId(ObjectId& itemId, ZoneId zoneId);
    virtual bool moveInventoryItem(ObjectId itemId, SlotId slotId);
    virtual bool switchInventoryItem(ObjectId itemId1, SlotId slotId1,
        ObjectId itemId2, SlotId slotId2);   
    virtual bool addInventoryItem(AccountId accountId, ObjectId characterId,
        const ItemInfo& itemInfo, InvenType invenType);
    virtual bool addInventoryEquipItem(AccountId accountId, ObjectId characterId, const ItemInfo& itemInfo,
        InvenType invenType);
    virtual bool addEquipSocketOption(AccountId accountId, ObjectId characterId, 
        ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo);
    virtual bool removeEquipSocketOption(ObjectId itemId, SocketSlotId slotId);
    virtual bool addInventoryAccessoryItem(AccountId accountId, ObjectId characterId, const ItemInfo& itemInfo,
        InvenType invenType);

    virtual bool changeEquipItemInfo(ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount);
	
    virtual bool removeInventoryItem(ObjectId itemId);
    virtual bool updateInventoryItemCount(ObjectId itemId, uint8_t itemCount);

    virtual bool addQuestItem(AccountId accountId, ObjectId characterId, const QuestItemInfo& questItemInfo);
    virtual bool removeQuestItem(ObjectId itemId);
    virtual bool updateQuestItemUsableCount(ObjectId itemId, uint8_t usableCount);
	virtual bool updateQuestItemCount(ObjectId itemId, uint8_t stackCount);

    virtual bool equipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, EquipPart equipPart);
    virtual bool unequipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, EquipPart unequipPart);
    virtual bool replaceInventoryWithEquipItem(AccountId accountId,
        ObjectId characterId, ObjectId invenItemId, EquipPart unequipPart,
        SlotId slotId, ObjectId equipItemId, EquipPart equipPart);

    virtual bool equipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, AccessoryIndex equipPart);
    virtual bool unequipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart);
    virtual bool replaceInventoryWithAccessoryItem(AccountId accountId,
        ObjectId characterId, ObjectId invenItemId, AccessoryIndex unequipPart,
        SlotId slotId, ObjectId equipItemId, AccessoryIndex equipPart);

    virtual bool changeCharacterState(ObjectId characterId, CreatureStateType state);

    virtual bool saveActionBar(ObjectId characterId, 
        ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code);

    virtual bool learnSkill(AccountId accountId, ObjectId characterId,
        SkillCode currentSkillCode, SkillCode learnSkillCode);
    virtual bool removeSkill(ObjectId characterId, SkillCode skillCode);
    virtual bool removeAllSkill(ObjectId characterId);

	virtual bool getQuests(ObjectId characterId, QuestCodes& questCodes,
		AcceptedQuestInfos& acceptQuestInfos);
    virtual bool getRepeatQuests(ObjectId characterId, 
        CompletedRepeatQuestInfos& completeRepeatQuestInfos,
        AcceptedRepeatQuestInfos& acceptQuestInfos, QuestCodes& removeQuestCodes);
	virtual bool acceptQuest(ObjectId characterId, QuestCode questCode);
    virtual bool acceptRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t gameTime);
	virtual bool cancelQuest(ObjectId characterId, QuestCode questCode);
    virtual bool cancelRepeatQuest(ObjectId characterId, QuestCode questCode);
	virtual bool completeQuest(ObjectId characterId, QuestCode questCode);
    virtual bool completeRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptTime);
    virtual bool updateCompleteRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptTime);
	virtual bool insertQuestMission(ObjectId characterId, QuestCode questCode,
		QuestMissionCode missionCode, const QuestGoalInfo& goalInfo);
    virtual bool insertRepeatQuestMission(ObjectId characterId, QuestCode questCode,
        QuestMissionCode missionCode, const QuestGoalInfo& goalInfo);
	virtual bool updateQuestMission(ObjectId characterId, QuestCode questCode,
		QuestMissionCode missionCode, const QuestGoalInfo& goalInfo);
    virtual bool updateRepeatQuestMission(ObjectId characterId, QuestCode questCode,
        QuestMissionCode missionCode, const QuestGoalInfo& goalInfo);
    virtual bool removeCompleteRepeatQuest(ObjectId characterId, QuestCode questCode);
    virtual bool removeCompleteQuest(ObjectId characterId, QuestCode questCode);

    virtual bool getGuildInfos(GuildInfos& guildInfos);
    virtual bool getGuildRanks(GuildInfo& guildInfo, GuildId guildId);
    virtual bool getGuildBankRights(GuildInfo& guildInfo, GuildId guildId);
    virtual bool getGuildSkills(GuildInfo& guildInfo, GuildId guildId);
    virtual bool getGuildMemberInfos(GuildId guildId, GuildMemberInfos& guildMemberInfos);
    virtual bool getGuildRelationships(GuildId guildId, GuildRelationshipInfos& relationInfos);
    virtual bool getGuildApplicants(GuildId guildId, GuildApplicantInfoMap& guildApplicantInfoMap);

    virtual bool createGuild(const BaseGuildInfo& guildInfo);
    virtual bool addGuildMember(GuildId guildId, const GuildMemberInfo& guildMemberInfo);
    virtual bool removeGuildMember(ObjectId characterId);
    virtual bool removeGuild(GuildId guildId);
    virtual bool addGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId, GuildRelatioshipType type);
    virtual bool removeGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId);
	virtual bool changeGuildMemberPosition(ObjectId characterId, GuildMemberPosition position);
    virtual bool addGuildApplicant(ObjectId characterId, GuildId guildId);
    virtual bool removeGuildApplicant(ObjectId characterId);
    virtual bool modifyGuildIntroduction(GuildId guildId, const GuildIntroduction& introduction);
    virtual bool modifyGuildNotice(GuildId guildId, const GuildNotice& notice);
    virtual bool addGuildRank(GuildId guildId, const GuildRankInfo& rankInfo, uint8_t vaultCount);
    virtual bool addGuildBankRights(GuildId guildId, GuildRankId rankId, VaultId vaultId, const GuildBankVaultRightInfo& bankRankInfo);
    virtual bool deleteGuildRank(GuildId guildId, GuildRankId rankId);
    virtual bool swapGuildRank(GuildId guildId, GuildRankId rankId1, GuildRankId rankId2);
    virtual bool updateGuildRankName(GuildId guildId, GuildRankId rankId, const GuildRankName& rankName);
    virtual bool updateGuildRankRights(GuildId guildId, GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay);
    virtual bool updateGuildBankRights(GuildId guildId, GuildRankId rankId, VaultId vaultId, uint32_t rights, uint32_t itemWithdrawalPerDay);
    virtual bool updateGuildRank(ObjectId playerId, GuildRankId rankId);
    virtual bool updateGuildExp(GuildId guildId, GuildExp exp, GuildSkillPoint currentSkillPoint, 
        GuildExp dayAddExp, sec_t lastUpdateTime);
    virtual bool removeAllGuildSkills(GuildId guildId);
    virtual bool removeGuildSkill(GuildId guildId, SkillCode guildSkillCode);
    virtual bool addGuildSkill(GuildId guildId, SkillCode guildSkillCode);

    virtual bool moveGuildInventoryItem(ObjectId itemId, VaultId vaultId, SlotId slotId);
    virtual bool switchGuildInventoryItem(ObjectId itemId1, VaultId vaultId1, SlotId slotId1,
        ObjectId itemId2, VaultId vaultId2, SlotId slotId2);
    virtual bool addGuildInventoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo);
    virtual bool addGuildInventoryEquipItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo);
    virtual bool addGuildEquipSocketOption(GuildId guildId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo);
    virtual bool addGuildInventoryAccessoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo);
    virtual bool updateGuildInventoryItemCount(ObjectId itemId1,
        uint8_t itemCount);
    virtual bool removeGuildInventoryItem(ObjectId itemId);
    virtual bool updateGuildGameMoney(GuildId guildId, GameMoney gameMoney);
    virtual bool updateGuildMemberWithdraw(ObjectId playerId, GameMoney dayWithdraw, sec_t resetTime);
    virtual bool updateGuildVaultName(GuildId guildId, VaultId vaultId, const VaultName& name);
    virtual bool addGuildVault(GuildId guildId, const VaultInfo& vaultInfo);

    virtual bool getGuildVaults(GuildInfo& guildInfo, GuildId guildId);
    virtual bool getGuildInventory(GuildInventoryInfo& inventoryInfo, GuildId guildId);
    virtual bool getGuildEventLog(GuildEventLogManager& logManager, GuildId guildId);
    virtual bool getGuildGameMoneyEventLog(GuildGameMoneyEventLogManager& logManager, GuildId guildId);
    virtual bool getGuildBankEventLog(GuildBankEventLogManager& logManager, GuildId guildId);

    virtual bool getMaxGuildId(GuildId& guildId);

    virtual ObjectId getPlayerId(const Nickname& nickname);
    virtual bool getMails(MailInfos& mails, ObjectId playerId);
    virtual bool insertMail(MailInfo& mail, ObjectId playerId);
    virtual bool addMailItem(ObjectId characterId, MailId mailld, 
        const ItemInfo& itemInfo);
    virtual bool addEquipMailItem(ObjectId characterId, MailId mailld, 
        const ItemInfo& itemInfo, const EquipItemInfo& equipItemInfo);
    virtual bool addMailEquipSocketOption(ObjectId characterId, MailId mailld, 
        ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo);
    virtual bool addAccessoryMailItem(ObjectId characterId, MailId mailld, 
        const ItemInfo& itemInfo, const AccessoryItemInfo& accessoryItemInfo);
    virtual bool readMail(MailId mailId);
    virtual bool deleteMail(MailId mailId);
    virtual bool deleteMailItems(MailId mailId);
    virtual bool hasNotReadMail(ObjectId characterId);

    virtual bool getAuctions(ServerAuctionInfoMap& auctionMap, ZoneId zoneId);
    virtual bool insertAuction(ZoneId zoneId, NpcCode npcCode, const FullAuctionInfo& auctionInfo);
    virtual bool addAuctionEquipInfo(ZoneId zoneId, AuctionId auctionId, const EquipItemInfo& equipItemInfo);
    virtual bool addAuctionEquipSocketOption(ZoneId zoneId, AuctionId auctionId, SocketSlotId slotId, const EquipSocketInfo& socketInfo);
    virtual bool addAuctionAccessoryInfo(ZoneId zoneId, AuctionId auctionId, const AccessoryItemInfo& accessoryItemInfo);
    virtual bool updateBid(ZoneId zoneId, AuctionId auctionId, ObjectId buyerId, GameMoney currentBidMoney);
    virtual bool deleteAuction(ZoneId zoneId, AuctionId auctionId);

    virtual bool getCharacterArenaRecords(CharacterArenaPlayResults& results, ObjectId characterId);
    virtual bool updateArenaRecord(ObjectId characterId, ArenaModeType modeType, const ArenaPlayResult& result);
    virtual bool updateArenaPoint(ObjectId characterId, ArenaPoint arenaPoint);
    virtual bool updateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime);

    virtual bool createBuilding(const BuildingInfo& buildInfo);
    virtual bool getBuildingInfos(ObjectIdSet& deleteItems, ObjectIdSet& deleteBuilds, 
        ObjectId& maxBuildingId, BuildingInfoMap& buildingInfo, ZoneId zoneId);
    virtual bool removeBuilding(ObjectId buildingId);
    virtual bool updateBuildingState(ObjectId buildingId, BuildingStateType state,
        sec_t startBuildTime, sec_t expireTime, HitPoint currentHitPoint);
    virtual bool addSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode, uint8_t count, sec_t completeTime);
    virtual bool removeSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode);
    virtual bool updateSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode, uint8_t count, sec_t completeTime);

    virtual bool updateBuildingOwner(ObjectId buildingId, BuildingOwnerType ownerType, 
        ObjectId playerId, GuildId guildId);
    virtual bool moveBuildingInventoryItem(ObjectId itemId, SlotId slotId);
    virtual bool switchBuildingInventoryItem(ObjectId itemId1, SlotId slotId1,
        ObjectId itemId2, SlotId slotId2);
    virtual bool addBuildingInventoryItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType);
    virtual bool addBuildingInventoryEquipItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType);
    virtual bool addBuildingEquipSocketOption(ObjectId buildingId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo);
    virtual bool addBuildingInventoryAccessoryItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType);

    virtual bool removeBuildingInventoryItem(ObjectId itemId);
    virtual bool updateBuildingInventoryItemCount(ObjectId itemId, uint8_t itemCount);
    virtual bool removeAllBuildingInventoryItem(ObjectId buildingId, InvenType invenType);
    
    virtual bool addBuildingGuard(ObjectId buildingId, const BuildingGuardInfo& info);
    virtual bool removeBuildingGuard(ObjectId guraderId);

    virtual bool addBindRecallInfo(AccountId accountId, ObjectId characterId, const BindRecallInfo& bindRecallInfo);
    virtual bool removeBindRecallInfo(ObjectId linkId);

    virtual bool releaseBeginnerProtection(ObjectId characterId);
    virtual bool addCooldown(AccountId accountId, ObjectId characterId, DataCode dataCode, sec_t expireTime);
    virtual bool removeAllCooldown(ObjectId characterId);
    virtual bool addRemainEffect(AccountId accountId, ObjectId characterId, const RemainEffectInfo& effectInfo);
    virtual bool removeAllRemainEffect(ObjectId characterId);

    virtual bool updateCharacterInventoryInfo(ObjectId characterId, InvenType invenType, bool isCashSlot, uint8_t count);

	virtual bool createVehicle(AccountId accountId, ObjectId characterId, const VehicleInfo& info);
	virtual bool createGlider(AccountId accountId, ObjectId characterId, const GliderInfo& info);
	virtual bool deleteVehicle(ObjectId id);
	virtual bool deleteGlider(ObjectId id);
	virtual bool selectVehicle(ObjectId characterId, ObjectId id);
	virtual bool selectGlider(ObjectId characterId, ObjectId id);
	virtual bool updateGliderDurability(ObjectId id, uint32_t currentValue);

	virtual bool getBuddies(BuddyInfos& buddyInfos, BlockInfos& blockInfos, ObjectId characterId);
	virtual bool addBuddy(ObjectId characterId1, ObjectId characterId2);
	virtual bool removeBuddy(ObjectId characterId1, ObjectId characterId2);
    virtual bool addBlock(ObjectId ownerId, ObjectId characterId);
    virtual bool removeBlock(ObjectId ownerId, ObjectId characterId);

    virtual bool getCharacterTitles(CharacterTitleCodeSet& titleCodes, ObjectId characterId);
    virtual bool getCompleteAchievements(CompleteAchievementInfoMap& completeAchievementMap, ObjectId characterId);
    virtual bool getProcessAchievements(ProcessAchievementInfoMap& achievementInfoMap, ObjectId characterId);

    virtual bool addCharacterTitle(ObjectId characterId, CharacterTitleCode titleCode);
    virtual bool addProcessAchievement(ObjectId characterId, AchievementCode code,
        const ProcessAchievementInfo& missionInfo);
    virtual bool updateProcessAchievement(ObjectId characterId, AchievementCode code,
        const ProcessAchievementInfo& missionInfo);
    virtual bool completeAchievement(ObjectId characterId, AchievementCode code, AchievementPoint point, sec_t now);

private:
    sne::database::AdoDatabase& getAdoDatabase() {
        return connection_.getDatabase();
    }

private:
    sne::database::AdoConnection connection_;

    GetPropertiesCommand getPropertiesCommand_;
    GetServerSpecCommand getServerSpecCommand_;
    GetShardInfoCommand getShardInfoCommand_;
    GetZoneInfoCommand getZoneInfoCommand_;
    GetZoneListCommand getZoneListCommand_;
    GetWorldTimeCommand getWorldTimeCommand_;
    UpdateWorldTimeCommand updateWorldTimeCommand_;
    GetMaxInventoryIdCommand getMaxInventoryIdCommand_;
    GetRecentLoginUserListCommand getRecentLoginUserListCommand_;
	GetFullUserInfoCommand getFullUserInfoCommand_;
    CreateCharacterCommand createCharacterCommand_;
    DeleteCharacterCommand deleteCharacterCommand_;
    CheckDuplicateNicknameCommand checkDuplicateNicknameCommand_;
    SaveCharacterStatsCommand saveCharacterStatsCommand_;

    LoadCharacterPropertiesCommand loadCharacterPropertiesCommand_;
    SaveCharacterPropertiesCommand saveCharacterPropertiesCommand_;
    GetCharacterInfoCommand getCharacterInfoCommand_;
    UpdateCharacterPointsCommand updateCharacterPointsCommand_;
    SaveSelectCharacterTitleCommand saveSelectCharacterTitleCommand_;

    MoveItemCommand moveItemCommand_;
    SwitchItemCommand switchItemCommand_;
    AddItemCommand addItemCommand_;
    AddEquipItemCommand addEquipItemCommand_;
    AddEquipSocketOptionCommand addEquipSocketOptionCommand_;
    RemoveEquipSocketOptionCommand removeEquipSocketOptionCommand_;
    AddAccessoryItemCommand addAccessoryItemCommand_;
    ChangeEquipItemInfoCommand changeEquipItemInfoCommand_;
	
    RemoveItemCommand removeItemCommand_;
    UpdateItemCountCommand updateItemCountCommand_;
    AddQuestItemCommand addQuestItemCommand_;
    RemoveQuestItemCommand removeQuestItemCommand_;
    UpdateQuestItemUsableCountCommand updateQuestItemUsableCountCommand_;
	UpdateQuestItemCountCommand updateQuestItemCountCommand_;

    EquipItemCommand equipItemCommand_;
    UnequipItemCommand unequipItemCommand_;
    ReplaceInventoryWithEquipItemCommand replaceInvenWithEquipItemCommand_;


    EquipAccessoryItemCommand equipAccessoryItemCommand_;
    UnequipAccessoryItemCommand unequipAccessoryItemCommand_;
    ReplaceInventoryWithAccessoryItemCommand replaceInventoryWithAccessoryItemCommand_;


    ChangeCharacterStateCommand changeCharacterStateCommand_;
    SaveActionBarCommand saveActionBarCommand_;
    LearnSkillCommand learnSkillCommand_;
    RemoveSkillCommand removeSkillCommand_;
    RemoveAllSkillCommand removeAllSkillCommand_;

	GetQuestsCommand getQuestsCommand_;
    GetRepeatQuestsCommand getRepeatQuestsCommand_;
	AcceptedQuestCommand acceptQuestCommand_;
    AcceptRepeatQuestCommand acceptRepeatQuestCommand_;
	CancelQuestCommand cancelQuestCommand_;
    CancelRepeatQuestCommand cancelRepeatQuestCommand_;
	CompleteQuestCommand completeQuestCommand_;
    CompleteRepeatQuestCommand completeRepeatQuestCommand_;
    UpdateCompleteRepeatQuestCommand updateCompleteRepeatQuestCommand_;
	InsertQuestMissionCommand insertQuestMissionCommand_;
    InsertRepeatQuestMissionCommand insertRepeatQuestMissionCommand_;
	UpdateQuestMissionCommand updateQuestMissionCommand_;
    UpdateRepeatQuestMissionCommand updateRepeatQuestMissionCommand_;
    RemoveCompleteRepeatQuest removeCompleteRepeatQuest_;
    RemoveCompleteQuestCommand removeCompleteQuestCommand_;

    CreateGuildCommand createGuildCommand_;
    AddGuildMemberCommand addGuildMemberCommand_;
    RemoveGuildMemberCommand removeGuildMemberCommand_;
    RemoveGuildCommand removeGuildCommand_;
    AddGuildRelationshipCommand addGuildRelationshipCommand_;
    RemoveGuildRelationshipCommand removeGuildRelationshipCommand_;
	ChangeGuildMemberPositionCommand changeGuildMemberPositionCommand_;
    AddGuildApplicantCommand addGuildApplicantCommand_;
    RemoveGuildApplicantCommand removeGuildApplicantCommand_;
    ModifyGuildIntroductionCommand modifyGuildIntroductionCommand_;
    ModifyGuildNoticeCommand modifyGuildNoticeCommand_;
    UpdateGuildGameMoneyCommand updateGuildGameMoney_;
    UpdateGuildMemberWithdrawCommand updateGuildMemberWithdrawCommand_;
    UpdateGuildVaultNameCommand updateGuildVaultNameCommand_;
    AddGuildVaultCommand addGuildVaultCommand_;

    AddGuildItemCommand addGuildItemCommand_;
    AddGuildEquipItemCommand addGuildEquipItemCommand_;
    AddGuildEquipSocketOptionCommand addGuildEquipSocketOptionCommand_;
    AddGuildAccessoryItemCommand addGuildAccessoryItemCommand_;
    RemoveGuildItemCommand removeGuildItemCommand_;
    UpdateGuildItemCountCommand updateGuildItemCountCommand_;
    MoveGuildItemCommand moveGuildItemCommand_;
    SwitchGuildItemCommand switchGuildItemCommand_;

    GetGuildInfosCommand getGuildInfosCommand_;
    GetGuildRanksCommand getGuildRanksCommand_;
    GetGuildBankRightsCommand getGuildBankRightsCommand_;
    GetGuildSkillsCommand getGuildSkillsCommand_;
    GetGuildMemberInfosCommand getGuildMemberInfosCommand_;
    GetGuildRelationshipsCommand getGuildRelationshipsCommand_;
    GuildApplicantsCommand guildApplicantsCommand_;
    GetMaxGuildIdCommand getMaxGuildIdCommand_;
    GetGuildVaultsCommand getGuildVaultsCommand_;
    GetGuildInventoryCommand getGuildInventory_;
    GetGuildEventLogCommand getGuildEventLogCommand_;
    GetGuildBankEventLogCommand getGuildBankEventLogCommand_;
    GetGuildGameMoneyEventLogCommand getGuildGameMoneyEventLogCommand_;
    AddGuildRankCommand addGuildRankCommand_;
    AddGuildBankRightsCommand addGuildBankRightsCommand_;
    DeleteGuildRankCommand deleteGuildRankCommand_;
    SwapGuildRankCommand swapGuildRankCommand_;
    UpdateGuildRankNameCommand updateGuildRankNameCommand_;
    UpdateGuildRankRightsCommand updateGuildRankRightsCommand_;
    UpdateGuildBankRightsCommand updateGuildBankRightsCommand_;
    UpdateGuildRankCommand updateGuildRankCommand_;
    UpdateGuildExpCommand updateGuildExpCommand_;
    AddGuildSkillCommand addGuildSkillCommand_;
    RemoveGuildSkillCommand removeGuildSkillCommand_;
    RemoveAllGuildSkillsCommand removeAllGuildSkillsCommand_;

    GetPlayerIdCommand getPlayerIdCommand_;
    GetMailsCommand getMailsCommand_;
    InsertMailCommand insertMailCommand_;
    AddMailItemCommand addMailItemCommand_;
    AddMailEquipItemCommand addMailEquipItemCommand_;
    AddMailEquipSocketOptionCommand addMailEquipSocketOptionCommand_;
    AddMailAccessoryItemCommand addMailAccessoryItemCommand_;
    DeleteMailItemCommand deleteMailItemCommand_;
    DeleteMailCommand deleteMailCommand_;
    ReadMailCommand readMailCommand_;
    HasNotReadMailCommand hasNotReadMailCommand_;

    GetAuctionsCommand getAuctionsCommand_;
    InsertAuctionCommand insertAuctionCommand_;
    AddAuctionEquipInfoCommand addAuctionEquipInfoCommand_;
    AddAuctionEquipSocketOptionCommand addAuctionEquipSocketOptionCommand_;
    AddAuctionAccessoryInfoCommand addAuctionAccessoryInfoCommand_;
    UpdateBidCommand updateBidCommand_;
    DeleteAuctionCommand deleteAuctionCommand_;

    GetPlayerArenaRecordsCommand getPlayerArenaRecordsCommand_;
    UpdateArenaRecordCommand updateArenaRecordCommand_;
    UpdateArenaPointCommand updateArenaPointCommand_;
    UpdateDeserterExpireTimeCommand updateDeserterExpireTimeCommand_;
    
    CreateBuildingCommand createBuildingCommand_;
    GetBuildingsCommand getBuildingsCommand_;
    RemoveBuildingCommand removeBuildingCommand_;
    UpdateBuildingStateCommand updateBuildingStateCommand_;
    UpdateBuildingOwnerCommand updateBuildingOwnerCommand_;
    MoveBuildingItemCommand moveBuildingItemCommand_;
    SwitchBuildingItemCommand switchBuildingItemCommand_;
    AddBuildingItemCommand addBuildingItemCommand_;
    AddBuildingEquipItemCommand addBuildingEquipItemCommand_;
    AddBuildingEquipSocketOptionCommand addBuildingEquipSocketOptionCommand_;
    AddBuildingAccessoryItemCommand addBuildingAccessoryItemCommand_;
    RemoveBuildingItemCommand removeBuildingItemCommand_;
    UpdateBuildingItemCountCommand updateBuildingItemCountCommand_;
    RemoveAllBuildingItemCommand removeAllBuildingItemCommand_;

    AddBuildingGuardCommand addBuildingGuardCommand_;
    RemoveBuildingGuardCommand removeBuildingGuardCommand_;

    AddSelectRecipeProductionCommand addSelectRecipeProductionCommand_;
    RemoveSelectRecipeProductionCommand removeSelectRecipeProductionCommand_;
    UpdateSelectRecipeProductionCommand updateSelectRecipeProductionCommand_;

    AddBindRecallInfoCommand addBindRecallInfoCommand_;
    RemoveBindRecallCommand removeBindRecallCommand_;

    ReleaseBeginnerProtectionCommand releaseBeginnerProtectionCommand_;

    AddCooldownCommand addCooldownCommand_;
    AddRemainEffectCommand addRemainEffectCommand_;
    RemoveAllCooldownCommand removeAllCooldownCommand_;
    RemoveAllRemainEffectCommand removeAllRemainEffectCommand_;

    UpdateCharacterInventoryInfoCommand updateCharacterInventoryInfoCommand_;

	CreateVehicleCommand createVehicleCommand_;
	CreateGliderCommand createGliderCommand_;
	DeleteVehicleCommand deleteVehicleCommand_;
	DeleteGliderCommand deleteGliderCommand_;

	SelectVehicleCommand selectVehicleCommand_;
	SelectGliderCommand selectGliderCommand_;
	UpdateGliderDurabilityCommand updateGliderDurabilityCommand_;

	GetBuddiesCommand getBuddiesCommand_;
	AddBuddyCommand addBuddyCommand_;
	RemoveBuddyCommand removeBuddyCommand_;
    AddBlockCommand addBlockCommand_;
    RemoveBlockCommand removeBlockCommand_;

    GetCharacterTitlesCommand getCharacterTitlesCommand_;
    GetCompleteAchievementsCommand getCompleteAchievementsCommand_;
    GetProcessAchievementsCommand getProcessAchievementsCommand_;
    AddCharacterTitleCommand addCharacterTitleCommand_;
    AddProcessAchievementCommand addProcessAchievementCommand_;
    UpdateProcessAchievementCommand updateProcessAchievementCommand_;
    CompleteAchievementCommand completeAchievementCommand_;

};

}} // namespace gideon { namespace serverbase {
