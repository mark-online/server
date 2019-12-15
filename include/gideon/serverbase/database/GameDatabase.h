#pragma once

#include <gideon/server/data/CreateCharacterInfo.h>
#include <gideon/server/data/ShardInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/cs/shared/data/MailInfo.h>
#include <gideon/cs/shared/data/AuctionInfo.h>
#include <gideon/cs/shared/data/buildingInfo.h>
#include <gideon/cs/shared/data/BuddyInfos.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/AchievementInfo.h>
#include <gideon/cs/shared/data/CharacterTitleInfo.h>
#include <gideon/server/data/CharacterStats.h>
#include <sne/server/common/Property.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/database/Database.h>

namespace gideon { namespace serverbase {

/**
 * @struct GameDatabase
 *
 * Game Database.
 */
class GameDatabase : public sne::database::Database
{
public:
    // = Helpers

    virtual void beginTransaction() = 0;
    virtual void commitTransaction() = 0;
    virtual void rollbackTransaction() = 0;

public:
    // = 서버 설정 관련

    /// 키/값 속성들을 읽어온다
    virtual bool getProperties(sne::server::Properties& properties) = 0;

    virtual bool getServerSpec(sne::server::ServerSpec& spec,
        const std::string& serverName, const std::string& suffix) = 0;
public:
    // = Shard 관련

    /// Shard 정보를 읽어온다
    virtual bool getShardInfo(FullShardInfo& shardInfo, ShardId shardId) = 0;

    // = Zone 관련

    /// 특정 존 정보를 읽어온다
    virtual bool getZoneInfo(ZoneInfo& zoneInfo, const std::string& name) = 0;

    /// 전체 존 정보를 읽어온다
    virtual bool getZoneInfoList(ZoneInfos& zoneInfos, ShardId shardId) = 0;

    /// 존의 월드 시간을 읽어온다
    virtual bool getWorldTime(WorldTime& worldTime, ZoneId zoneId) = 0;

public:
    // = 사용자 관련

    /// 가장 최근에 로그인한 사용자 목록을 최대 maxCount 만틈 읽어온다
    virtual bool getRecentLoginUserList(AccountIds& userList, uint16_t maxCount) = 0;

    /// 전체 사용자 정보를 얻는다
    virtual bool getFullUserInfo(FullUserInfo& userInfo, AccountId accountId) = 0;

    // = 캐릭터 관련

    /// 캐릭터를 생성한다
    virtual ErrorCode createCharacter(ObjectId& characterId, 
        AccountId accountId, const Nickname& nickname, EquipType selectEquipType,
        CharacterClass characterClass, SexType sexType, const CreateCharacterEquipments& createCharacterEquipments,
        const CharacterAppearance& appearance, ZoneId zoneId, const ObjectPosition& position) = 0;

    /// 캐릭터를 삭제한다
    virtual ErrorCode deleteCharacter(GuildId& guildId, ObjectId characterId) = 0;

    /// 중복 닉네임 체크
    virtual ErrorCode checkDuplicateNickname(const Nickname& nickname) = 0;

    /// 캐릭터의 최종 상태를 저장한다
    virtual bool saveCharacterStats(AccountId accountId, ObjectId characterId,
        const DBCharacterStats& saveInfo) = 0;

    /// 캐릭터의 속성(config)를 읽어온다
    virtual bool loadCharacterProperties(std::string& config, std::string& prefs, ObjectId characterId) = 0;

    /// 캐릭터의 속성(config)를 저장한다
    virtual bool saveCharacterProperties(AccountId accountId, ObjectId characterId,
        const std::string& config, const std::string& prefs) = 0;

    /// 캐릭터 상세 정보를 읽어온다
    virtual bool getCharacterInfo(FullCharacterInfo& characterInfo,
        ObjectId characterId) = 0;

    /// 케릭터 hp mp cp
    virtual bool updateCharacterPoints(ObjectId characterId, const Points& points) = 0;

    virtual bool saveSelectCharacterTitle(ObjectId characterId, const CharacterTitleCode titleCode) = 0;

public:
    // = 인벤토리 관련

    /// 인벤토리 최대 값을 가지고 온다.
    virtual bool getMaxInventoryId(ObjectId& itemId, ZoneId zoneId) = 0;

    /// 인벤토리 아이템을 빈 슬롯으로 옮긴다
    virtual bool moveInventoryItem(ObjectId itemId, SlotId slotId) = 0;

    /// 인벤토리 아이템들의 슬롯을 교환한다
    virtual bool switchInventoryItem(ObjectId itemId1, SlotId slotId1,
        ObjectId itemId2, SlotId slotId2) = 0;
    
