#pragma once

#include "../communityserver_export.h"
#include <gideon/server/rpc/ZoneCommunityRpc.h>
#include <gideon/cs/shared/data/ServerType.h>
#include <sne/server/s2s/ManagedServerSideProxy.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace communityserver {

/**
 * @class CommunityServerSideProxy
 *
 * CommunityServer-side proxy
 */
class CommunityServer_Export CommunityServerSideProxy :
    public sne::server::ManagedServerSideProxy,
    public rpc::ZoneCommunityRpc,
    public rpc::ZonePartyRpc,
    public rpc::ZoneGuildRpc,
    public rpc::ZoneGuildInventoryRpc,
	public rpc::ZoneDominionRpc,
	public rpc::ZoneTeleportRpc,
    public rpc::ZoneCheatRpc,
    public rpc::ZoneBuddyRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(CommunityServerSideProxy);

public:
    CommunityServerSideProxy(sne::server::ServerSideBridge* bridge);
    virtual ~CommunityServerSideProxy();

public:
    bool isZoneServerProxy() const {
        return isZoneServer(serverType_);
    }

    ZoneId getZoneId() const {
        return zoneId_;
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    // = rpc::ZoneCommunityRpc overriding
    OVERRIDE_SRPC_METHOD_1(z2m_rendezvous,
        ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_0(z2m_onRendezvous);

    OVERRIDE_SRPC_METHOD_0(z2m_ready);

    OVERRIDE_SRPC_METHOD_2(z2m_worldMapOpened,
        MapCode, worldMapCode, ObjectId, mapId);
    OVERRIDE_SRPC_METHOD_2(z2m_worldMapClosed,
        MapCode, worldMapCode, ObjectId, mapId);

    OVERRIDE_SRPC_METHOD_1(z2m_initPlayerInfo,
        PlayerInfo, playerInfo);
    OVERRIDE_SRPC_METHOD_3(z2m_changeCharacterClass,
        AccountId, accountId, ObjectId, characterId, CharacterClass, cc);
    OVERRIDE_SRPC_METHOD_3(z2m_updateCreatureLevel,
        AccountId, accountId, ObjectId, characterId, CreatureLevel, level);

    OVERRIDE_SRPC_METHOD_4(z2m_worldMapEntered,
        ObjectId, mapId, AccountId, accountId, ObjectId, playerId,
        Position, position);
    OVERRIDE_SRPC_METHOD_2(z2m_worldMapLeft,
        AccountId, accountId, ObjectId, playerId);

	OVERRIDE_SRPC_METHOD_2(z2m_deleteCharacter,
		ObjectId, characterId, GuildId, guildId);

public:
    // = rpc::ZonePartyRpc overriding
    OVERRIDE_SRPC_METHOD_1(z2m_createRandumDungeonPublicParty,
        GameObjectInfo, randumDungionInfo);
    OVERRIDE_SRPC_METHOD_2(z2m_onCreateRandumDungeonPublicParty,
        GameObjectInfo, randumDungionInfo, PartyId, partyId);
    OVERRIDE_SRPC_METHOD_1(z2m_destoryRandumDungeonPublicParty,
        PartyId, partyId);

    OVERRIDE_SRPC_METHOD_2(z2m_createWorldEventPublicParty,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode);
    OVERRIDE_SRPC_METHOD_3(z2m_onCreateWorldEventPublicParty,
        WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, PartyId, partyId);
    OVERRIDE_SRPC_METHOD_1(z2m_destoryWorldEventPublicParty,
        PartyId, partyId);

    OVERRIDE_SRPC_METHOD_2(z2m_notifyPartyInfo,
        PartyId, partyId, AccountId, accountId);

    OVERRIDE_SRPC_METHOD_2(z2m_evPartyMemberJoined,
        PartyId, partyId, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_3(z2m_evPartyMemberCharacterClassChanged,
        PartyId, partyId, ObjectId, playerId, CharacterClass, cc);
    OVERRIDE_SRPC_METHOD_2(z2m_evPartyMemberLeft,
        PartyId, partyId, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_3(z2m_evPartyMemberRejoined,
        PartyId, partyId, AccountId, accountId, ObjectId, playerId);

    OVERRIDE_SRPC_METHOD_2(z2m_evPartyTypeChanged,
        PartyId, partyId, PartyType, partyType);

public:
    // = rpc::ZoneGuildRpc overriding
    OVERRIDE_SRPC_METHOD_2(z2m_createGuild,
        GuildMemberInfo, guildMemberInfo, BaseGuildInfo, guildInfo);
    OVERRIDE_SRPC_METHOD_2(z2m_addGuildMember,
        GuildId, guildId, GuildMemberInfo, guildMemberInfo);
    OVERRIDE_SRPC_METHOD_2(z2m_removeGuildMember,
        GuildId, guildId, ObjectId, characterId);
	OVERRIDE_SRPC_METHOD_3(z2m_changeGuildMemberPosition,
		GuildId, guildId, ObjectId, characterId, GuildMemberPosition, position);
    OVERRIDE_SRPC_METHOD_2(z2m_addGuildRelationship,
        GuildId, guildId, GuildRelationshipInfo, info);
    OVERRIDE_SRPC_METHOD_2(z2m_removeGuildRelationship,
        GuildId, guildId, GuildId, targetGuildId);
    OVERRIDE_SRPC_METHOD_3(z2m_purchaseVault,
        AccountId, accountId, BaseVaultInfo, vaultInfo, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_3(z2m_activateGuildSkill,
        AccountId, accountId, ObjectId, playerId, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_2(z2m_deactivateGuildSkills,
        AccountId, accountId, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_3(z2m_addGuildExp,
        AccountId, accountId, ObjectId, playerId, GuildExp, exp);
    OVERRIDE_SRPC_METHOD_3(z2m_addCheatGuildExp,
        AccountId, accountId, ObjectId, playerId, GuildExp, exp);
    OVERRIDE_SRPC_METHOD_3(z2m_addGuildGameMoney,
        AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);

    OVERRIDE_SRPC_METHOD_3(z2m_onPurchaseVault,
        ErrorCode, errorCode, ObjectId, playerId, BaseVaultInfo, vaultInfo);
    OVERRIDE_SRPC_METHOD_4(z2m_onActivateGuildSkill,
        ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_3(z2m_onDeactivateGuildSkills,
        ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId);

    OVERRIDE_SRPC_METHOD_2(z2m_guildLevelUpdated,
        GuildId, guildId, GuildLevel, guildLevel);
    OVERRIDE_SRPC_METHOD_2(z2m_updateGuildExpAddibleState,
        GuildId, guildId, bool, guildExpAddible);

public:
    // = rpc::ZoneGuildInventoryRpc overriding
    OVERRIDE_SRPC_METHOD_5(z2m_pushGuildInventoryItem,
        AccountId, accountId, ObjectId, playerId, ItemInfo, itemInfo, VaultId, vaultId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_5(z2m_popGuildInventoryItem,
        AccountId, accountId, ObjectId, playerId, ObjectId, itemId, VaultId, vaultId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_5(z2m_switchGuildInventoryItem,
        AccountId, accountId, ObjectId, playerId, VaultId, vaultId, ObjectId, guildItemId, ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_3(z2m_depositGuildGameMoney,
        AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_3(z2m_withdrawGuildGameMoney,
        AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);

    OVERRIDE_SRPC_METHOD_3(z2m_onPushGuildInventoryItem,
        ErrorCode, errorCode, ObjectId, playerId, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_4(z2m_onPopGuildInventoryItem,
        ErrorCode, errorCode, ObjectId, playerId, ItemInfo, itemInfo, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_4(z2m_onSwitchGuildInventoryItem,
        ErrorCode, errorCode, ObjectId, playerId, ObjectId, playerItemId, ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_3(z2m_onDepositGuildGameMoney,
        ErrorCode, errorCode, ObjectId, playerId, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_5(z2m_onWithdrawGuildGameMoney,
        ErrorCode, errorCode, ObjectId, playerId, sec_t, resetDayWithdrawTime, GameMoney, todayWithdraw, GameMoney, gameMoney);
public:
    // = rpc::ZoneDominionRpc overriding
	OVERRIDE_SRPC_METHOD_2(z2m_addDominion,
		MapCode, worldMapCode, MoreBuildingDominionInfo, addInfo);
	OVERRIDE_SRPC_METHOD_2(z2m_removeDominion,
		MapCode, worldMapCode, ObjectId, buildingId);
	OVERRIDE_SRPC_METHOD_3(z2m_setActivateDominion,
		MapCode, worldMapCode, ObjectId, buildingId, bool, isActivate);

public:
    // = rpc::ZoneTeleportRpc overriding
	OVERRIDE_SRPC_METHOD_4(z2m_requestRecall,
		AccountId, callerId, Nickname, callerName, Nickname, callee, WorldPosition, worldPosition);

	OVERRIDE_SRPC_METHOD_3(z2m_evRecallRequested,
		Nickname, caller, Nickname, callee, WorldPosition, worldPosition);

	OVERRIDE_SRPC_METHOD_4(z2m_requestTeleportToBuilding,
		ObjectId, ownerId, MapCode, worldMapCode, GuildId, guildId, ObjectId, buildingId);

	OVERRIDE_SRPC_METHOD_5(z2m_onRequestTeleportToBuilding,
		ErrorCode, errorCode, ObjectId, ownerId, BuildingCode, toBuildingCode,
		ObjectId, fromBuildingId, WorldPosition, buildingPosition);

public:
    // = rpc::ZoneCheatRpc overriding
    OVERRIDE_SRPC_METHOD_1(z2m_getWorldUserInfos,
        ObjectId, playerId);

    OVERRIDE_SRPC_METHOD_2(z2m_onGetWorldUserInfos,
        ObjectId, playerId, WorldUserInfos, infos);

public:
    // = rpc::ZoneBuddyRpc overridng
    OVERRIDE_SRPC_METHOD_2(z2m_onBuddyAdded,
        ObjectId, playerId, uint32_t, buddyCount);

protected: // for Test
    // = sne::server::ManagedServerSideProxy overriding
    virtual void bridged();
    virtual void unbridged();

private:
    // = sne::server::ServerSideProxy overriding
    virtual bool verifyAuthentication(
        const sne::server::S2sCertificate& certificate) const;

private:
    ServerType serverType_;
    ZoneId zoneId_;
};

}} // namespace gideon { namespace communityserver {
