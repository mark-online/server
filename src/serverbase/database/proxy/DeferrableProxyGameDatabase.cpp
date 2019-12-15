#include "ServerBasePCH.h"
#include "DeferrableProxyGameDatabase.h"
#include <gideon/serverbase/app/BaseSessionServerApp.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/server/data/ServerAddress.h>
#include <gideon/cs/shared/data/ServerType.h>
#include <sne/database/proxy/DatabaseClientSideProxy.h>
#include <sne/server/s2s/ClientSideProxyFactory.h>
#include <sne/server/config/ConfigReader.h>
#include <sne/server/utility/Profiler.h>
#include <sne/server/common/Property.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/Session.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/extension/SessionExtension.h>
#include <sne/base/utility/Assert.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace serverbase {

using sne::database::proxy::RequestId;
using sne::base::Future;

namespace {

/**
 * @class DatabaseProxyClientSideRpcExtension
 */
class DatabaseProxyClientSideRpcExtension :
    public sne::base::AbstractSessionExtension<DatabaseProxyClientSideRpcExtension>,
    public rpc::DatabaseProxyRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(DatabaseProxyClientSideRpcExtension);

    SNE_GENERATE_SESSION_EXTENSION_ID(DatabaseProxyClientSideRpcExtension);

public:
    DatabaseProxyClientSideRpcExtension() :
        callback_(nullptr),
        mailCallback_(nullptr),
        buddyCallback_(nullptr) {}

    void initialize(DeferrableProxyGameDatabaseCallback& callback) {
        callback_ = &callback;
    }

    void registGameMailCallback(GameMailCallback* mailCallback) {
        mailCallback_ = mailCallback;
    }

    void registDBQueryPlayerCallback(DBQueryPlayerCallback* playerCallback) {
        playerCallback_ = playerCallback;
    }

    void registBuddyCallback(CommunityBuddyCallback* buddyCallback) {
        buddyCallback_ = buddyCallback;
    }

public:
    virtual void attachedTo(sne::base::SessionImpl& sessionImpl) override;

public:
    OVERRIDE_SRPC_METHOD_1(s2d_getProperties,
        RequestId, requestId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetProperties,
        RequestId, requestId, ErrorCode, errorCode,
        sne::server::Properties, properties);

    OVERRIDE_SRPC_METHOD_3(s2d_getServerSpec,
        RequestId, requestId, std::string, serverName, std::string, suffix);

    OVERRIDE_SRPC_METHOD_3(s2d_onGetServerSpec,
        RequestId, requestId, ErrorCode, errorCode, sne::server::ServerSpec, spec);
    
    OVERRIDE_SRPC_METHOD_2(s2d_getMaxInventoryId,
        RequestId, requestId, ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetMaxInventoryId,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, inventoryMaxId);

    OVERRIDE_SRPC_METHOD_2(s2d_getShardInfo,
        RequestId, requestId, ShardId, shardId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetShardInfo,
        RequestId, requestId, ErrorCode, errorCode, FullShardInfo, shardInfo);