    /// 인벤토리에 아이템을 추가한다.
    virtual bool addInventoryItem(AccountId accountId, ObjectId characterId,
        const ItemInfo& itemInfo, InvenType invenType) = 0;

    /// 인벤토리에 장착 아이템을 추가한다.
    virtual bool addInventoryEquipItem(AccountId accountId, ObjectId characterId, const ItemInfo& itemInfo,
        InvenType invenType) = 0;
    virtual bool addEquipSocketOption(AccountId accountId, ObjectId characterId, 
        ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo) = 0;
    virtual bool removeEquipSocketOption(ObjectId itemId, SocketSlotId slotId) = 0;

    virtual bool addInventoryAccessoryItem(AccountId accountId, ObjectId characterId, const ItemInfo& itemInfo,
        InvenType invenType) = 0;

    /// 창착 아이템 정보를 변경한다.
    virtual bool changeEquipItemInfo(ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount) = 0;

    /// 인벤토리에 아이템을 삭제한다.
    virtual bool removeInventoryItem(ObjectId itemId) = 0;

    /// 인벤토리에 아이템 갯수를 변화시킨다
    virtual bool updateInventoryItemCount(ObjectId itemId1,
        uint8_t itemCount) = 0;

    /// 퀘스트 아이템 추가
    virtual bool addQuestItem(AccountId accountId, ObjectId characterId, const QuestItemInfo& questItemInfo) = 0;

    /// 퀘스트 아이템 삭제
    virtual bool removeQuestItem(ObjectId itemId) = 0;

    /// 퀘스트 아이템 사용갯수 변경
    virtual bool updateQuestItemUsableCount(ObjectId itemId, uint8_t usableCount) = 0;
	
	/// 퀘스트 아이템 스택
	virtual bool updateQuestItemCount(ObjectId itemId, uint8_t stackCount) = 0;

public:
    // = 착용 장비 관련

    /// 장비를 장착한다
    virtual bool equipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, EquipPart equipPart) = 0;

    /// 장비를 탈착한다
    virtual bool unequipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, EquipPart unequipPart) = 0;

    /// 인벤에 있는 아이템과 장착슬롯에 있는 아이템을 교환한다.
    virtual bool replaceInventoryWithEquipItem(AccountId accountId,
        ObjectId characterId, ObjectId itemId1, EquipPart unequipPart, SlotId slotId,
        ObjectId itemId2, EquipPart equipPart) = 0;

public:
    // = 착용 악세사리 관련

    /// 악세사리를 장착한다
    virtual bool equipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, AccessoryIndex equipIndex) = 0;

    /// 악세사리를 탈착한다
    virtual bool unequipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, AccessoryIndex unequipIndex) = 0;

    /// 인벤에 있는 아이템과 장착슬롯에 있는 악세사리을 교환한다.
    virtual bool replaceInventoryWithAccessoryItem(AccountId accountId,
        ObjectId characterId, ObjectId itemId1, AccessoryIndex unequipIndex, SlotId slotId,
        ObjectId itemId2, AccessoryIndex equipIndex) = 0;

public:
    // = 케릭터 상태 변환(전투 평화 상태만 저장)
    virtual bool changeCharacterState(ObjectId characterId, CreatureStateType  state) = 0;

public:
    // = 액션바 관련
    virtual bool saveActionBar(ObjectId characterId, 
        ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code) = 0;

    virtual bool learnSkill(AccountId accountId, ObjectId characterId, 
        SkillCode currentSkillCode, SkillCode learnSkillCode) = 0;
    virtual bool removeSkill(ObjectId characterId, SkillCode skillCode) = 0;
    virtual bool removeAllSkill(ObjectId characterId) = 0;

