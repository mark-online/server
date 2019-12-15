#pragma once

#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/PlayerInfo.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/DominionInfo.h>
#include <gideon/cs/shared/data/CheatInfo.h>
#include <gideon/cs/shared/data/WorldEventInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/srpc/RpcInterface.h>

namespace gideon { namespace rpc {

/**
 * @class ZoneCommunityRpc
 * Zone Server <-> Community Server messages
 */
class ZoneCommunityRpc : boost::noncopyable
{
public:
    virtual ~ZoneCommunityRpc() {}

public:
    /**
     * 로그인 서버와 랑데뷰한다.
     */
    DECLARE_SRPC_METHOD_1(ZoneCommunityRpc, z2m_rendezvous,
        ZoneId, zoneId);

    /// 랑데뷰 요청에 대한 응답
    DECLARE_SRPC_METHOD_0(ZoneCommunityRpc, z2m_onRendezvous);

public:
    /// 로그인 서버에게 준비 완료를 알린다.
    DECLARE_SRPC_METHOD_0(ZoneCommunityRpc, z2m_ready);

public:
    /// 월드맵이 열렸다
    DECLARE_SRPC_METHOD_2(ZoneCommunityRpc, z2m_worldMapOpened,
        MapCode, worldMapCode, ObjectId, mapId);

    /// 월드맵이 닫혔다
    DECLARE_SRPC_METHOD_2(ZoneCommunityRpc, z2m_worldMapClosed,
        MapCode, worldMapCode, ObjectId, mapId);

public:
    /// 플레이어가 월드맵에 입장하였음을 알린다
    DECLARE_SRPC_METHOD_1(ZoneCommunityRpc, z2m_initPlayerInfo,
        PlayerInfo, playerInfo);

    DECLARE_SRPC_METHOD_3(ZoneCommunityRpc, z2m_changeCharacterClass,
        AccountId, accountId, ObjectId, characterId, CharacterClass, cc);

    /// 플레이가 레벨업 했다.
    DECLARE_SRPC_METHOD_3(ZoneCommunityRpc, z2m_updateCreatureLevel,
        AccountId, accountId, ObjectId, characterId, CreatureLevel, level);


    DECLARE_SRPC_METHOD_4(ZoneCommunityRpc, z2m_worldMapEntered,
        ObjectId, mapId, AccountId, accountId, ObjectId, playerId,
        Position, position);

    /// 플레이어가 월드맵에서 퇴장였음을 알린다
    DECLARE_SRPC_METHOD_2(ZoneCommunityRpc, z2m_worldMapLeft,
        AccountId, accountId, ObjectId, playerId);


public:
	DECLARE_SRPC_METHOD_2(ZoneCommunityRpc, z2m_deleteCharacter,
		ObjectId, characterId, GuildId, guildId);
};


/**
 * @class ZonePartyRpc
 * Zone Server <- Community Server party messages
 */
class ZonePartyRpc : boost::noncopyable
{
public:
    virtual ~ZonePartyRpc() {}

    // 공개 파티가 만들어졌다.
    DECLARE_SRPC_METHOD_1(ZonePartyRpc, z2m_createRandumDungeonPublicParty,
        GameObjectInfo, randumDungionInfo);
    
    DECLARE_SRPC_METHOD_2(ZonePartyRpc, z2m_onCreateRandumDungeonPublicParty,
        GameObjectInfo, randumDungionInfo, PartyId, partyId);

    DECLARE_SRPC_METHOD_1(ZonePartyRpc, z2m_destoryRandumDungeonPublicParty,
        PartyId, partyId);

    DECLARE_SRPC_METHOD_2(ZonePartyRpc, z2m_createWorldEventPublicParty,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode);

    DECLARE_SRPC_METHOD_3(ZonePartyRpc, z2m_onCreateWorldEventPublicParty,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, PartyId, partyId);

    DECLARE_SRPC_METHOD_1(ZonePartyRpc, z2m_destoryWorldEventPublicParty,
        PartyId, partyId);

    /// z2m_evPartyMemberJoined 메세지를 받은 후 다시 알린다
    DECLARE_SRPC_METHOD_2(ZonePartyRpc, z2m_notifyPartyInfo,
        PartyId, partyId, AccountId, accountId);

