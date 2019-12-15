#pragma once

#include "callback/GameMailCallback.h"
#include "callback/CommunityBuddyCallback.h"
#include "callback/DBQueryPlayerCallback.h"
#include <gideon/server/data/CreateCharacterInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/QuestInfo.h>
#include <gideon/cs/shared/data/MailInfo.h>
#include <gideon/cs/shared/data/AuctionInfo.h>
#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <gideon/cs/shared/data/AchievementInfo.h>
#include <gideon/cs/shared/data/CharacterTitleInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/VaultInfo.h>
#include <gideon/server/data/CharacterStats.h>
#include <sne/server/common/Property.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/database/Database.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace serverbase {

/**
 * @struct ProxyGameDatabase
 *
 * DB Proxy Server를 이용하는 Game Database.
 */
class ProxyGameDatabase : public sne::database::Database
{
public:
    // = 서버 설정 관련

    /// 키/값 속성들을 읽어온다
    virtual sne::base::Future::Ref getProperties() = 0;

    /// 서버 정보를 읽어온다
    virtual sne::base::Future::Ref getServerSpec(
        const std::string& serverName, const std::string& suffix) = 0;
public:
    // = 아이디 설정 관련
    virtual sne::base::Future::Ref getMaxInventoryId(ZoneId zoneId) = 0;

public:
    // = Shard 관련

    /// Shard 정보를 읽어온다
    virtual sne::base::Future::Ref getShardInfo(ShardId shardId) = 0;

    // = Zone 관련

    /// 특정 존 정보를 읽어온다
    virtual sne::base::Future::Ref getZoneInfo(const std::string& name) = 0;

    /// 전체 존 정보를 읽어온다
    virtual sne::base::Future::Ref getZoneInfoList(ShardId shardId) = 0;

    /// 존의 월드 시간을 얻어온다
    virtual sne::base::Future::Ref getWorldTime(ZoneId zoneId) = 0;

    /// 존의 월드 시간을 갱신한다
    virtual bool asyncUpdateWorldTime(ZoneId zoneId, WorldTime worldTime) = 0;

public:
    virtual void registGameMailCallback(GameMailCallback& gameMailCallback) = 0;
    virtual void registDBQueryPlayerCallback(DBQueryPlayerCallback& playerCallback) = 0;
	virtual void registBuddyCallback(CommunityBuddyCallback& buddyCallback) = 0;

public:
    // = 캐릭터 관련

    /// 캐릭터를 생성한다
	virtual sne::base::Future::Ref asyncGetFullUserInfo(const AccountInfo& accountInfo) = 0;
    virtual sne::base::Future::Ref asyncCreateCharacter(const CreateCharacterInfo& createCharacterInfo,
        const CreateCharacterEquipments& createCharacterEquipments, ZoneId zoneId,
        const ObjectPosition& position) = 0;

    /// 캐릭터를 삭제한다
    virtual sne::base::Future::Ref asyncDeleteCharacter(AccountId accountId,
        ObjectId characterId) = 0;
    
    /// 중복 닉네임을 체크 한다.
    virtual sne::base::Future::Ref asyncCheckDuplicateNickname(AccountId accountId,
        const Nickname& nickname) = 0;

    /// 캐릭터의 마지막 상태를 저장한다
    virtual void asyncSaveCharacterStats(AccountId accountId, ObjectId characterId,
        const DBCharacterStats& saveInfo) = 0;

    /// 캐릭터의 선택 칭호를 저장한다
    virtual void asyncSaveSelectCharacterTitle(AccountId accountId, ObjectId characterId,
        CharacterTitleCode titleCode) = 0;

    /// 캐릭터의 클라이언트 속성(config)을 읽어온다
    virtual sne::base::Future::Ref asyncLoadCharacterProperties(AccountId accountId,
        ObjectId characterId) = 0;

    /// 캐릭터의 클라이언트 속성(config)을 저장한다
    virtual void asyncSaveCharacterProperties(AccountId accountId, ObjectId characterId,
        const std::string& config, const std::string& prefs) = 0;

public:
    // = 인벤토리 관련

