#pragma once

#include <gideon/server/data/CharacterStats.h>
#include <gideon/server/data/CreateCharacterInfo.h>
#include <gideon/server/data/ShardInfo.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/cs/shared/data/UnitInfo.h>
#include <gideon/cs/shared/data/MailInfo.h>
#include <gideon/cs/shared/data/AuctionInfo.h>
#include <gideon/cs/shared/data/BuddyInfos.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/AchievementInfo.h>
#include <gideon/cs/shared/data/CharacterTitleInfo.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/server/common/Property.h>
#include <sne/database/proxy/DatabaseRequest.h>
#include <sne/srpc/RpcInterface.h>

namespace gideon { namespace rpc {

using sne::database::proxy::RequestId;

/**
 * @class DatabaseProxyRpc
 * Server <-> Database Proxy Server messages
 */
class DatabaseProxyRpc : public boost::noncopyable
{
public:
    /**
     * @struct CreateCharacterParameters
     */
    struct CreateCharacterParameters
    {
        Nickname nickname_;
        CharacterClass characterClass_;
        SexType sexType_;
        CreateCharacterEquipments createCharacterEquipments_;
		CharacterAppearance appearance_; 
        ZoneId zoneId_;
        ObjectPosition position_;
		SkillCode subSkillCode_;

        CreateCharacterParameters() :
            characterClass_(ccUnknown),
            sexType_(stMale),
            zoneId_(invalidZoneId) {}

        CreateCharacterParameters(const Nickname& nickname,
            CharacterClass characterClass, SexType sexType,
            const CreateCharacterEquipments& createCharacterEquipments,
            const CharacterAppearance& appearance, SkillCode subSkillCode, 
			ZoneId zoneId, const ObjectPosition& position) :
            nickname_(nickname),
            characterClass_(characterClass),
            sexType_(sexType),
            createCharacterEquipments_(createCharacterEquipments),
			appearance_(appearance), 
			subSkillCode_(subSkillCode),
            zoneId_(zoneId),
            position_(position) {}

