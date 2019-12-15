#include "CommunityServerPCH.h"
#include "CommunityServerSideProxy.h"
#include "../CommunityService.h"
#include "../guild/CommunityGuildManager.h"
#include "../guild/CommunityGuild.h"
#include "../guild/inventory/GuildInventory.h"
#include "../channel/WorldMapChannelManager.h"
#include "../user/CommunityUserManager.h"
#include "../user/CommunityUser.h"
#include "../dominion/CommunityDominionManager.h"
#include "../Party/CommunityPartyManager.h"
#include "../Party/CommunityParty.h"
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace communityserver {

namespace {


inline ErrorCode queryGuild(CommunityGuildPtr& guildPtr, AccountId accountId)
{
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
    if (! user) {        
        return ecServerInternalError;
    }

    guildPtr = COMMUNITYGUILD_MANAGER->getGuild(user->getGuildId());
    if (guildPtr.get() == nullptr) {
        return ecGuildNotFindGuild;
    }

    return ecOk;
}

} // namespace

IMPLEMENT_SRPC_EVENT_DISPATCHER(CommunityServerSideProxy);

CommunityServerSideProxy::CommunityServerSideProxy(
    sne::server::ServerSideBridge* bridge) :
    sne::server::ManagedServerSideProxy(bridge),
    serverType_(stUnknown),
    zoneId_(invalidZoneId)
{
    if (hasSession()) {
        assert(getSession().hasImpl());
        sne::sgp::RpcingExtension* extension =
            getSession().getImpl().getExtension<sne::sgp::RpcingExtension>();
        extension->registerRpcForwarder(*this);
        extension->registerRpcReceiver(*this);
    }
}


CommunityServerSideProxy::~CommunityServerSideProxy()
{
}

// = sne::server::ManagedServerSideProxy overriding

void CommunityServerSideProxy::bridged()
{
    sne::server::ManagedServerSideProxy::bridged();

    SNE_LOG_WARNING("CommunityServerSideProxy::bridged(S%u).",
        getServerId());
}


void CommunityServerSideProxy::unbridged()
{
    SNE_LOG_WARNING("CommunityServerSideProxy::unbridged(S%u,%d).",
        getServerId(), serverType_);

    if (! sne::server::isValid(getServerId())) {
        return;
    }

    if (isZoneServerProxy()) {
        WORLDMAP_CHANNEL_MANAGER->zoneServerDisconnected(zoneId_);

        SNE_LOG_INFO("ZoneServer(S%u) connected", getServerId());
    }
    else {
        assert(false);
    }

    sne::server::ManagedServerSideProxy::unbridged();

    serverType_ = stUnknown;
    zoneId_ = invalidZoneId;
}

// = sne::server::ServerSideProxy overriding

bool CommunityServerSideProxy::verifyAuthentication(
    const sne::server::S2sCertificate& certificate) const
{
    return isSucceeded(COMMUNITY_SERVICE->authenticate(certificate));
}

// = sne::srpc::RpcForwarder overriding

void CommunityServerSideProxy::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityServerSideProxy::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void CommunityServerSideProxy::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityServerSideProxy::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::ZoneCommunityRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityServerSideProxy, z2m_rendezvous,
    ZoneId, zoneId)
{
    serverType_ = stZoneServer;
    zoneId_ = zoneId;

    z2m_onRendezvous();

    SNE_LOG_INFO("ZoneServer(S%u) connected", getServerId());
}


FORWARD_SRPC_METHOD_0(CommunityServerSideProxy, z2m_onRendezvous);


RECEIVE_SRPC_METHOD_0(CommunityServerSideProxy, z2m_ready)
{
    //COMMUNITY_SERVICE->zoneServerConnected(getServerId());

    SNE_LOG_WARNING("ZoneServer(S%u) readied.",
        getServerId());
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_worldMapOpened,
    MapCode, worldMapCode, ObjectId, mapId)
{
    assert(isValidZoneId(zoneId_));

    WORLDMAP_CHANNEL_MANAGER->worldMapOpened(zoneId_, worldMapCode, mapId);
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_worldMapClosed,
    MapCode, worldMapCode, ObjectId, mapId)
{
    assert(isValidZoneId(zoneId_));

    WORLDMAP_CHANNEL_MANAGER->worldMapClosed(zoneId_, worldMapCode, mapId);
}