    /// 아이템 슬롯 변경
    virtual void asyncMoveInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId, SlotId slotId) = 0;

    /// 아이템 슬롯 교환
    virtual void asyncSwitchInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId1, ObjectId itemId2) = 0;

    /// 아이템 추가
    virtual void asyncAddInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, const ItemInfo& itemInfo) = 0;

    virtual void asyncChangeEquipItemInfo(AccountId accountId, ObjectId characterId, 
        ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount) = 0;

    virtual void asyncEnchantEquipItem(AccountId accountId, ObjectId characterId, 
        ObjectId itemId, SocketSlotId id, EquipSocketInfo& socketInfo) = 0;

	virtual void asyncUnenchantEquipItem(AccountId accountId, ObjectId characterId, 
		ObjectId itemId, SocketSlotId id) = 0;

    /// 아이템 삭제
    virtual void asyncRemoveInventoryItem(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId)  =0;

    /// 아이템 갯수 변화
    virtual void asyncUpdateInventoryItemCount(AccountId accountId, ObjectId characterId,
        InvenType invenType, ObjectId itemId1, uint8_t itemCount) = 0;

    virtual void asyncAddQuestItem(AccountId accountId, ObjectId characterId,
        const QuestItemInfo& questItemInfo) = 0;

    virtual void asyncRemoveQuestItem(AccountId accountId, ObjectId characterId,
        ObjectId questItemId) = 0;

    virtual void asyncUpdateQuestItemUsableCount(AccountId accountId, ObjectId characterId,
        ObjectId questItemId, uint8_t usableCount) = 0;

	virtual void asyncUpdateQuestItemCount(AccountId accountId, ObjectId characterId,
		ObjectId questItemId, uint8_t stackCount) = 0;

public:

    /// 장비 장착
    virtual void asyncEquipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, EquipPart equipPart) = 0;

    /// 장비 탈착
    virtual void asyncUnequipItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, EquipPart unequipPart) = 0;

    /// 장비 교환 (인벤 <-> 장착)
    virtual void asyncReplaceInventoryWithEquipItem(AccountId accountId,
        ObjectId characterId, ObjectId unequipItemId, EquipPart unequipIndex,
        ObjectId equipItemId, EquipPart equipIndex) = 0;

    virtual void asyncEquipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, AccessoryIndex equipIndex) = 0;

    /// 악세사리 탈착
    virtual void asyncUnequipAccessoryItem(AccountId accountId, ObjectId characterId,
        ObjectId itemId, SlotId slotId, AccessoryIndex unequipIndex) = 0;

    /// 악세사리 교환 (인벤 <-> 장착)
    virtual void asyncReplaceInventoryWithAccessoryItem(AccountId accountId,
        ObjectId characterId, ObjectId unequipItemId, AccessoryIndex unequipIndex,
        ObjectId equipItemId, AccessoryIndex equipIndex) = 0;


    // 케릭터 상태 변화(평화 or 전쟁)
    virtual void asyncChangeCharacterState(AccountId accountId,
        ObjectId characterId, CreatureStateType state) = 0;

    // 액션바 저장
    virtual void asyncSaveActionBar(AccountId accountId, ObjectId characterId, 
        ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code) = 0;

public:
    // 스킬 관련
    virtual void asyncLearnSkill(AccountId accountId, ObjectId characterId, 
        SkillCode currentSkillCode, SkillCode learnSkillCode) = 0;
    virtual void asyncRemoveSkill(AccountId accountId, ObjectId characterId, 
        SkillCode skillCode) = 0;
    virtual void asyncRemoveAllSkill(AccountId accountId, ObjectId characterId) = 0;

public:
	// 퀘스트 관련
	virtual void asyncAcceptedQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode) = 0;

    virtual void asyncAcceptRepeatQuest(AccountId accountId, ObjectId characterId, 
        QuestCode questCode, sec_t repeatAcceptTime) = 0;

	virtual void asyncCancelQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode, bool isRepeatQuest) = 0;

	virtual void asyncCompleteQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode, bool isRepeatQuest) = 0;

	virtual void asyncUpdateQuestMission(AccountId accountId, ObjectId characterId, 
		QuestCode questCode, QuestMissionCode missionCode, const QuestGoalInfo& goalInfo,
        bool isRepeatQuest) = 0;

	virtual void asyncRemoveCompleteQuest(AccountId accountId, ObjectId characterId, 
		QuestCode questCode) = 0;