    OVERRIDE_SRPC_METHOD_2(s2d_getZoneInfo,
        RequestId, requestId, std::string, name);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetZoneInfo,
        RequestId, requestId, ErrorCode, errorCode, ZoneInfo, zoneInfo);

    OVERRIDE_SRPC_METHOD_2(s2d_getZoneInfoList,
        RequestId, requestId, ShardId, shardId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetZoneInfoList,
        RequestId, requestId, ErrorCode, errorCode, ZoneInfos, zoneInfos);

    OVERRIDE_SRPC_METHOD_2(s2d_getWorldTime,
        RequestId, requestId, ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetWorldTime,
        RequestId, requestId, ErrorCode, errorCode, WorldTime, worldTime);

    OVERRIDE_SRPC_METHOD_2(s2d_getFullUserInfo,
        RequestId, requestId, AccountId, accountId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetFullUserInfo,
        RequestId, requestId, ErrorCode, errorCode, FullUserInfo, userInfo);

    OVERRIDE_SRPC_METHOD_5(s2d_createCharacter,
        RequestId, requestId, CreateCharacterInfo, createCharacterInfo, CreateCharacterEquipments, createCharacterEquipments, 
        ZoneId, zoneId, ObjectPosition, position);

    OVERRIDE_SRPC_METHOD_3(s2d_onCreateCharacter,
        RequestId, requestId, ErrorCode, errorCode, FullCharacterInfo, characterInfo);

    OVERRIDE_SRPC_METHOD_3(s2d_deleteCharacter,
        RequestId, requestId, AccountId, accountId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_4(s2d_onDeleteCharacter,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId, GuildId, guildId);
    
    OVERRIDE_SRPC_METHOD_3(s2d_checkDuplicateNickname,
        RequestId, requestId, AccountId, accountId, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_4(s2d_onCheckDuplicateNickname,
        RequestId, requestId, ErrorCode, errorCode, AccountId, accountId, Nickname, nickname);

    OVERRIDE_SRPC_METHOD_3(s2d_saveCharacterStats, 
        AccountId, accountId, ObjectId, characterId,
        DBCharacterStats, saveInfo);
    OVERRIDE_SRPC_METHOD_3(s2d_saveSelectCharacterTitle,
        AccountId, accountId, ObjectId, characterId,
        CharacterTitleCode, titleCode);

    OVERRIDE_SRPC_METHOD_3(s2d_loadCharacterProperties, 
        RequestId, requestId, AccountId, accountId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_5(s2d_onLoadCharacterProperties, 
        RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId,
        std::string, config, std::string, prefs);

    OVERRIDE_SRPC_METHOD_4(s2d_saveCharacterProperties, 
        AccountId, accountId, ObjectId, characterId,
        std::string, config, std::string, prefs);

    OVERRIDE_SRPC_METHOD_5(s2d_moveInventoryItem,
        AccountId, accountId, ObjectId, characterId,
        InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_5(s2d_switchInventoryItem,
        AccountId, accountId, ObjectId, characterId,
        InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_4(s2d_addInventoryItem,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType, ItemInfo, itemInfo);

    OVERRIDE_SRPC_METHOD_5(s2d_changeEquipItemInfo,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount);
    OVERRIDE_SRPC_METHOD_5(s2d_enchantEquipItem,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
        SocketSlotId, id, EquipSocketInfo, socketInfo);
    OVERRIDE_SRPC_METHOD_4(s2d_unenchantEquipItem,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId, SocketSlotId, id);

    OVERRIDE_SRPC_METHOD_4(s2d_removeInventoryItem,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType, ObjectId, objectId);
    OVERRIDE_SRPC_METHOD_5(s2d_updateInventoryItemCount,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType, ObjectId, objectId,
        uint8_t, itemCount);
    OVERRIDE_SRPC_METHOD_3(s2d_addQuestItem,
        AccountId, accountId, ObjectId, characterId, QuestItemInfo, questItemInfo);
    OVERRIDE_SRPC_METHOD_3(s2d_removeQuestItem,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_4(s2d_updateQuestItemUsableCount,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
        uint8_t, ueableCount);
    OVERRIDE_SRPC_METHOD_4(s2d_updateQuestItemCount,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
        uint8_t, stackCount);

    OVERRIDE_SRPC_METHOD_4(s2d_equipItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, EquipPart, equipPart);
    OVERRIDE_SRPC_METHOD_5(s2d_unequipItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, SlotId, slotId, EquipPart, unequipPart);
    OVERRIDE_SRPC_METHOD_6(s2d_replaceInventoryWithEquipItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, unequipItemId, EquipPart, unequipPart, ObjectId, equipItemId,
        EquipPart, equipPart);

    OVERRIDE_SRPC_METHOD_4(s2d_equipAccessoryItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, AccessoryIndex, equipPart);
    OVERRIDE_SRPC_METHOD_5(s2d_unequipAccessoryItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPart);
    OVERRIDE_SRPC_METHOD_6(s2d_replaceInventoryWithAccessoryItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, unequipItemId, AccessoryIndex, unequipPart, ObjectId, equipItemId,
        AccessoryIndex, equipPart);

    OVERRIDE_SRPC_METHOD_3(s2d_changeCharacterState,
        AccountId, accountId, ObjectId, characterId, CreatureStateType, state);

    OVERRIDE_SRPC_METHOD_5(s2d_saveActionBar,
        AccountId, accountId, ObjectId, characterId, ActionBarIndex, abiIndex, 
        ActionBarPosition, abpIndex, DataCode, code);

    OVERRIDE_SRPC_METHOD_4(s2d_learnSkill,
        AccountId, accountId, ObjectId, characterId, SkillCode, currentSkillCode, 
        SkillCode, learnSkillCode);
    OVERRIDE_SRPC_METHOD_3(s2d_removeSkill,
        AccountId, accountId, ObjectId, characterId, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_2(s2d_removeAllSkill,
        AccountId, accountId, ObjectId, characterId);

    OVERRIDE_SRPC_METHOD_3(s2d_acceptQuest,
        AccountId, accountId, ObjectId, characterId, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_4(s2d_acceptRepeatQuest,
        AccountId, accountId, ObjectId, characterId, QuestCode, questCode, sec_t, acceptRepeatTime);
    OVERRIDE_SRPC_METHOD_4(s2d_cancelQuest,
        AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest);
    OVERRIDE_SRPC_METHOD_4(s2d_completeQuest,
        AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest);
    OVERRIDE_SRPC_METHOD_6(s2d_updateQuestMission,
        AccountId, accountId, ObjectId, characterId, QuestCode, questCode,
        QuestMissionCode, missionCode, QuestGoalInfo, goalInfo, bool, isRepeatQuest);
    OVERRIDE_SRPC_METHOD_3(s2d_removeCompleteQuest,
        AccountId, accountId, ObjectId, characterId, QuestCode, questCode);

    OVERRIDE_SRPC_METHOD_1(s2d_getGuildInfos,
        RequestId, requestId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetGuildInfos,
        RequestId, requestId, ErrorCode, errorCode, GuildInfos, guildInfos);
    OVERRIDE_SRPC_METHOD_1(s2d_getMaxGuildId,
        RequestId, requestId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetMaxGuildId,
        RequestId, requestId, ErrorCode, errorCode, GuildId, guildId);

    OVERRIDE_SRPC_METHOD_1(s2d_createGuild,
        BaseGuildInfo, guildInfo);
    OVERRIDE_SRPC_METHOD_3(s2d_addGuildMember,
        AccountId, accountId, GuildId, guildId, GuildMemberInfo, memberInfo);
    OVERRIDE_SRPC_METHOD_1(s2d_removeGuildMember,
        ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_1(s2d_removeGuild,
        GuildId, guildId);
    OVERRIDE_SRPC_METHOD_3(s2d_addGuildRelationship,
        GuildId, ownerGuildId, GuildId, targetGuildId, GuildRelatioshipType, type);
    OVERRIDE_SRPC_METHOD_2(s2d_removeGuildRelationship,
        GuildId, ownerGuildId, GuildId, targetGuildId);
    OVERRIDE_SRPC_METHOD_2(s2d_changeGuildMemberPosition,
        ObjectId, characterId, GuildMemberPosition, position);
    OVERRIDE_SRPC_METHOD_2(s2d_addGuildApplicant,
        ObjectId, characterId, GuildId, guildId);
    OVERRIDE_SRPC_METHOD_1(s2d_removeGuildApplicant,
        ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_2(s2d_modifyGuildIntroduction,
        GuildId, guildId, GuildIntroduction, introduction);
    OVERRIDE_SRPC_METHOD_2(s2d_modifyGuildNotice,
        GuildId, guildId, GuildNotice, notice);
    OVERRIDE_SRPC_METHOD_3(s2d_addGuildRank,
        GuildId, guildId, GuildRankInfo, rankInfo, uint8_t, vaultCount);
    OVERRIDE_SRPC_METHOD_4(s2d_addGuilBankRank,
        GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, GuildBankVaultRightInfo, bankRankInfo);
    OVERRIDE_SRPC_METHOD_2(s2d_deleteGuildRank,
        GuildId, guildId, GuildRankId, rankId);
    OVERRIDE_SRPC_METHOD_3(s2d_swapGuildRank,
        GuildId, guildId, GuildRankId, rankId1, GuildRankId, rankId2);
    OVERRIDE_SRPC_METHOD_3(s2d_updateGuildRankName,
        GuildId, guildId, GuildRankId, rankId, GuildRankName, rankName);
    OVERRIDE_SRPC_METHOD_4(s2d_updateGuildRankRights,
        GuildId, guildId, GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_5(s2d_updateGuildBankRights,
        GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_3(s2d_updateGuildRank,
        GuildId, guildId, ObjectId, playerId, GuildRankId, rankId);

    OVERRIDE_SRPC_METHOD_1(s2d_removeAllGuildSkills,
        GuildId, guildId);
    OVERRIDE_SRPC_METHOD_2(s2d_removeGuildSkills,
        GuildId, guildId, SkillCodes, skillCodes);
    OVERRIDE_SRPC_METHOD_2(s2d_addGuildSkill,
        GuildId, guildId, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_2(s2d_updateGuildExp,
        GuildId, guildId, GuildLevelInfo, guildLevelInfo);

    OVERRIDE_SRPC_METHOD_4(s2d_moveGuildInventoryItem,
        GuildId, guildId, VaultId, vaultId, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_6(s2d_switchGuildInventoryItem,
        GuildId, guildId, VaultId, vaultId, ObjectId, itemId1, SlotId, slotId1, ObjectId, itemId2, SlotId, slotId2);
    OVERRIDE_SRPC_METHOD_3(s2d_addGuildInventoryItem,
        GuildId, guildId, VaultId, vaultId, ItemInfo, itemInfo);

    OVERRIDE_SRPC_METHOD_2(s2d_removeGuildInventoryItem,
        GuildId, guildId, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_3(s2d_updateGuildInventoryItemCount,
        GuildId, guildId, ObjectId, itemId, uint8_t, count);
    OVERRIDE_SRPC_METHOD_2(s2d_updateGuildGameMoney,
        GuildId, guildId, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_3(s2d_updateGuildMemberWithdraw,
        ObjectId, playerId, GameMoney, dayWithdraw, sec_t, resetTime);
    OVERRIDE_SRPC_METHOD_3(s2d_updateGuildVaultName,
        GuildId, guildId, VaultId, vaultId, VaultName, name);
    OVERRIDE_SRPC_METHOD_2(s2d_addGuildVault,
        GuildId, guildId, VaultInfo, vaultInfo);

    OVERRIDE_SRPC_METHOD_1(s2d_syncMail,
        ObjectId, characterId);    
    OVERRIDE_SRPC_METHOD_2(s2d_sendMailByNickname,
        Nickname, reveiverNickname, MailInfo, maillInfo);
    OVERRIDE_SRPC_METHOD_2(s2d_sendMailByPlayerId,
        ObjectId, playerId, MailInfo, maillInfo);
    OVERRIDE_SRPC_METHOD_1(s2d_aquireItemsInMail,
        MailId, mailId);
    OVERRIDE_SRPC_METHOD_1(s2d_deleteMail,
        MailId, mailId);
    OVERRIDE_SRPC_METHOD_1(s2d_readMail,
        MailId, mailId);
    OVERRIDE_SRPC_METHOD_1(s2d_queryNotReadMail,
        ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_2(s2d_onNotReadMail,
        ObjectId, characterId, bool, hasNotReadMail);

    OVERRIDE_SRPC_METHOD_2(s2d_onSyncMail,
        ObjectId, characterId, MailInfos, mailInfos);
    OVERRIDE_SRPC_METHOD_2(s2d_evMailReceived,
        ObjectId, receiverId, MailInfo, maillInfo);

    OVERRIDE_SRPC_METHOD_2(s2d_getAuctionInfos,
        RequestId, requestId, ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_3(s2d_onAuctionInfos,
        RequestId, requestId, ErrorCode, errorCode, ServerAuctionInfoMap, serverAuctionInfoMap);

    OVERRIDE_SRPC_METHOD_3(s2d_createAuction,
        ZoneId, zoneId, NpcCode, npcCode, FullAuctionInfo, auctionInfo);
    OVERRIDE_SRPC_METHOD_2(s2d_deleteAuction,
        ZoneId, zoneId, AuctionId, auctionId);
    OVERRIDE_SRPC_METHOD_4(s2d_updateBid,
        ZoneId, zoneId, AuctionId, auctionId, ObjectId, buyerId, GameMoney, currentBidMoney);

    OVERRIDE_SRPC_METHOD_5(s2d_updateArenaRecord,
        AccountId, accountId, ObjectId, characterId, ArenaModeType, arenaMode,
        uint32_t, resultScore, ArenaResultType, resultType);
    OVERRIDE_SRPC_METHOD_3(s2d_updateArenaPoint,
        AccountId, accountId, ObjectId, characterId, ArenaPoint, arenaPoint);
    OVERRIDE_SRPC_METHOD_2(s2d_updateDeserterExpireTime,
        ObjectId, characterId, sec_t, deserterExpireTime);

    OVERRIDE_SRPC_METHOD_4(s2d_moveBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_6(s2d_switchBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2,
        SlotId, slot1, SlotId, slot2);
    OVERRIDE_SRPC_METHOD_3(s2d_addBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_3(s2d_removeBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ObjectId, objectId);
    OVERRIDE_SRPC_METHOD_4(s2d_updateBuildingInventoryItemCount,
        ObjectId, buildingId, InvenType, invenType, ObjectId, objectId, uint8_t, itemCount);
    OVERRIDE_SRPC_METHOD_2(s2d_removeAllBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType);
    OVERRIDE_SRPC_METHOD_2(s2d_addBuildingGuard,
        ObjectId, buildingId, BuildingGuardInfo, info);
    OVERRIDE_SRPC_METHOD_1(s2d_removeBuildingGuard,
        ObjectId, guardId);
    
    OVERRIDE_SRPC_METHOD_1(s2d_createBuilding,
        BuildingInfo, buildingInfo);
    OVERRIDE_SRPC_METHOD_1(s2d_deleteBuilding,
        ObjectId, buildingId);
    OVERRIDE_SRPC_METHOD_5(s2d_updateBuildingState,
        ObjectId, buildingId, BuildingStateType, state, sec_t, startBuildTime,
        sec_t, expireTime, HitPoint, currentHitPoint);
    OVERRIDE_SRPC_METHOD_4(s2d_updateBuildingOwner,
        ObjectId, buildingId, BuildingOwnerType, ownerType, ObjectId, playerId, GuildId, guildId);

    OVERRIDE_SRPC_METHOD_3(s2d_addBindRecallInfo,
        AccountId, accountId, ObjectId, characterId, BindRecallInfo, bindRecallInfo);
    OVERRIDE_SRPC_METHOD_3(s2d_removeBindRecallIInfo,
        AccountId, accountId, ObjectId, characterId, ObjectId, linkId);

    OVERRIDE_SRPC_METHOD_4(s2d_addSelectRecipeProduction,
        ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime);
    OVERRIDE_SRPC_METHOD_2(s2d_removeSelectRecipeProduction,
        ObjectId, buildingId, RecipeCode, recipeCode);
    OVERRIDE_SRPC_METHOD_4(s2d_updateSelectRecipeProduction,
        ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime);
    
    OVERRIDE_SRPC_METHOD_2(s2d_releaseBeginnerProtection,
        AccountId, accountId, ObjectId, characterId);

    OVERRIDE_SRPC_METHOD_2(s2d_loadBuildingInfo,
        RequestId, requestId, ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_4(s2d_onLoadBuildingInfo,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, maxBuildingId,
        BuildingInfoMap, buildingInfoMap);

    OVERRIDE_SRPC_METHOD_3(s2d_addCooldownInfos,
        AccountId, accountId, ObjectId, characterId, CooltimeInfos, infos);
    OVERRIDE_SRPC_METHOD_3(s2d_addRemainEffects,
        AccountId, accountId, ObjectId, characterId, RemainEffectInfos, infos);

    OVERRIDE_SRPC_METHOD_5(s2d_updateCharacterInventoryInfo,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType,
        bool, isCashSlot, uint8_t, count);

    OVERRIDE_SRPC_METHOD_3(s2d_createVehicle,
        AccountId, accountId, ObjectId, characterId, VehicleInfo, info);
    OVERRIDE_SRPC_METHOD_3(s2d_createGlider,
        AccountId, accountId, ObjectId, characterId, GliderInfo, info);
    OVERRIDE_SRPC_METHOD_3(s2d_deleteVehicle,
        AccountId, accountId, ObjectId, characterId, ObjectId, id);
    OVERRIDE_SRPC_METHOD_3(s2d_deleteGlider,
        AccountId, accountId, ObjectId, characterId, ObjectId, id);

    OVERRIDE_SRPC_METHOD_3(s2d_selectVehicle,
        AccountId, accountId, ObjectId, characterId, ObjectId, id);
    OVERRIDE_SRPC_METHOD_3(s2d_selectGlider,
        AccountId, accountId, ObjectId, characterId, ObjectId, id);

    OVERRIDE_SRPC_METHOD_4(s2d_updateGliderDurability,
        AccountId, accountId, ObjectId, characterId, ObjectId, id, uint32_t, currentValue);

    OVERRIDE_SRPC_METHOD_2(s2d_getBuddies,
        AccountId, accountId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_2(s2d_addBuddy,
        ObjectId, characterId1, ObjectId, characterId2);
    OVERRIDE_SRPC_METHOD_2(s2d_removeBuddy,
        ObjectId, characterId1, ObjectId, characterId2);
    OVERRIDE_SRPC_METHOD_2(s2d_addBlock,
        ObjectId, ownerId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_2(s2d_removeBlock,
        ObjectId, ownerId, ObjectId, characterId);

    OVERRIDE_SRPC_METHOD_3(s2d_onGetBuddies,
        AccountId, accountId, BuddyInfos, buddyInfos, BlockInfos, blockInfos);
    
    OVERRIDE_SRPC_METHOD_2(s2d_queryAchievements,
        AccountId, accountId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_4(s2d_onQueryAchievements,
        ErrorCode, errorCode, ObjectId, characterId, ProcessAchievementInfoMap, processInfoMap,
        CompleteAchievementInfoMap, completeInfoMap);

    OVERRIDE_SRPC_METHOD_4(s2d_updateProcessAchievement,
        AccountId, accountId, ObjectId, characterId, AchievementCode, code, ProcessAchievementInfo, missionInfo);
    OVERRIDE_SRPC_METHOD_4(s2d_completeAchievement,
        AccountId, accountId, ObjectId, characterId, AchievementCode, code, AchievementPoint, point);
    OVERRIDE_SRPC_METHOD_3(s2d_addCharacterTitle,
        AccountId, accountId, ObjectId, characterId, CharacterTitleCode, titleCode);
    
    OVERRIDE_SRPC_METHOD_2(s2d_queryCharacterTitles,
        AccountId, accountId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_3(s2d_onQueryCharacterTitles,
        ErrorCode, errorCode, ObjectId, characterId, CharacterTitleCodeSet, titleCodeSet);

private:
    DeferrableProxyGameDatabaseCallback* callback_;
    GameMailCallback* mailCallback_;
    CommunityBuddyCallback* buddyCallback_;
    DBQueryPlayerCallback* playerCallback_;
};

IMPLEMENT_SRPC_EVENT_DISPATCHER(DatabaseProxyClientSideRpcExtension);

void DatabaseProxyClientSideRpcExtension::attachedTo(
    sne::base::SessionImpl& sessionImpl)
{
    Parent::attachedTo(sessionImpl);

    sne::sgp::RpcingExtension* extension =
        sessionImpl.getExtension<sne::sgp::RpcingExtension>();
    extension->registerRpcForwarder(*this);
    extension->registerRpcReceiver(*this);
}


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_getProperties,
    RequestId, requestId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetProperties,
   RequestId, requestId, ErrorCode, errorCode,
   sne::server::Properties, properties)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetPropertiesRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetPropertiesRequestFuture& requestFuture =
        static_cast<GetPropertiesRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.properties_ = properties;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_getServerSpec,
    RequestId, requestId, std::string, serverName, std::string, suffix);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetServerSpec,
    RequestId, requestId, ErrorCode, errorCode, sne::server::ServerSpec, spec)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetServerSpecRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetServerSpecRequestFuture& requestFuture =
        static_cast<GetServerSpecRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.serverSpec_ = spec;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getMaxInventoryId,
    RequestId, reequestId, ZoneId, zoneId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetMaxInventoryId,
    RequestId, requestId, ErrorCode, errorCode, ObjectId, inventoryMaxId)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetMaxInventoryIdRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetMaxItemInventoryIdRequestFuture& requestFuture =
        static_cast<GetMaxItemInventoryIdRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.maxInventoryId_ = inventoryMaxId;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getShardInfo,
    RequestId, requestId, ShardId, shardId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetShardInfo,
    RequestId, requestId, ErrorCode, errorCode, FullShardInfo, shardInfo)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetShardInfoRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetShardInfoRequestFuture& requestFuture =
        static_cast<GetShardInfoRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.shardInfo_ = shardInfo;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getZoneInfo,
    RequestId, requestId, std::string, name);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetZoneInfo,
    RequestId, requestId, ErrorCode, errorCode, ZoneInfo, zoneInfo)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetZoneInfoRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetZoneInfoRequestFuture& requestFuture =
        static_cast<GetZoneInfoRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.zoneInfo_ = zoneInfo;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getZoneInfoList,
    RequestId, requestId, ShardId, shardId);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getWorldTime,
    RequestId, requestId, ZoneId, zoneId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetZoneInfoList,
    RequestId, requestId, ErrorCode, errorCode, ZoneInfos, zoneInfos)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetZoneInfoListRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetZoneInfoListRequestFuture& requestFuture =
        static_cast<GetZoneInfoListRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.zoneInfos_ = zoneInfos;
    requestFuture.completed();
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetWorldTime,
    RequestId, requestId, ErrorCode, errorCode, WorldTime, worldTime)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetWorldTimeRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetWorldTimeRequestFuture& requestFuture =
        static_cast<GetWorldTimeRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.worldTime_ = worldTime;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getFullUserInfo,
    RequestId, requestId, AccountId, accountId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetFullUserInfo,
    RequestId, requestId, ErrorCode, errorCode, FullUserInfo, userInfo)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetFullUserInfoRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetFullUserInfoRequestFuture& requestFuture =
        static_cast<GetFullUserInfoRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.userInfo_ = userInfo;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_createCharacter,
    RequestId, requestId, CreateCharacterInfo, createCharacterInfo, CreateCharacterEquipments, createCharacterEquipments, 
    ZoneId, zoneId, ObjectPosition, position);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onCreateCharacter,
    RequestId, requestId, ErrorCode, errorCode, FullCharacterInfo, characterInfo)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("CreateCharacterRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    CreateCharacterRequestFuture& requestFuture =
        static_cast<CreateCharacterRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.characterInfo_ = characterInfo;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_deleteCharacter,
    RequestId, requestId, AccountId, accountId, ObjectId, characterId);


RECEIVE_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_onDeleteCharacter,
    RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId, GuildId, guildId)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("DeleteCharacterRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    DeleteCharacterRequestFuture& requestFuture =
        static_cast<DeleteCharacterRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.characterId_ = characterId;
    requestFuture.guildId_ = guildId;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_checkDuplicateNickname,
    RequestId, requestId, AccountId, accountId, Nickname, nickname);


RECEIVE_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_onCheckDuplicateNickname,
    RequestId, requestId, ErrorCode, errorCode, AccountId, accountId, Nickname, nickname)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("DeleteCharacterRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    CheckDuplicateNicknameRequestFuture& requestFuture =
        static_cast<CheckDuplicateNicknameRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.accountId_ = accountId;
    requestFuture.nickname_ = nickname;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_saveCharacterStats, 
    AccountId, accountId, ObjectId, characterId,
    DBCharacterStats, saveInfo);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_saveSelectCharacterTitle,
    AccountId, accountId, ObjectId, characterId,
    CharacterTitleCode, titleCode);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_loadCharacterProperties, 
    RequestId, requestId, AccountId, accountId, ObjectId, characterId);


RECEIVE_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_onLoadCharacterProperties, 
    RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId,
    std::string, config, std::string, prefs)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("LoadCharacterPropertiesRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    LoadCharacterPropertiesRequestFuture& requestFuture =
        static_cast<LoadCharacterPropertiesRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.characterId_ = characterId;
    requestFuture.config_ = config;
    requestFuture.prefs_ = prefs;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_saveCharacterProperties, 
    AccountId, accountId, ObjectId, characterId,
    std::string, config, std::string, prefs);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_moveInventoryItem,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, 
    ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_switchInventoryItem,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, 
    ObjectId, itemId1, ObjectId, itemId2);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_addInventoryItem,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, ItemInfo, itemInfo);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_changeEquipItemInfo,
   AccountId, accountId, ObjectId, characterId, ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_enchantEquipItem,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
    SocketSlotId, id, EquipSocketInfo, socketInfo);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_unenchantEquipItem,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId, SocketSlotId, id);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_removeInventoryItem,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, ObjectId, objectId);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_updateInventoryItemCount,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, ObjectId, objectId,
    uint8_t, itemCount);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addQuestItem,
    AccountId, accountId, ObjectId, characterId, QuestItemInfo, questItemInfo);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_removeQuestItem,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateQuestItemUsableCount,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
    uint8_t, ueableCount);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateQuestItemCount,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
    uint8_t, stackCount);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_equipItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, EquipPart, equipPart);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_unequipItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, SlotId, slotId, EquipPart, unequipPart);