RECEIVE_SRPC_METHOD_1(CommunityServerSideProxy, z2m_initPlayerInfo,
    PlayerInfo, playerInfo)
{
    COMMUNITYUSER_MANAGER->addNickname(playerInfo.nickname_, playerInfo.accountId_);
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(playerInfo.accountId_);
    if (user) {
        user->initPlayerInfo(getServerId(), playerInfo);
    }
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_changeCharacterClass,
    AccountId, accountId, ObjectId, characterId, CharacterClass, cc)
{
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
    if (user) {
        user->characterClassChanged(characterId, cc);
    }
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_updateCreatureLevel,
    AccountId, accountId, ObjectId, characterId, CreatureLevel, level)
{
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
    if (user) {
        user->updateCharacterLevel(characterId, level);
    }
}


RECEIVE_SRPC_METHOD_4(CommunityServerSideProxy, z2m_worldMapEntered,
    ObjectId, mapId, AccountId, accountId, ObjectId, playerId, Position, position)
{
    WORLDMAP_CHANNEL_MANAGER->worldMapEntered(getServerId(), zoneId_, mapId,
        accountId, playerId, position);
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_worldMapLeft,
    AccountId, accountId, ObjectId, playerId)
{
    // 존서버 이동간에 존서버 파티원 삭제 처리는 존서버에서 한다.
    WORLDMAP_CHANNEL_MANAGER->worldMapLeft(accountId, playerId);
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_deleteCharacter,
    ObjectId, characterId, GuildId, guildId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(guildId);
    if (guild.get() != nullptr) {
        if (isSucceeded(guild->dropout(characterId))) {
            if (guild->shouldDestory()) {
                COMMUNITYGUILD_MANAGER->removeGuild(guildId);
            }
        }
    }
}

// = rpc::ZonePartyRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityServerSideProxy, z2m_createRandumDungeonPublicParty,
    GameObjectInfo, randumDungionInfo)
{
    PartyId partyId = invalidPartyId;
    COMMUNITYPARTY_MANAGER->createParty(partyId, ptPublic, nullptr);
    z2m_onCreateRandumDungeonPublicParty(randumDungionInfo, partyId);
}


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_onCreateRandumDungeonPublicParty,
    GameObjectInfo, randumDungionInfo, PartyId, partyId);


RECEIVE_SRPC_METHOD_1(CommunityServerSideProxy, z2m_destoryRandumDungeonPublicParty,
    PartyId, partyId)
{
    CommunityPartyManager::CommunityPartyPtr ptr = COMMUNITYPARTY_MANAGER->getParty(partyId);
    if (ptr.get()) {
        ptr->closePublicEvent();
        if (ptr->isEmptyMember() && ptr->shouldDestroy()) {
            COMMUNITYPARTY_MANAGER->destroyParty(partyId);
        }
    }
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_createWorldEventPublicParty,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode)
{
    PartyId partyId = invalidPartyId;
    COMMUNITYPARTY_MANAGER->createParty(partyId, ptPublic, nullptr);
    z2m_onCreateWorldEventPublicParty(worldEventCode, missionCode, partyId);
}


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_onCreateWorldEventPublicParty,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, PartyId, partyId);


RECEIVE_SRPC_METHOD_1(CommunityServerSideProxy, z2m_destoryWorldEventPublicParty,
    PartyId, partyId)
{
    CommunityPartyManager::CommunityPartyPtr ptr = COMMUNITYPARTY_MANAGER->getParty(partyId);
    if (ptr.get()) {
        ptr->closePublicEvent();
        if (ptr->isEmptyMember() && ptr->shouldDestroy()) {
            COMMUNITYPARTY_MANAGER->destroyParty(partyId);
        }
    }
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_notifyPartyInfo,
    PartyId, partyId, AccountId, accountId)
{
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
    if (user) {
        user->notifyPartyInfo(partyId);
    }
}


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_evPartyMemberJoined,
    PartyId, partyId, ObjectId, playerId);


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_evPartyMemberCharacterClassChanged,
    PartyId, partyId, ObjectId, playerId, CharacterClass, cc);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_evPartyMemberLeft,
    PartyId, partyId, ObjectId, playerId);


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_evPartyMemberRejoined,
    PartyId, partyId, AccountId, accountId, ObjectId, playerId);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_evPartyTypeChanged,
    PartyId, partyId, PartyType, partyType);


// = rpc::ZoneGuildRpc overriding

FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_createGuild,
    GuildMemberInfo, guildMemberInfo, BaseGuildInfo, guildInfo);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_addGuildMember,
    GuildId, guildId, GuildMemberInfo, guildMemberInfo);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_removeGuildMember,
    GuildId, guildId, ObjectId, characterId);


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_changeGuildMemberPosition,
    GuildId, guildId, ObjectId, characterId, GuildMemberPosition, position);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_addGuildRelationship,
    GuildId, guildId, GuildRelationshipInfo, info);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_removeGuildRelationship,
    GuildId, guildId, GuildId, targetGuildId);


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_purchaseVault,
    AccountId, accountId, BaseVaultInfo, vaultInfo, ObjectId, playerId)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isSucceeded(errorCode)) {
        errorCode = guild->purchaseVault(vaultInfo, playerId);
    }

    z2m_onPurchaseVault(errorCode, playerId, vaultInfo);
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_activateGuildSkill,
    AccountId, accountId, ObjectId, playerId, SkillCode, skillCode)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isSucceeded(errorCode)) {
        errorCode = guild->activateGuildSkill(skillCode, playerId);
    }

    if (isFailed(errorCode)) {
        z2m_onActivateGuildSkill(errorCode, guild.get() != nullptr ? guild->getGuildId() : invalidGuildId,
            playerId, skillCode);
    }
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_deactivateGuildSkills,
    AccountId, accountId, ObjectId, playerId)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isSucceeded(errorCode)) {
        errorCode = guild->deactivateGuildSkills(playerId);
    }

    if (isFailed(errorCode)) {
        z2m_onDeactivateGuildSkills(errorCode, guild.get() != nullptr ? guild->getGuildId() : invalidGuildId,
            playerId);
    }
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_addGuildExp,
    AccountId, accountId, ObjectId, playerId, GuildExp, exp)
{
    playerId;
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isSucceeded(errorCode)) {
        guild->addExp(exp);
    }
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_addCheatGuildExp,
    AccountId, accountId, ObjectId, playerId, GuildExp, exp)
{
    playerId;
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isSucceeded(errorCode)) {
        guild->addCheatExp(exp);
    }
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_addGuildGameMoney,
    AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney)
{
    playerId;
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isSucceeded(errorCode)) {
        guild->upGuildGameMoney(gameMoney);
    }
}


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_onPurchaseVault,
    ErrorCode, errorCode, ObjectId, playerId, BaseVaultInfo, vaultInfo);


FORWARD_SRPC_METHOD_4(CommunityServerSideProxy, z2m_onActivateGuildSkill,
    ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId, SkillCode, skillCode);


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_onDeactivateGuildSkills,
    ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_guildLevelUpdated,
    GuildId, guildId, GuildLevel, guildLevel);


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_updateGuildExpAddibleState,
    GuildId, guildId, bool, guildExpAddible);

// = rpc::ZoneGuildInventoryRpc overriding

RECEIVE_SRPC_METHOD_5(CommunityServerSideProxy, z2m_pushGuildInventoryItem,
    AccountId, accountId, ObjectId, playerId, ItemInfo, itemInfo, VaultId, vaultId, SlotId, slotId)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isFailed(errorCode)) {
        z2m_onPushGuildInventoryItem(errorCode, playerId, invalidObjectId);
        return;
    }
        
    errorCode = guild->pushGuildInventoryItem(playerId, itemInfo, vaultId, slotId);
    z2m_onPushGuildInventoryItem(errorCode, playerId, itemInfo.itemId_);
}



RECEIVE_SRPC_METHOD_5(CommunityServerSideProxy, z2m_popGuildInventoryItem,
    AccountId, accountId, ObjectId, playerId, ObjectId, itemId, VaultId, vaultId, SlotId, slotId)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isFailed(errorCode)) {
        z2m_onPopGuildInventoryItem(errorCode, playerId, ItemInfo(), invalidSlotId);
        return;
    }

    ItemInfo guildItemInfo;
    errorCode = guild->popGuildInventoryItem(guildItemInfo, playerId, vaultId, itemId);
    
    z2m_onPopGuildInventoryItem(errorCode, playerId, guildItemInfo, slotId);    
}


RECEIVE_SRPC_METHOD_5(CommunityServerSideProxy, z2m_switchGuildInventoryItem,
    AccountId, accountId, ObjectId, playerId, VaultId, vaultId, ObjectId, guildItemId, ItemInfo, itemInfo)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isFailed(errorCode)) {
        z2m_onSwitchGuildInventoryItem(errorCode, playerId, invalidObjectId, ItemInfo());
        return;
    }

    ItemInfo guildItemInfo;

    errorCode = guild->switchGuildInventoryItem(guildItemInfo, playerId, vaultId, guildItemId, itemInfo);
    
    z2m_onSwitchGuildInventoryItem(errorCode, playerId, itemInfo.itemId_, guildItemInfo);;    
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_depositGuildGameMoney,
    AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isFailed(errorCode)) {
        z2m_onDepositGuildGameMoney(errorCode, playerId, 0);
        return;
    }
    errorCode = guild->deposit(playerId, gameMoney);
    z2m_onDepositGuildGameMoney(errorCode, playerId, gameMoney);
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_withdrawGuildGameMoney,
    AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney)
{
    CommunityGuildPtr guild;
    ErrorCode errorCode = queryGuild(guild, accountId);
    if (isFailed(errorCode)) {
        z2m_onWithdrawGuildGameMoney(errorCode, playerId, 0, 0, 0);
        return;
    }
    sec_t resetDayWithdrawTime = 0;
    GameMoney todayWithdraw = 0;
    errorCode = guild->withdraw(resetDayWithdrawTime, todayWithdraw, playerId, gameMoney);
    z2m_onWithdrawGuildGameMoney(errorCode, playerId, resetDayWithdrawTime, todayWithdraw, gameMoney);
}


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_onPushGuildInventoryItem,
    ErrorCode, errorCode, ObjectId, playerId, ObjectId, itemId);