public:
    virtual sne::base::Future::Ref getMaxGuildId() = 0;
    virtual sne::base::Future::Ref getGuildInfos() = 0;

    virtual void asyncCreateGuild(const BaseGuildInfo& guildInfo) = 0;
    virtual void asyncAddGuildMember(AccountId accountId, GuildId guildId,
        const GuildMemberInfo& memberInfo) = 0;
    virtual void asyncRemoveGuildMember(ObjectId characterId) = 0;
    virtual void asyncRemoveGuild(GuildId guildId) = 0;
    virtual void asyncAddGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId, GuildRelatioshipType type) = 0;
    virtual void asyncRemoveGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId) = 0;
	virtual void asyncChangeGuildMemberPosition(ObjectId characterId, GuildMemberPosition position) = 0;
    virtual void asyncAddGuildApplicant(ObjectId characterId, GuildId guildId) = 0;
    virtual void asyncRemoveGuildApplicant(ObjectId characterId) = 0;
    virtual void asyncModifyGuildIntroduction(GuildId guildId, const GuildIntroduction& introduction) = 0;
    virtual void asyncModifyGuildNotice(GuildId guildId, const GuildNotice& notice) = 0;
    virtual void asyncAddGuildRank(GuildId guildId, const GuildRankInfo& rankInfo, uint8_t vaultCount) = 0;
    virtual void asyncAddGuildBankRights(GuildId guildId, GuildRankId guildRankId, VaultId vaultId, const GuildBankVaultRightInfo& bankRankInfo) = 0;
    virtual void asyncDeleteGuildRank(GuildId guildId, GuildRankId rankId) = 0;
    virtual void asyncSwapGuildRank(GuildId guildId, GuildRankId rankId1, GuildRankId rankId2) = 0;
    virtual void asyncUpdateGuildRankName(GuildId guildId, GuildRankId rankId, const GuildRankName& rankName) = 0;
    virtual void asyncUpdateGuildRankRights(GuildId guildId, GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay) = 0;
    virtual void asyncUpdateGuildBankRights(GuildId guildId, GuildRankId rankId,
        VaultId vaultId, uint32_t rights, uint32_t itemWithdrawalPerDay) = 0;
    virtual void asyncUpdateGuildRank(GuildId guildId, ObjectId playerId, GuildRankId rankId) = 0;

    virtual void asyncRemoveAllGuildSkills(GuildId guildId) = 0;
    virtual void asyncRemoveGuildSkills(GuildId guildId, const SkillCodes& skillCodes) = 0;
    virtual void asyncAddGuildSkill(GuildId guildId, SkillCode skillCode) = 0;
    virtual void asyncUpdateGuildExpInfo(GuildId guildId, const GuildLevelInfo& guildLevelInfo) = 0;

    virtual void asyncMoveGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId, SlotId slotId) = 0;
    virtual void asyncSwitchGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId1, SlotId slotId1,
        ObjectId itemId2, SlotId slotId2) = 0;
    virtual void asyncAddGuildInventoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo) = 0;
    virtual void asyncRemoveGuildInventoryItem(GuildId guildId, ObjectId itemId) = 0;
    virtual void asyncUpdateGuildInventoryItemCount(GuildId guildId, ObjectId itemId, uint8_t itemCount) = 0;
    virtual void asyncUpdateGuildGameMoney(GuildId guildId, GameMoney gameMoney) = 0;
    virtual void asyncUpdateGuildMemberWithdraw(ObjectId playerId, GameMoney dayWithdraw, sec_t resetTime) = 0;
    virtual void asyncUpdateGuildVaultName(GuildId guildId, VaultId vaultId, const VaultName& name) = 0;
    virtual void asyncAddGuildVault(GuildId guildId, const VaultInfo& vaultInfo) = 0;

    virtual void asyncAddGuildEventLog(GuildId guildId, const GuildEventLogInfo& guildEventLog) = 0;
    virtual void asyncAddGuildBankEventLog(GuildId guildId, const GuildBankEventLogInfo& guildBankEventLog) = 0;
    virtual void asyncAddGuildGameMoneyEventLog(GuildId guildId, const GuildGameMoneyEventLogInfo& guildGameMoneyEventLog) = 0;

public:
    virtual void asyncSyncMail(ObjectId characterId) = 0;
    virtual void asyncSendMail(const Nickname& reveiverNickname, const MailInfo& maillInfo) = 0;
    virtual void asyncSendMail(ObjectId receiverId, const MailInfo& maillInfo) = 0;
    virtual void asyncAquireItemsInMail(MailId mailId) = 0;
    virtual void asyncDeleteMail(MailId mailId) = 0;
    virtual void asyncReadMail(MailId mailId) = 0;
    virtual void asyncQueryHasNotMail(ObjectId characterId) = 0;

public:
    virtual sne::base::Future::Ref getAuctionInfos(ZoneId zoneId) = 0;

    virtual void asyncCreateAuction(ZoneId zoneId, NpcCode npcCode, const FullAuctionInfo& auctionInfo) = 0;
    virtual void asyncDeleteAuction(ZoneId zoneId, AuctionId auctionId) = 0;
    virtual void asyncUpdateBid(ZoneId zoneId, AuctionId auctionId, ObjectId buyerId, GameMoney currentBidMoney) = 0;

public:
    virtual void asyncUpdateArenaRecord(AccountId accountId, ObjectId characterId, ArenaModeType arenaMode,
        uint32_t resultScore, ArenaResultType resultType) = 0;
    virtual void asyncUpdateArenaPoint(AccountId accountId, ObjectId characterId, ArenaPoint arenaPoint) = 0;
    virtual void asyncUpdateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime) = 0;