FORWARD_SRPC_METHOD_6(DatabaseProxyClientSideRpcExtension, s2d_replaceInventoryWithEquipItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, unequipItemId, EquipPart, unequipPart, ObjectId, equipItemId,
    EquipPart, equipPart);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_equipAccessoryItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, AccessoryIndex, equipPart);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_unequipAccessoryItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPart);


FORWARD_SRPC_METHOD_6(DatabaseProxyClientSideRpcExtension, s2d_replaceInventoryWithAccessoryItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, unequipItemId, AccessoryIndex, unequipPart, ObjectId, equipItemId,
    AccessoryIndex, equipPart);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_changeCharacterState,
    AccountId, accountId, ObjectId, characterId, CreatureStateType, state);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_saveActionBar,
    AccountId, accountId, ObjectId, characterId, ActionBarIndex, abiIndex,
    ActionBarPosition, abpIndex, DataCode, code);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_learnSkill,
    AccountId, accountId, ObjectId, characterId, SkillCode, currentSkillCode, 
    SkillCode, learnSkillCode);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_removeSkill,
    AccountId, accountId, ObjectId, characterId, SkillCode, skillCode);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeAllSkill,
    AccountId, accountId, ObjectId, characterId);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_acceptQuest,
    AccountId, accountId, ObjectId, characterId, QuestCode, questCode);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_acceptRepeatQuest,
    AccountId, accountId, ObjectId, characterId, QuestCode, questCode, sec_t, acceptRepeatTime);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_cancelQuest,
    AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_completeQuest,
    AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest);