        template <typename Stream>
        void serialize(Stream& stream) {
            stream & nickname_ & characterClass_ & sexType_ &
                createCharacterEquipments_ & appearance_ & subSkillCode_ & zoneId_ & position_;
        }
    };

public:
    virtual ~DatabaseProxyRpc() {}

public:
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_getProperties,
        RequestId, requestId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetProperties,
        RequestId, requestId, ErrorCode, errorCode,
        sne::server::Properties, properties);

    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_getServerSpec,
        RequestId, requestId, std::string, serverName, std::string, suffix);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetServerSpec,
        RequestId, requestId, ErrorCode, errorCode, sne::server::ServerSpec, spec);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getMaxInventoryId,
        RequestId, requestId, ZoneId, zoneId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetMaxInventoryId,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, inventoryMaxId);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getShardInfo,
        RequestId, requestId, ShardId, shardId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetShardInfo,
        RequestId, requestId, ErrorCode, errorCode, FullShardInfo, shardInfo);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getZoneInfo,
        RequestId, requestId, std::string, name);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetZoneInfo,
        RequestId, requestId, ErrorCode, errorCode, ZoneInfo, zoneInfo);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getZoneInfoList,
        RequestId, requestId, ShardId, shardId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetZoneInfoList,
        RequestId, requestId, ErrorCode, errorCode, ZoneInfos, zoneInfos);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getWorldTime,
        RequestId, requestId, ZoneId, zoneId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetWorldTime,
        RequestId, requestId, ErrorCode, errorCode, WorldTime, worldTime);

	DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getFullUserInfo,
        RequestId, requestId, AccountId, accountId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetFullUserInfo,
        RequestId, requestId, ErrorCode, errorCode, FullUserInfo, userInfo);


    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_createCharacter,
        RequestId, requestId, CreateCharacterInfo, createCharacterInfo, CreateCharacterEquipments, createCharacterEquipments, 
        ZoneId, zoneId, ObjectPosition, position);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onCreateCharacter,
        RequestId, requestId, ErrorCode, errorCode, FullCharacterInfo, characterInfo);

    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_deleteCharacter,
        RequestId, requestId, AccountId, accountId, ObjectId, characterId);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_onDeleteCharacter,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId, GuildId, guildId);

    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_checkDuplicateNickname,
        RequestId, requestId, AccountId, accountId, Nickname, nickname);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_onCheckDuplicateNickname,
        RequestId, requestId, ErrorCode, errorCode, AccountId, accountId, Nickname, nickname);


    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_saveCharacterStats, 
        AccountId, accountId, ObjectId, characterId,
        DBCharacterStats, saveInfo);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_saveSelectCharacterTitle, 
        AccountId, accountId, ObjectId, characterId,
        CharacterTitleCode, titleCode);

    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_loadCharacterProperties, 
        RequestId, requestId, AccountId, accountId, ObjectId, characterId);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_onLoadCharacterProperties, 
        RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId,
        std::string, config, std::string, prefs);

    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_saveCharacterProperties, 
        AccountId, accountId, ObjectId, characterId,
        std::string, config, std::string, prefs);

    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_moveInventoryItem,
        AccountId, accountId, ObjectId, characterId,
        InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_switchInventoryItem,
        AccountId, accountId, ObjectId, characterId,
        InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_addInventoryItem,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType, ItemInfo, itemInfo);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_changeEquipItemInfo,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_enchantEquipItem,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
        SocketSlotId, id, EquipSocketInfo, socketInfo);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_unenchantEquipItem,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId, SocketSlotId, id);


    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_removeInventoryItem,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType, ObjectId, objectId);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_updateInventoryItemCount,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType, 
		ObjectId, objectId, uint8_t, itemCount);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addQuestItem,
        AccountId, accountId, ObjectId, characterId, QuestItemInfo, questItemInfo);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_removeQuestItem,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateQuestItemUsableCount,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
        uint8_t, ueableCount);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateQuestItemCount,
        AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
        uint8_t, stackCount);

    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_equipItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, EquipPart, equipPart);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_unequipItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, SlotId, slotId, EquipPart, unequipPart);   
    DECLARE_SRPC_METHOD_6(DatabaseProxyRpc, s2d_replaceInventoryWithEquipItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, unequipItemId, EquipPart, unequipPart, ObjectId, equipItemId,
        EquipPart, equipPart);

    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_equipAccessoryItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, AccessoryIndex, equipPart);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_unequipAccessoryItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPart);   
    DECLARE_SRPC_METHOD_6(DatabaseProxyRpc, s2d_replaceInventoryWithAccessoryItem,
        AccountId, accountId, ObjectId, characterId,
        ObjectId, unequipItemId, AccessoryIndex, unequipPart, ObjectId, equipItemId,
        AccessoryIndex, equipPart);

    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_changeCharacterState,
        AccountId, accountId, ObjectId, characterId, CreatureStateType, state);

    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_saveActionBar,
        AccountId, accountId, ObjectId, characterId, ActionBarIndex, abiIndex, 
        ActionBarPosition, abpIndex, DataCode, code);

    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_learnSkill,
        AccountId, accountId, ObjectId, characterId, SkillCode, currentSkillCode, 
        SkillCode, learnSkillCode);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_removeSkill,
        AccountId, accountId, ObjectId, characterId, SkillCode, skillCode);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeAllSkill,
        AccountId, accountId, ObjectId, characterId);



	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_acceptQuest,
		AccountId, accountId, ObjectId, characterId, QuestCode, questCode);
	DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_acceptRepeatQuest,
		AccountId, accountId, ObjectId, characterId, QuestCode, questCode, sec_t, acceptRepeatTime);
	DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_cancelQuest,
		AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest);
	DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_completeQuest,
		AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest);
	DECLARE_SRPC_METHOD_6(DatabaseProxyRpc, s2d_updateQuestMission,
		AccountId, accountId, ObjectId, characterId, QuestCode, questCode,
		QuestMissionCode, missionCode, QuestGoalInfo, goalInfo, bool, isRepeatQuest);
	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_removeCompleteQuest,
		AccountId, accountId, ObjectId, characterId, QuestCode, questCode);


    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_getGuildInfos,
        RequestId, requestId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetGuildInfos,
        RequestId, requestId, ErrorCode, errorCode, GuildInfos, guildInfos);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_getMaxGuildId,
        RequestId, requestId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetMaxGuildId,
        RequestId, requestId, ErrorCode, errorCode, GuildId, guildId);

    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_createGuild,
        BaseGuildInfo, guildInfo);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addGuildMember,
        AccountId, accountId, GuildId, guildId, GuildMemberInfo, memberInfo);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_removeGuildMember,
        ObjectId, characterId);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_removeGuild,
        GuildId, guildId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addGuildRelationship,
        GuildId, ownerGuildId, GuildId, targetGuildId, GuildRelatioshipType, type);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeGuildRelationship,
        GuildId, ownerGuildId, GuildId, targetGuildId);
	DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_changeGuildMemberPosition,
		ObjectId, characterId, GuildMemberPosition, position);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_addGuildApplicant,
        ObjectId, characterId, GuildId, guildId);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_removeGuildApplicant,
        ObjectId, characterId);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_modifyGuildIntroduction,
        GuildId, guildId, GuildIntroduction, introduction);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_modifyGuildNotice,
        GuildId, guildId, GuildNotice, notice);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addGuildRank,
        GuildId, guildId, GuildRankInfo, rankInfo, uint8_t, vaultCount);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_addGuilBankRank,
        GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, GuildBankVaultRightInfo, bankRankInfo);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_deleteGuildRank,
        GuildId, guildId, GuildRankId, rankId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_swapGuildRank,
        GuildId, guildId, GuildRankId, rankId1, GuildRankId, rankId2);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_updateGuildRankName,
        GuildId, guildId, GuildRankId, rankId, GuildRankName, rankName);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateGuildRankRights,
        GuildId, guildId, GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_updateGuildBankRights,
        GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_updateGuildRank,
        GuildId, guildId, ObjectId, playerId, GuildRankId, rankId);

    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_removeAllGuildSkills,
        GuildId, guildId);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeGuildSkills,
        GuildId, guildId, SkillCodes, skillCodes);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_addGuildSkill,
        GuildId, guildId, SkillCode, skillCode);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_updateGuildExp,
        GuildId, guildId, GuildLevelInfo, guildLevelInfo);

    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_moveGuildInventoryItem,
        GuildId, guildId, VaultId, VaultId, ObjectId, itemId, SlotId, slotId);
    DECLARE_SRPC_METHOD_6(DatabaseProxyRpc, s2d_switchGuildInventoryItem,
        GuildId, guildId, VaultId, vaultId, ObjectId, itemId1, SlotId, slotId1, ObjectId, itemId2, SlotId, slotId2);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addGuildInventoryItem,
        GuildId, guildId, VaultId, vaultId, ItemInfo, itemInfo);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeGuildInventoryItem,
        GuildId, guildId, ObjectId, itemId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_updateGuildInventoryItemCount,
        GuildId, guildId, ObjectId, itemId, uint8_t, count);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_updateGuildGameMoney,
        GuildId, guildId, GameMoney, gameMoney);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_updateGuildMemberWithdraw,
        ObjectId, playerId, GameMoney, dayWithdraw, sec_t, resetTime);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_updateGuildVaultName,
        GuildId, guildId, VaultId, vaultId, VaultName, name);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_addGuildVault,
        GuildId, guildId, VaultInfo, vaultInfo);

    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_syncMail,
        ObjectId, characterId);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_sendMailByNickname,
        Nickname, reveiverNickname, MailInfo, maillInfo);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_sendMailByPlayerId,
        ObjectId, playerId, MailInfo, maillInfo);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_aquireItemsInMail,
        MailId, mailId);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_deleteMail,
        MailId, mailId);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_readMail,
        MailId, mailId);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_queryNotReadMail,
        ObjectId, characterId);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_onNotReadMail,
        ObjectId, characterId, bool, hasNotReadMail);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_onSyncMail,
        ObjectId, characterId, MailInfos, mailInfos);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_evMailReceived,
        ObjectId, receiverId, MailInfo, maillInfo);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getAuctionInfos,
        RequestId, requestId, ZoneId, zoneId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onAuctionInfos,
        RequestId, requestId, ErrorCode, errorCode, ServerAuctionInfoMap, serverAuctionInfoMap);

    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_createAuction,
        ZoneId, zoneId, NpcCode, npcCode, FullAuctionInfo, auctionInfo);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_deleteAuction,
        ZoneId, zoneId, AuctionId, auctionId);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateBid,
        ZoneId, zoneId, AuctionId, auctionId, ObjectId, buyerId, GameMoney, currentBidMoney);

    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_updateArenaRecord,
        AccountId, accountId, ObjectId, characterId, ArenaModeType, arenaMode,
        uint32_t, resultScore, ArenaResultType, resultType);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_updateArenaPoint,
        AccountId, accountId, ObjectId, characterId, ArenaPoint, arenaPoint);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_updateDeserterExpireTime,
        ObjectId, characterId, sec_t, deserterExpireTime);


    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_moveBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    DECLARE_SRPC_METHOD_6(DatabaseProxyRpc, s2d_switchBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2,
        SlotId, slot1, SlotId, slot2);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ItemInfo, itemInfo);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_removeBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType, ObjectId, objectId);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateBuildingInventoryItemCount,
        ObjectId, buildingId, InvenType, invenType, ObjectId, objectId, uint8_t, itemCount);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeAllBuildingInventoryItem,
        ObjectId, buildingId, InvenType, invenType);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_addBuildingGuard,
        ObjectId, buildingId, BuildingGuardInfo, info);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_removeBuildingGuard,
        ObjectId, guardId);

    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_createBuilding,
        BuildingInfo, buildingInfo);
    DECLARE_SRPC_METHOD_1(DatabaseProxyRpc, s2d_deleteBuilding,
        ObjectId, buildingId);
    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_updateBuildingState,
        ObjectId, buildingId, BuildingStateType, state, sec_t, startBuildTime,
        sec_t, expireTime, HitPoint, currentHitPoint);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateBuildingOwner,
        ObjectId, buildingId, BuildingOwnerType, ownerType, ObjectId, playerId, GuildId, guildId);
    
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addBindRecallInfo,
        AccountId, accountId, ObjectId, characterId, BindRecallInfo, bindRecallInfo);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_removeBindRecallIInfo,
        AccountId, accountId, ObjectId, characterId, ObjectId, linkId);

    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_addSelectRecipeProduction,
        ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime);
    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeSelectRecipeProduction,
        ObjectId, buildingId, RecipeCode, recipeCode);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateSelectRecipeProduction,
        ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_releaseBeginnerProtection,
        AccountId, accountId, ObjectId, characterId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc,s2d_addCooldownInfos,
        AccountId, accountId, ObjectId, characterId, CooltimeInfos, infos);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc,s2d_addRemainEffects,
        AccountId, accountId, ObjectId, characterId, RemainEffectInfos, infos);

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_loadBuildingInfo,
        RequestId, requestId, ZoneId, zoneId);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_onLoadBuildingInfo,
        RequestId, requestId, ErrorCode, errorCode, ObjectId, maxBuildingId,
        BuildingInfoMap, buildingInfoMap);

    DECLARE_SRPC_METHOD_5(DatabaseProxyRpc, s2d_updateCharacterInventoryInfo,
        AccountId, accountId, ObjectId, characterId, InvenType, invenType,
        bool, isCashSlot, uint8_t, count);

	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_createVehicle,
		AccountId, accountId, ObjectId, characterId, VehicleInfo, info);
	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_createGlider,
		AccountId, accountId, ObjectId, characterId, GliderInfo, info);
	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_deleteVehicle,
		AccountId, accountId, ObjectId, characterId, ObjectId, id);
	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_deleteGlider,
		AccountId, accountId, ObjectId, characterId, ObjectId, id);
	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_selectVehicle,
		AccountId, accountId, ObjectId, characterId, ObjectId, id);
	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_selectGlider,
		AccountId, accountId, ObjectId, characterId, ObjectId, id);
	DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateGliderDurability,
		AccountId, accountId, ObjectId, characterId, ObjectId, id, uint32_t, currentValue);


	DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_getBuddies,
		AccountId, accountId, ObjectId, characterId);
	DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_addBuddy,
		ObjectId, characterId1, ObjectId, characterId2);
	DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeBuddy,
		ObjectId, characterId1, ObjectId, characterId2);
	DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_addBlock,
		ObjectId, ownerId, ObjectId, characterId);
	DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_removeBlock,
		ObjectId, ownerId, ObjectId, characterId);

	DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_onGetBuddies,
		AccountId, accountId, BuddyInfos, buddyInfos, BlockInfos, blockInfos);
        

    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_queryAchievements,
        AccountId, accountId, ObjectId, characterId);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_onQueryAchievements,
        ErrorCode, errorCode, ObjectId, characterId, ProcessAchievementInfoMap, processInfoMap,
        CompleteAchievementInfoMap, completeInfoMap);


    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_updateProcessAchievement,
        AccountId, accountId, ObjectId, characterId, AchievementCode, code, ProcessAchievementInfo, missionInfo);
    DECLARE_SRPC_METHOD_4(DatabaseProxyRpc, s2d_completeAchievement,
        AccountId, accountId, ObjectId, characterId, AchievementCode, code, AchievementPoint, point);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc, s2d_addCharacterTitle,
        AccountId, accountId, ObjectId, characterId, CharacterTitleCode, titleCode);


    DECLARE_SRPC_METHOD_2(DatabaseProxyRpc, s2d_queryCharacterTitles,
        AccountId, accountId, ObjectId, characterId);
    DECLARE_SRPC_METHOD_3(DatabaseProxyRpc,s2d_onQueryCharacterTitles,
        ErrorCode, errorCode, ObjectId, characterId, CharacterTitleCodeSet, titleCodeSet);

};

}} // namespace gideon { namespace rpc {