public:
    // = 빌딩 인벤토리 관련
    /// 캐릭터의 클라이언트 속성(config)을 읽어온다
    virtual sne::base::Future::Ref loadBuildingInfos(ZoneId zoneId) = 0;

    /// 아이템 슬롯 변경
    virtual void asyncMoveBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, ObjectId itemId, SlotId slotId) = 0;

    /// 아이템 슬롯 교환
    virtual void asyncSwitchBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, ObjectId itemId1, ObjectId itemId2,
        SlotId slotId1, SlotId slotId2) = 0;

    /// 아이템 추가
    virtual void asyncAddBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, const ItemInfo& itemInfo) = 0;

    /// 아이템 삭제
    virtual void asyncRemoveBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType, ObjectId itemId)  =0;

    /// 아이템 갯수 변화
    virtual void asyncUpdateBuildingInventoryItemCount(ObjectId buildingId,
        InvenType invenType, ObjectId itemId1, uint8_t itemCount) = 0;

    virtual void asyncRemoveAllBuildingInventoryItem(ObjectId buildingId,
        InvenType invenType) = 0;

    virtual void asyncAddBuildingGuard(ObjectId buildingId, const BuildingGuardInfo& guardInfo) = 0;
    virtual void asyncRemoveBuildingGuard(ObjectId guardId) = 0;

    virtual void asyncCreateBuilding(const BuildingInfo& buildingInfo) = 0;
    virtual void asyncDeleteBuilding(ObjectId buildingId) = 0;
    virtual void asyncUpdateBuildingState(ObjectId buildingId, BuildingStateType state,
        sec_t startBuildTime, sec_t expireTime, HitPoint currentHitPoint) = 0;
    virtual void asyncUpdateBuildingOwner(ObjectId buildingId, 
        BuildingOwnerType ownerType, ObjectId characterId, GuildId guildId) = 0;

    virtual void asyncAddBindRecall(AccountId accountId, ObjectId characterId, 
        const BindRecallInfo& bindRecallInfo) = 0;
    virtual void asyncRemoveBindRecall(AccountId accountId, ObjectId characterId, 
        ObjectId linkId) = 0;

    virtual void asyncAddSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
        uint8_t count, sec_t completeTime) = 0;
    virtual void asyncRemoveSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode) = 0;
    virtual void asyncUpdateSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
        uint8_t count, sec_t completeTime) = 0;

    virtual void asyncReleaseBeginnerProtection(AccountId accountId, ObjectId characterId) = 0;

    virtual void asyncAddCooldownInfos(AccountId accountId, ObjectId characterId, const CooltimeInfos& infos) = 0;
    virtual void asyncAddRemainEffects(AccountId accountId, ObjectId characterId, const RemainEffectInfos& infos) = 0;
    
    virtual void asyncUpdateCharacterInventoryInfo(AccountId accountId, ObjectId characterId, InvenType invenType,
        bool isCashSlot, uint8_t count) = 0;

	virtual void asyncCreateVehicle(AccountId accountId, ObjectId characterId, const VehicleInfo& info) = 0;
	virtual void asyncCreateGlider(AccountId accountId, ObjectId characterId, const GliderInfo& info) = 0;
	virtual void asyncDeleteVehicle(AccountId accountId, ObjectId characterId, ObjectId id) = 0;
	virtual void asyncDeleteGlider(AccountId accountId, ObjectId characterId, ObjectId id) = 0;
	virtual void asyncSelectVehicle(AccountId accountId, ObjectId characterId, ObjectId id) = 0;
	virtual void asyncSelectGlider(AccountId accountId, ObjectId characterId, ObjectId id) = 0;
	virtual void asyncUpdateGliderDurability(AccountId accountId, ObjectId characterId, ObjectId id, uint32_t currentValue) = 0;


	virtual void asyncGetBuddies(AccountId accountId, ObjectId characterId) = 0;
	virtual void asyncAddBuddy(ObjectId characterId1, ObjectId characterId2) = 0;
	virtual void asyncRemoveBuddy(ObjectId characterId1, ObjectId characterId2) = 0;
    virtual void asyncAddBlock(ObjectId ownerId, ObjectId characterId) = 0;
    virtual void asyncRemoveBlock(ObjectId ownerId, ObjectId characterId) = 0;

    virtual void asyncQueryAchievements(AccountId accountId, ObjectId characterId) = 0;
    virtual void asyncUpdateProcessAchievement(AccountId accountId, ObjectId characterId, AchievementCode code, const ProcessAchievementInfo& missionInfo) = 0;
    virtual void asyncCompleteAchievement(AccountId accountId, ObjectId characterId, AchievementCode code, AchievementPoint point) = 0;
    
    virtual void asyncQueryCharacterTitles(AccountId accountId, ObjectId characterId) = 0;
    virtual void asyncAddCharacterTitle(AccountId accountId, ObjectId characterId, CharacterTitleCode titleCode) = 0;

};

}} // namespace gideon { namespace serverbase {