FORWARD_SRPC_METHOD_6(DatabaseProxyClientSideRpcExtension, s2d_updateQuestMission,
    AccountId, accountId, ObjectId, characterId, QuestCode, questCode,
    QuestMissionCode, missionCode, QuestGoalInfo, goalInfo, bool, isRepeatQuest);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_removeCompleteQuest,
    AccountId, accountId, ObjectId, characterId, QuestCode, questCode);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_getGuildInfos,
    RequestId, requestId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetGuildInfos,
     RequestId, requestId, ErrorCode, errorCode, GuildInfos, guildInfos)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetGuildInfosRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetGuildInfosRequestFuture& requestFuture =
        static_cast<GetGuildInfosRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.guildInfos_ = guildInfos;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_getMaxGuildId,
    RequestId, requestId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetMaxGuildId,
    RequestId, requestId, ErrorCode, errorCode, GuildId, guildId)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetMaxGuildIdRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetMaxGuildIdRequestFuture& requestFuture =
        static_cast<GetMaxGuildIdRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.maxGuildId_ = guildId;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_createGuild,
    BaseGuildInfo, guildInfo);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addGuildMember,
    AccountId, accountId, GuildId, guildId, GuildMemberInfo, memberInfo);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addGuildRelationship,
    GuildId, ownerGuildId, GuildId, targetGuildId, GuildRelatioshipType, type);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_changeGuildMemberPosition,
    ObjectId, characterId, GuildMemberPosition, position);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_addGuildApplicant,
    ObjectId, characterId, GuildId, guildId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_removeGuildApplicant,
    ObjectId, characterId);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_modifyGuildIntroduction,
    GuildId, guildId, GuildIntroduction, introduction);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_modifyGuildNotice,
    GuildId, guildId, GuildNotice, notice);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addGuildRank,
    GuildId, guildId, GuildRankInfo, rankInfo, uint8_t, vaultCount);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_addGuilBankRank,
    GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, GuildBankVaultRightInfo, bankRankInfo);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_deleteGuildRank,
    GuildId, guildId, GuildRankId, rankId);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_swapGuildRank,
    GuildId, guildId, GuildRankId, rankId1, GuildRankId, rankId2);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_updateGuildRankName,
    GuildId, guildId, GuildRankId, rankId, GuildRankName, rankName);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateGuildRankRights,
    GuildId, guildId, GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_updateGuildBankRights,
    GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_updateGuildRank,
    GuildId, guildId, ObjectId, playerId, GuildRankId, rankId);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeGuildSkills,
    GuildId, guildId, SkillCodes, skillCodes);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_removeAllGuildSkills,
    GuildId, guildId);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_addGuildSkill,
    GuildId, guildId, SkillCode, skillCode);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_updateGuildExp,
    GuildId, guildId, GuildLevelInfo, guildLevelInfo);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_moveGuildInventoryItem,
    GuildId, guildId, VaultId, vaultId, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_6(DatabaseProxyClientSideRpcExtension, s2d_switchGuildInventoryItem,
    GuildId, guildId, VaultId, vaultId, ObjectId, itemId1, SlotId, slotId1, ObjectId, itemId2, SlotId, slotId2);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addGuildInventoryItem,
    GuildId, guildId, VaultId, vaultId, ItemInfo, itemInfo);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeGuildInventoryItem,
    GuildId, guildId, ObjectId, itemId);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_updateGuildInventoryItemCount,
    GuildId, guildId, ObjectId, itemId, uint8_t, count);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_updateGuildGameMoney,
    GuildId, guildId, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_updateGuildMemberWithdraw,
    ObjectId, playerId, GameMoney, dayWithdraw, sec_t, resetTime);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension,s2d_updateGuildVaultName,
    GuildId, guildId, VaultId, vaultId, VaultName, name);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_addGuildVault,
    GuildId, guildId, VaultInfo, vaultInfo);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeGuildRelationship,
    GuildId, ownerGuildId, GuildId, targetGuildId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_removeGuildMember,
    ObjectId, characterId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_removeGuild,
    GuildId, guildId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_syncMail,
     ObjectId, characterId);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_sendMailByNickname,
    Nickname, reveiverNickname, MailInfo, maillInfo);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_sendMailByPlayerId,
    ObjectId, playerId, MailInfo, maillInfo);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_aquireItemsInMail,
    MailId, mailId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_deleteMail,
    MailId, mailId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_readMail,
    MailId, mailId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_queryNotReadMail,
    ObjectId, characterId);


RECEIVE_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_onNotReadMail,
    ObjectId, characterId, bool, hasNotReadMail)
{
    if (mailCallback_ && hasNotReadMail) {
        mailCallback_->onNotReadMail(characterId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_onSyncMail,
    ObjectId, characterId, MailInfos, mailInfos)
{
    if (mailCallback_) {
        mailCallback_->onSyncMail(characterId, mailInfos);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_evMailReceived,
    ObjectId, receiverId, MailInfo, maillInfo)
{
    if (mailCallback_) {
        mailCallback_->onReceive(receiverId, maillInfo);
    }
}

FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getAuctionInfos,
    RequestId, requestId, ZoneId, zoneId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onAuctionInfos,
    RequestId, requestId, ErrorCode, errorCode, ServerAuctionInfoMap, serverAuctionInfoMap)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("GetAuctionInfosRequestFuture not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    GetAuctionInfosRequestFuture& requestFuture =
        static_cast<GetAuctionInfosRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.auctionMap_ = serverAuctionInfoMap;
    requestFuture.completed();
}


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_createAuction,
    ZoneId, zoneId, NpcCode, npcCode, FullAuctionInfo, auctionInfo);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_deleteAuction,
    ZoneId, zoneId, AuctionId, auctionId);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateBid,
    ZoneId, zoneId, AuctionId, auctionId, ObjectId, buyerId, GameMoney, currentBidMoney);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_updateArenaRecord,
    AccountId, accountId, ObjectId, characterId, ArenaModeType, arenaMode,
    uint32_t, resultScore, ArenaResultType, resultType);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_updateArenaPoint,
    AccountId, accountId, ObjectId, characterId, ArenaPoint, arenaPoint);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_updateDeserterExpireTime,
    ObjectId, characterId, sec_t, deserterExpireTime);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_moveBuildingInventoryItem,
    ObjectId, buildingId, InvenType, invenType, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_6(DatabaseProxyClientSideRpcExtension, s2d_switchBuildingInventoryItem,
    ObjectId, buildingId, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2,
    SlotId ,slot1, SlotId, slot2);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addBuildingInventoryItem,
    ObjectId, buildingId, InvenType, invenType, ItemInfo, itemInfo);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_removeBuildingInventoryItem,
    ObjectId, buildingId, InvenType, invenType, ObjectId, objectId);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateBuildingInventoryItemCount,
    ObjectId, buildingId, InvenType, invenType, ObjectId, objectId, uint8_t, itemCount);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeAllBuildingInventoryItem,
    ObjectId, buildingId, InvenType, invenType);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_addBuildingGuard,
    ObjectId, buildingId, BuildingGuardInfo, info);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_removeBuildingGuard,
    ObjectId, guardId);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_deleteBuilding,
    ObjectId, buildingId);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_updateBuildingState,
    ObjectId, buildingId, BuildingStateType, state, sec_t, startBuildTime,
    sec_t, expireTime, HitPoint, currentHitPoint);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateBuildingOwner,
    ObjectId, buildingId, BuildingOwnerType, ownerType, ObjectId, playerId, GuildId, guildId);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addBindRecallInfo,
    AccountId, accountId, ObjectId, characterId, BindRecallInfo, bindRecallInfo);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_removeBindRecallIInfo,
    AccountId, accountId, ObjectId, characterId, ObjectId, linkId);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_addSelectRecipeProduction,
    ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeSelectRecipeProduction,
    ObjectId, buildingId, RecipeCode, recipeCode);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateSelectRecipeProduction,
    ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_releaseBeginnerProtection,
    AccountId, accountId, ObjectId, characterId);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addCooldownInfos,
    AccountId, accountId, ObjectId, characterId, CooltimeInfos, infos);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addRemainEffects,
    AccountId, accountId, ObjectId, characterId, RemainEffectInfos, infos);


