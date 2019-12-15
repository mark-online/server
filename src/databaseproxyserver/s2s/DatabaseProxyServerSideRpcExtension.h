#pragma once

#include <gideon/server/rpc/DatabaseProxyRpc.h>
#include <sne/base/session/extension/SessionExtension.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace databaseproxyserver {

using sne::database::proxy::RequestId;

/**
 * @class DatabaseProxyServerSideRpcExtension
 *
 * SessionExtension for Database proxy RPC
 */
class DatabaseProxyServerSideRpcExtension :
    public sne::base::AbstractSessionExtension<DatabaseProxyServerSideRpcExtension>,
    public rpc::DatabaseProxyRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(DatabaseProxyServerSideRpcExtension);

    SNE_GENERATE_SESSION_EXTENSION_ID(DatabaseProxyServerSideRpcExtension);

public:
    virtual void attachedTo(sne::base::SessionImpl& sessionImpl) override;

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

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
        RequestId, requestId, ErrorCode, errorCode,
        FullCharacterInfo, characterInfo);

    OVERRIDE_SRPC_METHOD_3(s2d_deleteCharacter,
        RequestId, requestId, AccountId, accountId, ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_4(s2d_onDeleteCharacter,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId, GuildId, guildId);

    OVERRIDE_SRPC_METHOD_3(s2d_checkDuplicateNickname,
        RequestId, requestId, AccountId, accountId, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_4(s2d_onCheckDuplicateNickname,
        RequestId, requestId, ErrorCode, errorCode, AccountId, accountId, Nickname, nickname);

    OVERRIDE_SRPC_METHOD_3(s2d_saveCharacterStats, 
        AccountId, accountId, ObjectId, characterId, DBCharacterStats, saveInfo);
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

    OVERRIDE_SRPC_METHOD_2(s2d_getMaxInventoryId,
        RequestId, requestId, ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_3(s2d_onGetMaxInventoryId,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, maxInventoryId);

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
};

}} // namespace gideon { namespace databaseproxyserver {