public:
	virtual bool getQuests(ObjectId characterId, QuestCodes& questCodes,
		AcceptedQuestInfos& acceptQuestInfos) = 0;
    virtual bool getRepeatQuests(ObjectId characterId, 
        CompletedRepeatQuestInfos& completeRepeatQuestInfos,
        AcceptedRepeatQuestInfos& acceptQuestInfos, QuestCodes& removeQuestCodes) = 0;

	virtual bool acceptQuest(ObjectId characterId, QuestCode questCode) = 0;
    virtual bool acceptRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t gameTime) = 0;
	virtual bool cancelQuest(ObjectId characterId, QuestCode questCode) = 0;
    virtual bool cancelRepeatQuest(ObjectId characterId, QuestCode questCode) = 0;
    virtual bool completeQuest(ObjectId characterId, QuestCode questCode) = 0;
    virtual bool completeRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptTime) = 0;
    virtual bool updateCompleteRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptTime) = 0;
    virtual bool insertQuestMission(ObjectId characterId, QuestCode questCode,
		QuestMissionCode missionCode, const QuestGoalInfo& goalInfo) = 0;
    virtual bool insertRepeatQuestMission(ObjectId characterId, QuestCode questCode,
        QuestMissionCode missionCode, const QuestGoalInfo& goalInfo) = 0;
	virtual bool updateQuestMission(ObjectId characterId, QuestCode questCode,
		QuestMissionCode missionCode, const QuestGoalInfo& goalInfo) = 0;
    virtual bool updateRepeatQuestMission(ObjectId characterId, QuestCode questCode,
        QuestMissionCode missionCode, const QuestGoalInfo& goalInfo) = 0;
    virtual bool removeCompleteRepeatQuest(ObjectId characterId, QuestCode questCode) = 0;
	virtual bool removeCompleteQuest(ObjectId characterId, QuestCode questCode) = 0;

public:
    virtual bool getGuildInfos(GuildInfos& guildInfos) = 0;
    virtual bool getGuildRanks(GuildInfo& guildInfo, GuildId guildId) = 0;
    virtual bool getGuildBankRights(GuildInfo& guildInfo, GuildId guildId) = 0;
    virtual bool getGuildSkills(GuildInfo& guildInfo, GuildId guildId) = 0;
    virtual bool getGuildMemberInfos(GuildId guildId, GuildMemberInfos& guildMemberInfos) = 0;
    virtual bool getGuildRelationships(GuildId guildId, GuildRelationshipInfos& relationInfos) = 0;
    virtual bool getGuildApplicants(GuildId guildId, GuildApplicantInfoMap& guildApplicantInfoMap) = 0;
    virtual bool getGuildVaults(GuildInfo& guildInfo, GuildId guildId) = 0;
    virtual bool getGuildEventLog(GuildEventLogManager& logManager, GuildId guildId) = 0;
    virtual bool getGuildGameMoneyEventLog(GuildGameMoneyEventLogManager& logManager, GuildId guildId) = 0;
    virtual bool getGuildBankEventLog(GuildBankEventLogManager& logManager, GuildId guildId) = 0;

    virtual bool createGuild(const BaseGuildInfo& guildInfo) = 0;
    virtual bool addGuildMember(GuildId guildId, const GuildMemberInfo& guildMemberInfo) = 0;
    virtual bool removeGuildMember(ObjectId characterId) = 0;
    virtual bool getMaxGuildId(GuildId& guildId) = 0;
    virtual bool removeGuild(GuildId guildId) = 0;
    virtual bool addGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId, GuildRelatioshipType type) = 0;
    virtual bool removeGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId) = 0;
	virtual bool changeGuildMemberPosition(ObjectId characterId, GuildMemberPosition position) = 0;
    virtual bool addGuildApplicant(ObjectId characterId, GuildId guildId) = 0;
    virtual bool removeGuildApplicant(ObjectId characterId) = 0;
    virtual bool modifyGuildIntroduction(GuildId guildId, const GuildIntroduction& introduction) = 0;
    virtual bool modifyGuildNotice(GuildId guildId, const GuildNotice& notice) = 0;
    virtual bool addGuildRank(GuildId guildId, const GuildRankInfo& rankInfo, uint8_t vaultCount) = 0;
    virtual bool addGuildBankRights(GuildId guildId, GuildRankId rankId, VaultId vaultId, const GuildBankVaultRightInfo& bankRankInfo) = 0;
    virtual bool deleteGuildRank(GuildId guildId, GuildRankId rankId) = 0;
    virtual bool swapGuildRank(GuildId guildId, GuildRankId rankId1, GuildRankId rankId2) = 0;
    virtual bool updateGuildRankName(GuildId guildId, GuildRankId rankId, const GuildRankName& rankName) = 0;
    virtual bool updateGuildRankRights(GuildId guildId, GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay) = 0;
    virtual bool updateGuildBankRights(GuildId guildId, GuildRankId rankId, VaultId vaultId, uint32_t rights, uint32_t itemWithdrawalPerDay) = 0;
    virtual bool updateGuildRank(ObjectId playerId, GuildRankId rankId) = 0;

    virtual bool updateGuildExp(GuildId guildId, GuildExp exp, GuildSkillPoint currentSkillPoint, 
        GuildExp dayAddExp, sec_t lastUpdateTime) = 0;
    virtual bool removeAllGuildSkills(GuildId guildId) = 0;
    virtual bool removeGuildSkill(GuildId guildId, SkillCode guildSkillCode) = 0;
    virtual bool addGuildSkill(GuildId guildId, SkillCode guildSkillCode) = 0;

    
    virtual bool moveGuildInventoryItem(ObjectId itemId, VaultId vaultId, SlotId slotId) = 0;
    virtual bool switchGuildInventoryItem(ObjectId itemId1, VaultId vaultId1, SlotId slotId1,
        ObjectId itemId2, VaultId vaultId2, SlotId slotId2) = 0;
    virtual bool addGuildInventoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo) = 0;
    virtual bool addGuildInventoryEquipItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo) = 0;
    virtual bool addGuildEquipSocketOption(GuildId guildId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo) = 0;
    virtual bool addGuildInventoryAccessoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo) = 0;
    virtual bool removeGuildInventoryItem(ObjectId itemId) = 0;
    virtual bool updateGuildInventoryItemCount(ObjectId itemId1,
        uint8_t itemCount) = 0;
    virtual bool updateGuildGameMoney(GuildId guildId, GameMoney gameMoney) = 0;
    virtual bool updateGuildMemberWithdraw(ObjectId playerId, GameMoney dayWithdraw, sec_t resetTime) = 0;
    virtual bool updateGuildVaultName(GuildId guildId, VaultId vaultId, const VaultName& name) = 0;
    virtual bool addGuildVault(GuildId guildId, const VaultInfo& vaultInfo) = 0;

    virtual bool getGuildInventory(GuildInventoryInfo& inventoryInfo, GuildId guildId) = 0;