FORWARD_SRPC_METHOD_5(DatabaseProxyClientSideRpcExtension, s2d_updateCharacterInventoryInfo,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType,
    bool, isCashSlot, uint8_t, count);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_createVehicle,
    AccountId, accountId, ObjectId, characterId, VehicleInfo, info);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_createGlider,
    AccountId, accountId, ObjectId, characterId, GliderInfo, info);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_deleteVehicle,
    AccountId, accountId, ObjectId, characterId, ObjectId, id);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_deleteGlider,
    AccountId, accountId, ObjectId, characterId, ObjectId, id);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_selectVehicle,
    AccountId, accountId, ObjectId, characterId, ObjectId, id);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_selectGlider,
    AccountId, accountId, ObjectId, characterId, ObjectId, id);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateGliderDurability,
    AccountId, accountId, ObjectId, characterId, ObjectId, id, uint32_t, currentValue);


FORWARD_SRPC_METHOD_1(DatabaseProxyClientSideRpcExtension, s2d_createBuilding,
    BuildingInfo, buildingInfo);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_loadBuildingInfo,
    RequestId, requestId, ZoneId, zoneId);


RECEIVE_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_onLoadBuildingInfo,
    RequestId, requestId, ErrorCode, errorCode, ObjectId, maxBuildingId,
    BuildingInfoMap, buildingInfoMap)
{
    Future::Ref future = callback_->getFuture(requestId);
    if (! future) {
        SNE_LOG_ERROR("LoadBuildingInfo not found(" __FUNCTION__ ", %d)",
            requestId);
        return;
    }

    LoadBuildingRequestFuture& requestFuture =
        static_cast<LoadBuildingRequestFuture&>(*future);
    requestFuture.errorCode_ = errorCode;
    requestFuture.maxBuildingId_ = maxBuildingId;
    requestFuture.buildingInfoMap_= buildingInfoMap;
    requestFuture.completed();
}

FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_getBuddies,
    AccountId, accountId, ObjectId, characterId);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_addBuddy,
    ObjectId, characterId1, ObjectId, characterId2);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeBuddy,
    ObjectId, characterId1, ObjectId, characterId2);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_addBlock,
    ObjectId, ownerId, ObjectId, characterId);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_removeBlock,
    ObjectId, ownerId, ObjectId, characterId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onGetBuddies,
    AccountId, accountId, BuddyInfos, buddyInfos, BlockInfos, blockInfos)
{
    if (buddyCallback_) {
        buddyCallback_->buddiesReceived(accountId, buddyInfos, blockInfos);
    }
}


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_queryAchievements,
    AccountId, accountId, ObjectId, characterId);


RECEIVE_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_onQueryAchievements,
    ErrorCode, errorCode, ObjectId, characterId, ProcessAchievementInfoMap, processInfoMap,
    CompleteAchievementInfoMap, completeInfoMap)
{
    if (playerCallback_) {
        playerCallback_->onQueryAchievements(errorCode, characterId, processInfoMap, completeInfoMap);
    }
}


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_updateProcessAchievement,
    AccountId, accountId, ObjectId, characterId, AchievementCode, code, ProcessAchievementInfo, missionInfo);


FORWARD_SRPC_METHOD_4(DatabaseProxyClientSideRpcExtension, s2d_completeAchievement,
    AccountId, accountId, ObjectId, characterId, AchievementCode, code, AchievementPoint, point);


FORWARD_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_addCharacterTitle,
    AccountId, accountId, ObjectId, characterId, CharacterTitleCode, titleCode);


FORWARD_SRPC_METHOD_2(DatabaseProxyClientSideRpcExtension, s2d_queryCharacterTitles,
    AccountId, accountId, ObjectId, characterId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyClientSideRpcExtension, s2d_onQueryCharacterTitles,
    ErrorCode, errorCode, ObjectId, characterId, CharacterTitleCodeSet, titleCodeSet)
{
    if (playerCallback_) {
        playerCallback_->onQueryCharacterTitles(errorCode, characterId, titleCodeSet);
    }
}

} // namespace

// = DeferrableProxyGameDatabase

DeferrableProxyGameDatabase::DeferrableProxyGameDatabase(
    sne::base::IoContextTask& ioServiceTask) :
    ioServiceTask_(ioServiceTask),
    rpcExtension_(nullptr)
{
}


bool DeferrableProxyGameDatabase::open(sne::server::ConfigReader& configReader)
{
    sne::server::Profiler profiler(__FUNCTION__);

    sne::server::ServerInfo serverInfo;
    serverInfo.ip_ = configReader.getString("database", "proxy-address");
    serverInfo.port_ =
        configReader.getNumeric<uint16_t>("database", "proxy-port");
    serverInfo.certificate_ =
        configReader.getString("database", "proxy-certificate");
    serverInfo.packetCipher_ =
        configReader.getString("database", "proxy-packet-cipher");
    serverInfo.heartbeatInterval_ =
        configReader.getNumeric<uint16_t>("database",
            "proxy-heartbeat-interval");
    serverInfo.description_ = getDatabaseProxyServerName();

    const msec_t connectionTimeout =
        configReader.getNumeric<msec_t>("database", "connection-timeout");

    if (! initDatabaseProxy(serverInfo, connectionTimeout)) {
        return false;
    }

    initRpc();
    return true;
}


void DeferrableProxyGameDatabase::close()
{
    if (databaseProxy_) {
        databaseProxy_->disconnect();
    }
}


bool DeferrableProxyGameDatabase::initDatabaseProxy(
    const sne::server::ServerInfo& serverInfo, msec_t connectionTimeout)
{
    databaseProxy_ =
        sne::server::ClientSideProxyFactory<
            sne::database::proxy::DatabaseClientSideProxy>::create(
                serverInfo, ioServiceTask_);

    databaseProxy_->setConnectionTimeout(connectionTimeout * 1000);
    if (! databaseProxy_->connect()) {
        SNE_LOG_ERROR("Failed to connect DatabaseClientSideProxy Server(%s:%d)",
            serverInfo.ip_.c_str(), serverInfo.port_);
        return false;
    }

    SNE_LOG_INFO("Connected to DatabaseClientSideProxy Server(%s:%d)",
        serverInfo.ip_.c_str(), serverInfo.port_);
    return true;
}


void DeferrableProxyGameDatabase::initRpc()
{
    assert(databaseProxy_.get() != nullptr);

    rpcExtension_ = 
        databaseProxy_->getSession()->getImpl().
            registerExtension<DatabaseProxyClientSideRpcExtension>();
    rpcExtension_->initialize(*this);
}


RequestId DeferrableProxyGameDatabase::pushFuture(
    Future::Ref future)
{
    // TODO: 요청이 무한정 쌓이는 것을 방지해야 한다

    std::unique_lock<std::mutex> lock(lock_);

    const RequestId requestId = ++currentRequestId_;

    requestFutureMap_.emplace(requestId, future);
    return requestId;
}


Future::Ref DeferrableProxyGameDatabase::popFuture(
    RequestId requestId)
{
    std::unique_lock<std::mutex> lock(lock_);

    RequestFutureMap::iterator pos = requestFutureMap_.find(requestId);
    if (pos == requestFutureMap_.end()) {
        return Future::Ref();
    }

    Future::Ref future = (*pos).second;
    requestFutureMap_.erase(requestId);
    return future;
}

// = ProxyGameDatabase overriding

Future::Ref DeferrableProxyGameDatabase::getProperties()
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetPropertiesRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getProperties(requestId);
    return future;
}


Future::Ref DeferrableProxyGameDatabase::getServerSpec(
    const std::string& serverName, const std::string& suffix)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetServerSpecRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getServerSpec(requestId, serverName, suffix);
    return future;
}


Future::Ref DeferrableProxyGameDatabase::getMaxInventoryId(ZoneId zoneId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetMaxItemInventoryIdRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getMaxInventoryId(requestId, zoneId);
    return future;
}


Future::Ref DeferrableProxyGameDatabase::getShardInfo(ShardId shardId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetShardInfoRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getShardInfo(requestId, shardId);
    return future;
}


Future::Ref DeferrableProxyGameDatabase::getZoneInfo(
    const std::string& name)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetZoneInfoRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getZoneInfo(requestId, name);
    return future;
}


Future::Ref DeferrableProxyGameDatabase::getZoneInfoList(ShardId shardId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetZoneInfoListRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getZoneInfoList(requestId, shardId);
    return future;
}


Future::Ref DeferrableProxyGameDatabase::getWorldTime(ZoneId zoneId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetWorldTimeRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getWorldTime(requestId, zoneId);
    return future;
}