    // 파티원이 추가됐음을 알린다.
    DECLARE_SRPC_METHOD_2(ZonePartyRpc, z2m_evPartyMemberJoined,
        PartyId, partyId, ObjectId, playerId);

    DECLARE_SRPC_METHOD_3(ZonePartyRpc, z2m_evPartyMemberCharacterClassChanged,
        PartyId, partyId, ObjectId, playerId, CharacterClass, cc);

    // 파티원이 추방됐음을 알린다.
    DECLARE_SRPC_METHOD_2(ZonePartyRpc, z2m_evPartyMemberLeft,
        PartyId, partyId, ObjectId, playerId);

    // 파티원이 존서버 이동을 통해 이동했음을 알린다
    DECLARE_SRPC_METHOD_3(ZonePartyRpc, z2m_evPartyMemberRejoined,
        PartyId, partyId, AccountId, accountId, ObjectId, playerId);

    // 파티 타입이 변경되었다
    DECLARE_SRPC_METHOD_2(ZonePartyRpc, z2m_evPartyTypeChanged,
        PartyId, partyId, PartyType, partyType);
};


/**
 * @class ZoneGuildRpc
 * Zone Server -> Community Server party messages
 */
class ZoneGuildRpc : boost::noncopyable
{
public:
    DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_createGuild,
        GuildMemberInfo, guildMemberInfo, BaseGuildInfo, guildInfo);

    DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_addGuildMember,
        GuildId, guildId, GuildMemberInfo, guildMemberInfo);

    DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_removeGuildMember,
        GuildId, guildId, ObjectId, characterId);

	DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_changeGuildMemberPosition,
		GuildId, guildId, ObjectId, characterId, GuildMemberPosition, position);

	DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_addGuildRelationship,
		GuildId, guildId, GuildRelationshipInfo, info);

	DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_removeGuildRelationship,
		GuildId, guildId, GuildId, targetGuildId);

public:
    /// 금고 구매 요청
    /// - response: z2m_onPurchaseVault
    DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_purchaseVault,
        AccountId, accountId, BaseVaultInfo, vaultInfo, ObjectId, playerId);

    /// - response: z2m_onActivateGuildSkill
    DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_activateGuildSkill,
        AccountId, accountId, ObjectId, playerId, SkillCode, skillCode);
    
    DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_deactivateGuildSkills,
        AccountId, accountId, ObjectId, playerId);

    /// - response: none
    DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_addGuildExp,
        AccountId, accountId, ObjectId, playerId, GuildExp, exp);

    DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_addCheatGuildExp,
        AccountId, accountId, ObjectId, playerId, GuildExp, exp);

    /// - response: none
    DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_addGuildGameMoney,
        AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);

public:
    DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_onPurchaseVault,
        ErrorCode, errorCode, ObjectId, playerId, BaseVaultInfo, vaultInfo);

    DECLARE_SRPC_METHOD_4(ZoneGuildRpc, z2m_onActivateGuildSkill,
        ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId, SkillCode, skillCode);

    DECLARE_SRPC_METHOD_3(ZoneGuildRpc, z2m_onDeactivateGuildSkills,
        ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId);

    DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_guildLevelUpdated,
        GuildId, guildId, GuildLevel, guildLevel);

    DECLARE_SRPC_METHOD_2(ZoneGuildRpc, z2m_updateGuildExpAddibleState,
        GuildId, guildId, bool, guildExpAddible);
};


/**
 * @class ZoneDominionRpc
 * Zone Server -> Community Server Dominion messages
 */
class ZoneGuildInventoryRpc : boost::noncopyable
{
public:
    // response: z2m_onPushGuildInventoryItem
    DECLARE_SRPC_METHOD_5(ZoneGuildInventoryRpc, z2m_pushGuildInventoryItem,
        AccountId, accountId, ObjectId, playerId, ItemInfo, itemInfo, VaultId, vaultId, SlotId, slotId);
    DECLARE_SRPC_METHOD_5(ZoneGuildInventoryRpc, z2m_popGuildInventoryItem,
        AccountId, accountId, ObjectId, playerId, ObjectId, itemId, VaultId, vaultId, SlotId, slotId);
    DECLARE_SRPC_METHOD_5(ZoneGuildInventoryRpc, z2m_switchGuildInventoryItem,
        AccountId, accountId, ObjectId, playerId, VaultId, vaultId, ObjectId, guildItemId, ItemInfo, itemInfo);
    DECLARE_SRPC_METHOD_3(ZoneGuildInventoryRpc, z2m_depositGuildGameMoney,
        AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);
    DECLARE_SRPC_METHOD_3(ZoneGuildInventoryRpc, z2m_withdrawGuildGameMoney,
        AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);