FORWARD_SRPC_METHOD_4(CommunityServerSideProxy, z2m_onPopGuildInventoryItem,
    ErrorCode, errorCode, ObjectId, playerId, ItemInfo, itemInfo, SlotId, slotId);


FORWARD_SRPC_METHOD_4(CommunityServerSideProxy, z2m_onSwitchGuildInventoryItem,
    ErrorCode, errorCode, ObjectId, playerId, ObjectId, playerItemId, ItemInfo, itemInfo);


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_onDepositGuildGameMoney,
    ErrorCode, errorCode, ObjectId, playerId, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_5(CommunityServerSideProxy, z2m_onWithdrawGuildGameMoney,
    ErrorCode, errorCode, ObjectId, playerId, sec_t, resetDayWithdrawTime, GameMoney, todayWithdraw, GameMoney, gameMoney);

// = rpc::ZoneDominionRpc overriding

RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_addDominion,
    MapCode, worldMapCode, MoreBuildingDominionInfo, addInfo)
{
    COMMUNITYDOMINON_MANAGER->addDominion(worldMapCode, addInfo);
}


RECEIVE_SRPC_METHOD_2(CommunityServerSideProxy, z2m_removeDominion,
    MapCode, worldMapCode, ObjectId, buildingId)
{
    COMMUNITYDOMINON_MANAGER->removeDominion(worldMapCode, buildingId);
}


RECEIVE_SRPC_METHOD_3(CommunityServerSideProxy, z2m_setActivateDominion,
    MapCode, worldMapCode, ObjectId, buildingId, bool, isActivate)
{
    COMMUNITYDOMINON_MANAGER->setActivate(worldMapCode, buildingId, isActivate);
}

// = rpc::ZoneTeleportRpc overriding

RECEIVE_SRPC_METHOD_4(CommunityServerSideProxy, z2m_requestRecall,
    AccountId, callerId, Nickname, callerName, Nickname, callee, WorldPosition, worldPosition)
{
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(callerId);
    if (user) {
        user->recall(callerName, callee, worldPosition);
    }
}


FORWARD_SRPC_METHOD_3(CommunityServerSideProxy, z2m_evRecallRequested,
    Nickname, caller, Nickname, callee, WorldPosition, worldPosition);


RECEIVE_SRPC_METHOD_4(CommunityServerSideProxy,  z2m_requestTeleportToBuilding,
    ObjectId, ownerId, MapCode, worldMapCode, GuildId, guildId, ObjectId, buildingId)
{
    BuildingCode buildingCode = invalidBuildingCode; 
    WorldPosition worldPosition;
    const ErrorCode errorCode = COMMUNITYDOMINON_MANAGER->checkTeleport(buildingCode,
        worldPosition, worldMapCode, guildId, buildingId);
    z2m_onRequestTeleportToBuilding(errorCode, ownerId, buildingCode, buildingId, worldPosition);
}


FORWARD_SRPC_METHOD_5(CommunityServerSideProxy, z2m_onRequestTeleportToBuilding,
    ErrorCode, errorCode, ObjectId, ownerId,
    BuildingCode, toBuildingCode, ObjectId, fromBuildingId,
    WorldPosition, buildingPosition);

// = rpc::ZoneCheatRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityServerSideProxy, z2m_getWorldUserInfos,
    ObjectId, playerId)
{
    z2m_onGetWorldUserInfos(playerId, COMMUNITYUSER_MANAGER->getWorldUserInfos());
}


FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_onGetWorldUserInfos,
    ObjectId, playerId, WorldUserInfos, infos);

// = rpc::ZoneBuddyRpc overriding
FORWARD_SRPC_METHOD_2(CommunityServerSideProxy, z2m_onBuddyAdded,
    ObjectId, playerId, uint32_t, buddyCount);
}} // namespace gideon { namespace communityserver {