bool DeferrableProxyGameDatabase::asyncUpdateWorldTime(ZoneId zoneId, WorldTime worldTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return false;
    }

    sne::database::proxy::ModifyRequest request(
        sne::database::proxy::rpLow,
        sne::database::proxy::rtStoredProcedure,
        "dbo.update_world_time");
    request << zoneId << worldTime;
    databaseProxy_->call(request);
    return true;
}


void DeferrableProxyGameDatabase::registGameMailCallback(GameMailCallback& gameMailCallback)
{
    rpcExtension_->registGameMailCallback(&gameMailCallback);
}


void DeferrableProxyGameDatabase::registDBQueryPlayerCallback(DBQueryPlayerCallback& playerCallback) 
{
    rpcExtension_->registDBQueryPlayerCallback(&playerCallback);
}


void DeferrableProxyGameDatabase::registBuddyCallback(CommunityBuddyCallback& buddyCallback)
{
    rpcExtension_->registBuddyCallback(&buddyCallback);
}


Future::Ref DeferrableProxyGameDatabase::asyncGetFullUserInfo(
    const AccountInfo& accountInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidAccountId(accountInfo.accountId_)) {
        return Future::Ref();
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetFullUserInfoRequestFuture>(accountInfo);

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getFullUserInfo(requestId, accountInfo.accountId_);
    return future;
}


Future::Ref DeferrableProxyGameDatabase::asyncCreateCharacter(const CreateCharacterInfo& createCharacterInfo,
    const CreateCharacterEquipments& createCharacterEquipments, ZoneId zoneId, const ObjectPosition& position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidAccountId(createCharacterInfo.accountId_)) {
        return Future::Ref();
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<CreateCharacterRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_createCharacter(requestId, createCharacterInfo, createCharacterEquipments, zoneId, position);
    return future;
}


sne::base::Future::Ref DeferrableProxyGameDatabase::asyncDeleteCharacter(
    AccountId accountId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidAccountId(accountId)) {
        return Future::Ref();
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<DeleteCharacterRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_deleteCharacter(requestId, accountId, characterId);
    return future;
}


sne::base::Future::Ref DeferrableProxyGameDatabase::asyncCheckDuplicateNickname(AccountId accountId,
    const Nickname& nickname)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidAccountId(accountId)) {
        return Future::Ref();
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<CheckDuplicateNicknameRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_checkDuplicateNickname(requestId, accountId, nickname);
    return future;
}


void DeferrableProxyGameDatabase::asyncSaveCharacterStats(AccountId accountId, ObjectId characterId,
    const DBCharacterStats& saveInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_saveCharacterStats(accountId, characterId, saveInfo);
}


void DeferrableProxyGameDatabase::asyncSaveSelectCharacterTitle(AccountId accountId, ObjectId characterId,
    CharacterTitleCode titleCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_saveSelectCharacterTitle(accountId, characterId, titleCode);
}    


sne::base::Future::Ref DeferrableProxyGameDatabase::asyncLoadCharacterProperties(
    AccountId accountId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidAccountId(accountId)) {
        return Future::Ref();
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<LoadCharacterPropertiesRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_loadCharacterProperties(requestId, accountId, characterId);
    return future;
}


void DeferrableProxyGameDatabase::asyncSaveCharacterProperties(AccountId accountId, ObjectId characterId,
    const std::string& config, const std::string& prefs)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_saveCharacterProperties(accountId, characterId, config, prefs);
}


void DeferrableProxyGameDatabase::asyncMoveInventoryItem(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId, SlotId slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_moveInventoryItem(accountId, characterId, invenType, itemId, slotId);
}


void DeferrableProxyGameDatabase::asyncSwitchInventoryItem(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId1, ObjectId itemId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_switchInventoryItem(accountId, characterId, invenType, 
        itemId1, itemId2);
}


void DeferrableProxyGameDatabase::asyncAddInventoryItem(AccountId accountId,
    ObjectId characterId, InvenType invenType, const ItemInfo& itemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addInventoryItem(accountId, characterId, invenType, itemInfo);
}


void DeferrableProxyGameDatabase::asyncChangeEquipItemInfo(AccountId accountId, ObjectId characterId, 
    ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_changeEquipItemInfo(accountId, characterId, itemId, newEquipCode, socketCount);
}


void DeferrableProxyGameDatabase::asyncEnchantEquipItem(AccountId accountId, ObjectId characterId, 
    ObjectId itemId, SocketSlotId id, EquipSocketInfo& socketInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_enchantEquipItem(accountId, characterId, itemId, id, socketInfo);
}


void DeferrableProxyGameDatabase::asyncUnenchantEquipItem(AccountId accountId, ObjectId characterId, 
    ObjectId itemId, SocketSlotId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_unenchantEquipItem(accountId, characterId, itemId, id);
}


void DeferrableProxyGameDatabase::asyncRemoveInventoryItem(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(itemId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeInventoryItem(accountId, characterId, invenType, itemId);
}


void DeferrableProxyGameDatabase::asyncUpdateInventoryItemCount(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId, uint8_t itemCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateInventoryItemCount(accountId, characterId, invenType, itemId, itemCount);
}


void DeferrableProxyGameDatabase::asyncAddQuestItem(AccountId accountId, ObjectId characterId,
    const QuestItemInfo& questItemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addQuestItem(accountId, characterId, questItemInfo);
}


void DeferrableProxyGameDatabase::asyncRemoveQuestItem(AccountId accountId, ObjectId characterId,
    ObjectId questItemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeQuestItem(accountId, characterId, questItemId);
}


void DeferrableProxyGameDatabase::asyncUpdateQuestItemUsableCount(AccountId accountId, ObjectId characterId,
    ObjectId questItemId, uint8_t usableCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateQuestItemUsableCount(accountId, characterId, questItemId, usableCount);
}


void DeferrableProxyGameDatabase::asyncUpdateQuestItemCount(AccountId accountId, ObjectId characterId,
    ObjectId questItemId, uint8_t stackCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateQuestItemCount(accountId, characterId, questItemId, stackCount);
}


void DeferrableProxyGameDatabase::asyncEquipItem(AccountId accountId,
    ObjectId characterId, ObjectId itemId, EquipPart equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_equipItem(accountId, characterId, itemId, equipPart);
}


void DeferrableProxyGameDatabase::asyncUnequipItem(AccountId accountId,
    ObjectId characterId, ObjectId itemId, SlotId slotId,
    EquipPart unequipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_unequipItem(accountId, characterId,
        itemId, slotId, unequipPart);
}


void DeferrableProxyGameDatabase::asyncReplaceInventoryWithEquipItem(
    AccountId accountId, ObjectId characterId,
    ObjectId unequipItemId, EquipPart unequipPart, ObjectId equipItemId,
    EquipPart equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_replaceInventoryWithEquipItem(accountId, characterId,
        unequipItemId, unequipPart, equipItemId, equipPart);
}


void DeferrableProxyGameDatabase::asyncEquipAccessoryItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, AccessoryIndex equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_equipAccessoryItem(accountId, characterId, itemId, equipPart);
}


void DeferrableProxyGameDatabase::asyncUnequipAccessoryItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_unequipAccessoryItem(accountId, characterId,
        itemId, slotId, unequipPart);
}



void DeferrableProxyGameDatabase::asyncReplaceInventoryWithAccessoryItem(AccountId accountId,
    ObjectId characterId, ObjectId unequipItemId, AccessoryIndex unequipPart,
    ObjectId equipItemId, AccessoryIndex equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_replaceInventoryWithAccessoryItem(accountId, characterId,
        unequipItemId, unequipPart, equipItemId, equipPart);
}


void DeferrableProxyGameDatabase::asyncChangeCharacterState(AccountId accountId,
    ObjectId characterId, CreatureStateType state)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_changeCharacterState(accountId, characterId, state);
}


void DeferrableProxyGameDatabase::asyncSaveActionBar(AccountId accountId, ObjectId characterId, 
    ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_saveActionBar(accountId, characterId, abiIndex, abpIndex, code);
}


void DeferrableProxyGameDatabase::asyncLearnSkill(AccountId accountId,
    ObjectId characterId, SkillCode currentSkillCode, SkillCode learnSkillCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_learnSkill(accountId, characterId, currentSkillCode, learnSkillCode);
}


void DeferrableProxyGameDatabase::asyncRemoveSkill(AccountId accountId,
    ObjectId characterId, SkillCode skillCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeSkill(accountId, characterId, skillCode);
}


void DeferrableProxyGameDatabase::asyncRemoveAllSkill(AccountId accountId,
    ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeAllSkill(accountId, characterId);
}


void DeferrableProxyGameDatabase::asyncAcceptedQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_acceptQuest(accountId, characterId, questCode);
}


void DeferrableProxyGameDatabase::asyncAcceptRepeatQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, sec_t repeatAcceptTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_acceptRepeatQuest(accountId, characterId, questCode, repeatAcceptTime);
}


void DeferrableProxyGameDatabase::asyncCancelQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, bool isRepeatQuest)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_cancelQuest(accountId, characterId, questCode, isRepeatQuest);
}


void DeferrableProxyGameDatabase::asyncCompleteQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, bool isRepeatQuest)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_completeQuest(accountId, characterId, questCode, isRepeatQuest);
}


void DeferrableProxyGameDatabase::asyncUpdateQuestMission(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, QuestMissionCode missionCode, const QuestGoalInfo& goalInfo, bool isRepeatQuest)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateQuestMission(accountId, characterId, questCode, missionCode, goalInfo, isRepeatQuest);
}


void DeferrableProxyGameDatabase::asyncRemoveCompleteQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeCompleteQuest(accountId, characterId, questCode);
}


sne::base::Future::Ref DeferrableProxyGameDatabase::getMaxGuildId()
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetMaxGuildIdRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getMaxGuildId(requestId);
    return future;
}