    DECLARE_SRPC_METHOD_3(ZoneGuildInventoryRpc, z2m_onPushGuildInventoryItem,
        ErrorCode, errorCode, ObjectId, playerId, ObjectId, itemId);
    DECLARE_SRPC_METHOD_4(ZoneGuildInventoryRpc, z2m_onPopGuildInventoryItem,
        ErrorCode, errorCode, ObjectId, playerId, ItemInfo, itemInfo, SlotId, slotId);
    DECLARE_SRPC_METHOD_4(ZoneGuildInventoryRpc, z2m_onSwitchGuildInventoryItem,
        ErrorCode, errorCode, ObjectId, playerId, ObjectId, playerItemId, ItemInfo, itemInfo);
    DECLARE_SRPC_METHOD_3(ZoneGuildInventoryRpc, z2m_onDepositGuildGameMoney,
        ErrorCode, errorCode, ObjectId, playerId, GameMoney, gameMoney);
    DECLARE_SRPC_METHOD_5(ZoneGuildInventoryRpc, z2m_onWithdrawGuildGameMoney,
        ErrorCode, errorCode, ObjectId, playerId, sec_t, resetDayWithdrawTime,
        GameMoney, todayWithdraw, GameMoney, gameMoney);
};


/**
 * @class ZoneDominionRpc
 * Zone Server -> Community Server Dominion messages
 */
class ZoneDominionRpc : boost::noncopyable
{
public:
	DECLARE_SRPC_METHOD_2(ZoneDominionRpc, z2m_addDominion,
		MapCode, worldMapCode, MoreBuildingDominionInfo, addInfo);

	DECLARE_SRPC_METHOD_2(ZoneDominionRpc, z2m_removeDominion,
		MapCode, worldMapCode, ObjectId, buildingId);

	DECLARE_SRPC_METHOD_3(ZoneDominionRpc, z2m_setActivateDominion,
		MapCode, worldMapCode, ObjectId, buildingId, bool, isActivate);
};


/**
 * @class ZoneTeleportRpc
 * Zone Server -> Community Server teleport messages
 */
class ZoneTeleportRpc : boost::noncopyable
{
public:
	DECLARE_SRPC_METHOD_4(ZoneTeleportRpc, z2m_requestRecall,
		AccountId, callerId, Nickname, callerName, Nickname, callee, WorldPosition, worldPosition);

	DECLARE_SRPC_METHOD_3(ZoneTeleportRpc, z2m_evRecallRequested,
		Nickname, caller, Nickname, callee, WorldPosition, worldPosition);

	DECLARE_SRPC_METHOD_4(ZoneTeleportRpc, z2m_requestTeleportToBuilding,
		ObjectId, ownerId, MapCode, worldMapCode, GuildId, guildId, ObjectId, buildingId);

	DECLARE_SRPC_METHOD_5(ZoneTeleportRpc, z2m_onRequestTeleportToBuilding,
		ErrorCode, errorCode, ObjectId, ownerId, BuildingCode, toBuildingCode, 
		ObjectId, fromBuildingId, WorldPosition, buildingPosition);
};


/**
 * @class ZoneCheatRpc
 * Zone Server -> Community Server teleport messages
 */
class ZoneCheatRpc : boost::noncopyable
{
public:
    DECLARE_SRPC_METHOD_1(ZoneCheatRpc, z2m_getWorldUserInfos,
        ObjectId, playerId);

    DECLARE_SRPC_METHOD_2(ZoneCheatRpc, z2m_onGetWorldUserInfos,
        ObjectId, playerId, WorldUserInfos, infos);
};

/**
 * @class ZoneBuddyRpc
 * Zone Server -> Community Server buddy messages
 */
class ZoneBuddyRpc : boost::noncopyable
{
public:
    DECLARE_SRPC_METHOD_2(ZoneBuddyRpc, z2m_onBuddyAdded,
        ObjectId, playerId, uint32_t, buddyCount);
};



}} // namespace gideon { namespace rpc {