public:
    virtual ObjectId getPlayerId(const Nickname& nickname) = 0;
    virtual bool getMails(MailInfos& mails, ObjectId playerId) = 0;
    virtual bool insertMail(MailInfo& mail, ObjectId playerId) = 0;
    virtual bool addMailItem(ObjectId characterId, MailId mailld, const ItemInfo& itemInfo) = 0;
    virtual bool addEquipMailItem(ObjectId characterId, MailId mailld, const ItemInfo& itemInfo,
        const EquipItemInfo& equipItemInfo) = 0;
    virtual bool addMailEquipSocketOption(ObjectId characterId, MailId mailld, 
        ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo) = 0;
    virtual bool addAccessoryMailItem(ObjectId characterId, MailId mailld, 
        const ItemInfo& itemInfo, const AccessoryItemInfo& equipItemInfo) = 0;
    virtual bool readMail(MailId mailId) = 0;
    virtual bool deleteMail(MailId mailId) = 0;
    virtual bool deleteMailItems(MailId mailId) = 0;
    virtual bool hasNotReadMail(ObjectId characterId) = 0;

public:
    virtual bool getAuctions(ServerAuctionInfoMap& auctionMap, ZoneId zoneId) = 0;
    virtual bool insertAuction(ZoneId zoneId, NpcCode npcCode, const FullAuctionInfo& auctionInfo) = 0;
    virtual bool addAuctionEquipInfo(ZoneId zoneId, AuctionId auctionId, const EquipItemInfo& equipItemInfo) = 0;
    virtual bool addAuctionEquipSocketOption(ZoneId zoneId, AuctionId auctionId, SocketSlotId slotId, const EquipSocketInfo& socketInfo) = 0;
    virtual bool addAuctionAccessoryInfo(ZoneId zoneId, AuctionId auctionId, const AccessoryItemInfo& accessoryItemInfo) = 0;
    virtual bool updateBid(ZoneId zoneId, AuctionId auctionId, ObjectId buyerId, GameMoney currentBidMoney) = 0;
    virtual bool deleteAuction(ZoneId zoneId, AuctionId auctionId) = 0;

public:
    virtual bool getCharacterArenaRecords(CharacterArenaPlayResults& results, ObjectId characterId) = 0;
    virtual bool updateArenaRecord(ObjectId characterId, ArenaModeType modeType, const ArenaPlayResult& result) = 0;
    virtual bool updateArenaPoint(ObjectId characterId, ArenaPoint arenaPoint) = 0;
    virtual bool updateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime) = 0;