sne::base::Future::Ref DeferrableProxyGameDatabase::getGuildInfos()
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetGuildInfosRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getGuildInfos(requestId);
    return future;
}


void DeferrableProxyGameDatabase::asyncCreateGuild(const BaseGuildInfo& guildInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_createGuild(guildInfo);
}


void DeferrableProxyGameDatabase::asyncAddGuildMember(AccountId accountId, GuildId guildId,
    const GuildMemberInfo& memberInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(memberInfo.playerId_)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addGuildMember(accountId, guildId, memberInfo);
}


void DeferrableProxyGameDatabase::asyncRemoveGuildMember(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeGuildMember(characterId);
}


void DeferrableProxyGameDatabase::asyncRemoveGuild(GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeGuild(guildId);
}


void DeferrableProxyGameDatabase::asyncAddGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId, GuildRelatioshipType type)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(ownerGuildId) && ! isValidGuildId(targetGuildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addGuildRelationship(ownerGuildId, targetGuildId, type);
}


void DeferrableProxyGameDatabase::asyncRemoveGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(ownerGuildId) && ! isValidGuildId(targetGuildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeGuildRelationship(ownerGuildId, targetGuildId);
}



void DeferrableProxyGameDatabase::asyncChangeGuildMemberPosition(ObjectId characterId, GuildMemberPosition position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_changeGuildMemberPosition(characterId, position);
}


void DeferrableProxyGameDatabase::asyncAddGuildApplicant(ObjectId characterId, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addGuildApplicant(characterId, guildId);
}


void DeferrableProxyGameDatabase::asyncRemoveGuildApplicant(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeGuildApplicant(characterId);
}


void DeferrableProxyGameDatabase::asyncModifyGuildIntroduction(GuildId guildId, const GuildIntroduction& introduction)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_modifyGuildIntroduction(guildId, introduction);
}


void DeferrableProxyGameDatabase::asyncModifyGuildNotice(GuildId guildId, const GuildNotice& notice)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_modifyGuildNotice(guildId, notice);
}


void DeferrableProxyGameDatabase::asyncAddGuildRank(GuildId guildId, const GuildRankInfo& rankInfo, uint8_t vaultCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addGuildRank(guildId, rankInfo, vaultCount);
}


void DeferrableProxyGameDatabase::asyncAddGuildBankRights(GuildId guildId, GuildRankId guildRankId, VaultId vaultId, const GuildBankVaultRightInfo& bankRankInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addGuilBankRank(guildId, guildRankId, vaultId, bankRankInfo);
}


void DeferrableProxyGameDatabase::asyncDeleteGuildRank(GuildId guildId, GuildRankId rankId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_deleteGuildRank(guildId, rankId);
}


void DeferrableProxyGameDatabase::asyncSwapGuildRank(GuildId guildId, GuildRankId rankId1, GuildRankId rankId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_swapGuildRank(guildId, rankId1, rankId2);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildRankName(GuildId guildId,
    GuildRankId rankId, const GuildRankName& rankName)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }


    rpcExtension_->s2d_updateGuildRankName(guildId, rankId, rankName);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildRankRights(GuildId guildId, GuildRankId rankId,
    uint32_t rights, uint32_t goldWithdrawalPerDay)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGuildRankRights(guildId, rankId, rights, goldWithdrawalPerDay);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildBankRights(GuildId guildId, GuildRankId rankId,
    VaultId vaultId, uint32_t rights, uint32_t itemWithdrawalPerDay)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGuildBankRights(guildId, rankId, vaultId, rights, itemWithdrawalPerDay);
}



void DeferrableProxyGameDatabase::asyncUpdateGuildRank(GuildId guildId, ObjectId playerId, GuildRankId rankId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId) ) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGuildRank(guildId, playerId, rankId);
}


void DeferrableProxyGameDatabase::asyncRemoveGuildSkills(GuildId guildId, const SkillCodes& skillCodes)
{
    sne::server::Profiler profiler(__FUNCTION__);
    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_removeGuildSkills(guildId, skillCodes);
}


void DeferrableProxyGameDatabase::asyncRemoveAllGuildSkills(GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);
    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_removeAllGuildSkills(guildId);
}


void DeferrableProxyGameDatabase::asyncAddGuildSkill(GuildId guildId, SkillCode skillCode)
{
    sne::server::Profiler profiler(__FUNCTION__);
    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_addGuildSkill(guildId, skillCode);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildExpInfo(GuildId guildId, const GuildLevelInfo& guildLevelInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);
    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_updateGuildExp(guildId, guildLevelInfo);
}


void DeferrableProxyGameDatabase::asyncMoveGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId, SlotId slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);
    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_moveGuildInventoryItem(guildId, vaultId, itemId, slotId);
}


void DeferrableProxyGameDatabase::asyncSwitchGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId1,
    SlotId slotId1, ObjectId itemId2, SlotId slotId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_switchGuildInventoryItem(guildId, vaultId, itemId1, slotId1, itemId2, slotId2);
}


void DeferrableProxyGameDatabase::asyncAddGuildInventoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addGuildInventoryItem(guildId, vaultId, itemInfo);
}


void DeferrableProxyGameDatabase::asyncRemoveGuildInventoryItem(GuildId guildId, ObjectId itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeGuildInventoryItem(guildId, itemId);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildInventoryItemCount(GuildId guildId, ObjectId itemId, uint8_t itemCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGuildInventoryItemCount(guildId, itemId, itemCount);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildGameMoney(GuildId guildId, GameMoney gameMoney)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGuildGameMoney(guildId, gameMoney);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildMemberWithdraw(ObjectId playerId,
    GameMoney dayWithdraw, sec_t resetTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(playerId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGuildMemberWithdraw(playerId, dayWithdraw, resetTime);
}


void DeferrableProxyGameDatabase::asyncUpdateGuildVaultName(GuildId guildId,
    VaultId vaultId, const VaultName& name)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGuildVaultName(guildId, vaultId, name);
}

void DeferrableProxyGameDatabase::asyncAddGuildVault(GuildId guildId, const VaultInfo& vaultInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addGuildVault(guildId, vaultInfo);
}


void DeferrableProxyGameDatabase::asyncAddGuildEventLog(GuildId guildId, const GuildEventLogInfo& guildEventLog)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    sne::database::proxy::ModifyRequest request(
        sne::database::proxy::rpLow,
        sne::database::proxy::rtStoredProcedure,
        "dbo.add_guild_event_log");

    request << guildEventLog.id_ << guildId << guildEventLog.logType_ << guildEventLog.playerId1_ << guildEventLog.playerId2_ <<
        guildEventLog.param1_;
    databaseProxy_->call(request);
}


void DeferrableProxyGameDatabase::asyncAddGuildBankEventLog(GuildId guildId, const GuildBankEventLogInfo& guildBankEventLog)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    sne::database::proxy::ModifyRequest request(
        sne::database::proxy::rpLow,
        sne::database::proxy::rtStoredProcedure,
        "dbo.add_guild_bank_event_log");

    request << guildBankEventLog.id_ << guildId << guildBankEventLog.vaultId_ << guildBankEventLog.logType_ << guildBankEventLog.playerId_ <<
        guildBankEventLog.param1_ << guildBankEventLog.param2_;
    databaseProxy_->call(request);
}


void DeferrableProxyGameDatabase::asyncAddGuildGameMoneyEventLog(GuildId guildId, const GuildGameMoneyEventLogInfo& guildGameMoneyEventLog)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidGuildId(guildId)) {
        assert(false);
        return;
    }

    sne::database::proxy::ModifyRequest request(
        sne::database::proxy::rpLow,
        sne::database::proxy::rtStoredProcedure,
        "dbo.add_guild_game_money_event_log");

    request << guildGameMoneyEventLog.id_ << guildId << guildGameMoneyEventLog.logType_ << guildGameMoneyEventLog.playerId_ <<
        guildGameMoneyEventLog.gameMoney_;
    databaseProxy_->call(request);
}


void DeferrableProxyGameDatabase::asyncSyncMail(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_syncMail(characterId);
}


void DeferrableProxyGameDatabase::asyncSendMail(const Nickname& reveiverNickname, const MailInfo& maillInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_sendMailByNickname(reveiverNickname, maillInfo);
}


void DeferrableProxyGameDatabase::asyncSendMail(ObjectId receiverId, const MailInfo& maillInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_sendMailByPlayerId(receiverId, maillInfo);
}