public:
    virtual bool createBuilding(const BuildingInfo& buildInfo) = 0;
    virtual bool getBuildingInfos(ObjectIdSet& deleteItems, ObjectIdSet& deleteBuilds, 
        ObjectId& maxBuildingId, BuildingInfoMap& buildingInfo, ZoneId zoneId) = 0;
    virtual bool removeBuilding(ObjectId buildingId) = 0;
    virtual bool updateBuildingState(ObjectId buildingId, BuildingStateType state,
        sec_t startBuildTime, sec_t expireTime, HitPoint currentHitPoint) = 0;
    virtual bool addSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode, uint8_t count, sec_t completeTime) = 0;
    virtual bool removeSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode) = 0;
    virtual bool updateSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode, uint8_t count, sec_t completeTime) = 0;

    virtual bool updateBuildingOwner(ObjectId buildingId, BuildingOwnerType ownerType, ObjectId playerId, GuildId guildId) = 0;
    virtual bool moveBuildingInventoryItem(ObjectId itemId, SlotId slotId) = 0;
    virtual bool switchBuildingInventoryItem(ObjectId itemId1, SlotId slotId1,
        ObjectId itemId2, SlotId slotId2) = 0;
    virtual bool addBuildingInventoryItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType) = 0;
    virtual bool addBuildingInventoryEquipItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType) = 0;
    virtual bool addBuildingEquipSocketOption(ObjectId buildingId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo) = 0;
    virtual bool addBuildingInventoryAccessoryItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType) = 0;

    virtual bool removeBuildingInventoryItem(ObjectId itemId) = 0;
    virtual bool updateBuildingInventoryItemCount(ObjectId itemId, uint8_t itemCount) = 0;
    virtual bool removeAllBuildingInventoryItem(ObjectId buildingId, InvenType invenType) = 0;

    virtual bool addBuildingGuard(ObjectId buildingId, const BuildingGuardInfo& info) = 0;
    virtual bool removeBuildingGuard(ObjectId guraderId) = 0;

    virtual bool addBindRecallInfo(AccountId accountId, ObjectId characterId, const BindRecallInfo& bindRecallInfo) = 0;
    virtual bool removeBindRecallInfo(ObjectId linkId) = 0;

    virtual bool releaseBeginnerProtection(ObjectId characterId) = 0;

    virtual bool addCooldown(AccountId accountId, ObjectId characterId, DataCode dataCode, sec_t expireTime) = 0;
    virtual bool removeAllCooldown(ObjectId characterId) = 0;
    virtual bool addRemainEffect(AccountId accountId, ObjectId characterId, const RemainEffectInfo& effectInfo) = 0;
    virtual bool removeAllRemainEffect(ObjectId characterId) = 0;

public:
    virtual bool updateCharacterInventoryInfo(ObjectId characterId, InvenType invenType, bool isCashSlot, uint8_t count) = 0;

public:
	virtual bool createVehicle(AccountId accountId, ObjectId characterId, const VehicleInfo& info) = 0;
	virtual bool createGlider(AccountId accountId, ObjectId characterId, const GliderInfo& info) = 0;
	virtual bool deleteVehicle(ObjectId id) = 0;
	virtual bool deleteGlider(ObjectId id) = 0;
	virtual bool selectVehicle(ObjectId characterId, ObjectId id) = 0;
	virtual bool selectGlider(ObjectId characterId, ObjectId id) = 0;
	virtual bool updateGliderDurability(ObjectId id, uint32_t currentValue) = 0;

public:
	virtual bool getBuddies(BuddyInfos& buddyInfos, BlockInfos& blockInfos, ObjectId characterId) = 0;
	virtual bool addBuddy(ObjectId characterId1, ObjectId characterId2) = 0;
	virtual bool removeBuddy(ObjectId characterId1, ObjectId characterId2) = 0;
    virtual bool addBlock(ObjectId ownerId, ObjectId characterId) = 0;
    virtual bool removeBlock(ObjectId ownerId, ObjectId characterId) = 0;

public:
    virtual bool getCharacterTitles(CharacterTitleCodeSet& titleCodes, ObjectId characterId) = 0;
    virtual bool getCompleteAchievements(CompleteAchievementInfoMap& completeAchievementMap, ObjectId characterId) = 0;
    virtual bool getProcessAchievements(ProcessAchievementInfoMap& achievementMap, ObjectId characterId) = 0;
   
    virtual bool addCharacterTitle(ObjectId characterId, CharacterTitleCode titleCode) = 0;
    virtual bool addProcessAchievement(ObjectId characterId, AchievementCode code,
        const ProcessAchievementInfo& missionInfo) = 0;
    virtual bool updateProcessAchievement(ObjectId characterId, AchievementCode code,
        const ProcessAchievementInfo& missionInfo) = 0;
    virtual bool completeAchievement(ObjectId characterId, AchievementCode code, AchievementPoint point, sec_t now) = 0;
};

}} // namespace gideon { namespace serverbase {