void DeferrableProxyGameDatabase::asyncAquireItemsInMail(MailId mailId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidMailId(mailId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_aquireItemsInMail(mailId);
}


void DeferrableProxyGameDatabase::asyncDeleteMail(MailId mailId) 
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidMailId(mailId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_deleteMail(mailId);
}


void DeferrableProxyGameDatabase::asyncReadMail(MailId mailId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidMailId(mailId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_readMail(mailId);
}


void DeferrableProxyGameDatabase::asyncQueryHasNotMail(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_queryNotReadMail(characterId);
}


sne::base::Future::Ref DeferrableProxyGameDatabase::getAuctionInfos(ZoneId zoneId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<GetAuctionInfosRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_getAuctionInfos(requestId, zoneId);
    return future;
}


void DeferrableProxyGameDatabase::asyncCreateAuction(ZoneId zoneId, NpcCode npcCode, const FullAuctionInfo& auctionInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidZoneId(zoneId) && ! isValidNpcCode(npcCode)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_createAuction(zoneId, npcCode, auctionInfo);
}


void DeferrableProxyGameDatabase::asyncDeleteAuction(ZoneId zoneId, AuctionId auctionId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidZoneId(zoneId) && ! isValidAuctionId(auctionId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_deleteAuction(zoneId, auctionId);
}


void DeferrableProxyGameDatabase::asyncUpdateBid(ZoneId zoneId, AuctionId auctionId, ObjectId buyerId, GameMoney currentBidMoney)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidZoneId(zoneId) && ! isValidAuctionId(auctionId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_updateBid(zoneId, auctionId, buyerId, currentBidMoney);
}


void DeferrableProxyGameDatabase::asyncUpdateArenaRecord(AccountId accountId, ObjectId characterId,
    ArenaModeType arenaMode, uint32_t resultScore, ArenaResultType resultType)
{
    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_updateArenaRecord(accountId, characterId, arenaMode, resultScore, resultType);    
}


void DeferrableProxyGameDatabase::asyncUpdateArenaPoint(AccountId accountId, ObjectId characterId, ArenaPoint arenaPoint)
{
    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_updateArenaPoint(accountId, characterId, arenaPoint);
}


void DeferrableProxyGameDatabase::asyncUpdateDeserterExpireTime(
    ObjectId characterId, sec_t deserterExpireTime)
{
    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_updateDeserterExpireTime(characterId, deserterExpireTime);
}


sne::base::Future::Ref DeferrableProxyGameDatabase::loadBuildingInfos(ZoneId zoneId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return Future::Ref();
    }

    auto future = std::make_shared<LoadBuildingRequestFuture>();

    const RequestId requestId = pushFuture(future);
    assert(requestId != sne::database::proxy::invalidRequestId);

    rpcExtension_->s2d_loadBuildingInfo(requestId, zoneId);
    return future;
}


void DeferrableProxyGameDatabase::asyncMoveBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, ObjectId itemId, SlotId slotId)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_moveBuildingInventoryItem(buildingId, invenType, itemId, slotId);
}


void DeferrableProxyGameDatabase::asyncSwitchBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, ObjectId itemId1, ObjectId itemId2,
    SlotId slotId1, SlotId slotId2)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_switchBuildingInventoryItem(buildingId, invenType, itemId1, itemId2, slotId1, slotId2);
}

void DeferrableProxyGameDatabase::asyncAddBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, const ItemInfo& itemInfo)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addBuildingInventoryItem(buildingId, invenType, itemInfo);
}


void DeferrableProxyGameDatabase::asyncRemoveBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, ObjectId itemId)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeBuildingInventoryItem(buildingId, invenType, itemId);
}


void DeferrableProxyGameDatabase::asyncUpdateBuildingInventoryItemCount(ObjectId buildingId,
    InvenType invenType, ObjectId itemId, uint8_t itemCount)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateBuildingInventoryItemCount(buildingId, invenType, itemId, itemCount);
}


void DeferrableProxyGameDatabase::asyncRemoveAllBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeAllBuildingInventoryItem(buildingId, invenType);
}


void DeferrableProxyGameDatabase::asyncAddBuildingGuard(ObjectId buildingId, const BuildingGuardInfo& guardInfo)
{
    if (! isValidObjectId(guardInfo.objectId_)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addBuildingGuard(buildingId, guardInfo);
}


void DeferrableProxyGameDatabase::asyncRemoveBuildingGuard(ObjectId guardId)
{
    if (! isValidObjectId(guardId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeBuildingGuard(guardId);
}


void DeferrableProxyGameDatabase::asyncCreateBuilding(const BuildingInfo& buildingInfo)
{
    if (! isValidObjectId(buildingInfo.objectId_)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_createBuilding(buildingInfo);
}


void DeferrableProxyGameDatabase::asyncDeleteBuilding(ObjectId buildingId)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_deleteBuilding(buildingId);
}


void DeferrableProxyGameDatabase::asyncUpdateBuildingState(ObjectId buildingId, BuildingStateType state,
    sec_t startBuildTime, sec_t expireTime, HitPoint currentHitPoint)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_updateBuildingState(buildingId, state, startBuildTime, expireTime, currentHitPoint);
}


void DeferrableProxyGameDatabase::asyncUpdateBuildingOwner(ObjectId buildingId, 
    BuildingOwnerType ownerType, ObjectId characterId, GuildId guildId)
{
    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }
    rpcExtension_->s2d_updateBuildingOwner(buildingId, ownerType, characterId, guildId);
}


void DeferrableProxyGameDatabase::asyncAddBindRecall(AccountId accountId, ObjectId characterId, 
    const BindRecallInfo& bindRecallInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addBindRecallInfo(accountId, characterId, bindRecallInfo);
}


void DeferrableProxyGameDatabase::asyncRemoveBindRecall(AccountId accountId, ObjectId characterId, 
    ObjectId linkId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeBindRecallIInfo(accountId, characterId, linkId);
}


void DeferrableProxyGameDatabase::asyncAddSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
    uint8_t count, sec_t completeTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addSelectRecipeProduction(buildingId, recipeCode, count, completeTime);
}


void DeferrableProxyGameDatabase::asyncRemoveSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeSelectRecipeProduction(buildingId, recipeCode);
}


void DeferrableProxyGameDatabase::asyncUpdateSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
    uint8_t count, sec_t completeTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(buildingId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateSelectRecipeProduction(buildingId, recipeCode, count, completeTime);
}


void DeferrableProxyGameDatabase::asyncReleaseBeginnerProtection(AccountId accountId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_releaseBeginnerProtection(accountId, characterId);
}


void DeferrableProxyGameDatabase::asyncAddCooldownInfos(AccountId accountId, ObjectId characterId, const CooltimeInfos& infos)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addCooldownInfos(accountId, characterId, infos);
}


void DeferrableProxyGameDatabase::asyncAddRemainEffects(AccountId accountId, ObjectId characterId, const RemainEffectInfos& infos)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addRemainEffects(accountId, characterId, infos);
}


void DeferrableProxyGameDatabase::asyncUpdateCharacterInventoryInfo(AccountId accountId, ObjectId characterId, InvenType invenType,
    bool isCashSlot, uint8_t count)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateCharacterInventoryInfo(accountId, characterId, invenType, isCashSlot, count);
}


void DeferrableProxyGameDatabase::asyncCreateVehicle(AccountId accountId, ObjectId characterId, const VehicleInfo& info)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_createVehicle(accountId, characterId, info);
}


void DeferrableProxyGameDatabase::asyncCreateGlider(AccountId accountId, ObjectId characterId, const GliderInfo& info)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_createGlider(accountId, characterId, info);
}


void DeferrableProxyGameDatabase::asyncDeleteVehicle(AccountId accountId, ObjectId characterId, ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_deleteVehicle(accountId, characterId, id);
}


void DeferrableProxyGameDatabase::asyncDeleteGlider(AccountId accountId, ObjectId characterId, ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_deleteGlider(accountId, characterId, id);
}


void DeferrableProxyGameDatabase::asyncSelectVehicle(AccountId accountId, ObjectId characterId, ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_selectVehicle(accountId, characterId, id);
}


void DeferrableProxyGameDatabase::asyncSelectGlider(AccountId accountId, ObjectId characterId, ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_selectGlider(accountId, characterId, id);
}


void DeferrableProxyGameDatabase::asyncUpdateGliderDurability(AccountId accountId, ObjectId characterId, ObjectId id, uint32_t currentValue)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(characterId)) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateGliderDurability(accountId, characterId, id, currentValue);
}


void DeferrableProxyGameDatabase::asyncGetBuddies(AccountId accountId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidAccountId(accountId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_getBuddies(accountId, characterId);
}


void DeferrableProxyGameDatabase::asyncAddBuddy(ObjectId characterId1, ObjectId characterId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidObjectId(characterId1) && isValidObjectId(characterId2))) {
            assert(false);
            return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addBuddy(characterId1, characterId2);
}


void DeferrableProxyGameDatabase::asyncRemoveBuddy(ObjectId characterId1, ObjectId characterId2) 
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidObjectId(characterId1) && isValidObjectId(characterId2))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeBuddy(characterId1, characterId2);
}


void DeferrableProxyGameDatabase::asyncAddBlock(ObjectId ownerId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidObjectId(ownerId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addBlock(ownerId, characterId);
}


void DeferrableProxyGameDatabase::asyncRemoveBlock(ObjectId ownerId, ObjectId characterId) 
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidObjectId(ownerId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_removeBlock(ownerId, characterId);
}


void DeferrableProxyGameDatabase::asyncQueryAchievements(AccountId accountId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidAccountId(accountId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_queryAchievements(accountId, characterId);
}


void DeferrableProxyGameDatabase::asyncUpdateProcessAchievement(AccountId accountId, ObjectId characterId, AchievementCode code, const ProcessAchievementInfo& missionInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidAccountId(accountId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_updateProcessAchievement(accountId, characterId, code, missionInfo);
}


void DeferrableProxyGameDatabase::asyncCompleteAchievement(AccountId accountId, ObjectId characterId, AchievementCode code, AchievementPoint point)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidAccountId(accountId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_completeAchievement(accountId, characterId, code, point);
}


void DeferrableProxyGameDatabase::asyncQueryCharacterTitles(AccountId accountId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidAccountId(accountId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_queryCharacterTitles(accountId, characterId);
}


void DeferrableProxyGameDatabase::asyncAddCharacterTitle(AccountId accountId, ObjectId characterId, CharacterTitleCode titleCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! (isValidAccountId(accountId) && isValidObjectId(characterId))) {
        assert(false);
        return;
    }

    if (! databaseProxy_->isActivated()) {
        SNE_LOG_ERROR(__FUNCTION__ " - disconnected");
        return;
    }

    rpcExtension_->s2d_addCharacterTitle(accountId, characterId, titleCode);
}

}} // namespace gideon { namespace serverbase {
