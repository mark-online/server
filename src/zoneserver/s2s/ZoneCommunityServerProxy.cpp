#include "ZoneServerPCH.h"
#include "ZoneCommunityServerProxy.h"
#include "../ZoneService.h"
#include "../world/World.h"
#include "../world/WorldMap.h"
#include "../user/ZoneUserManager.h"
#include "../user/ZoneUser.h"
#include "../service/party/Party.h"
#include "../service/party/PartyService.h"
#include "../service/guild/Guild.h"
#include "../service/guild/GuildService.h"
#include "../service/world_event/WorldEventService.h"
#include "../service/world_event/WorldEvent.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/RandomDungeon.h"
#include "../model/gameobject/ability/Partyable.h"
#include "../model/gameobject/ability/Guildable.h"
#include "../model/gameobject/ability/Moneyable.h"
#include "../model/gameobject/ability/Inventoryable.h"
#include "../model/gameobject/ability/Achievementable.h"
#include "../model/item/Inventory.h"
#include "../model/state/ItemManageState.h"
#include "../controller/EntityController.h"
#include "../controller/callback/PartyCallback.h"
#include "../controller/callback/TeleportCallback.h"
#include "../controller/callback/GuildCallback.h"
#include "../controller/callback/BuildingTeleportCallback.h"
#include "../controller/callback/CheatCallback.h"
#include "../controller/callback/OutsideInventoryCallback.h"
#include "../helper/InventoryHelper.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>
#include <sne/database/DatabaseManager.h>

namespace gideon { namespace zoneserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(ZoneCommunityServerProxy);

ZoneCommunityServerProxy::ZoneCommunityServerProxy(
    const sne::server::ServerInfo& serverInfo,
    const sne::server::ClientSideSessionConfig& config) :
    sne::server::ClientSideProxy(serverInfo, config),
    isRendezvoused_(false),
    isReadied_(false)
{
    sne::base::Session* session = getSession();
    if (session != nullptr) {
        sne::sgp::RpcingExtension* extension =
            session->getImpl().getExtension<sne::sgp::RpcingExtension>();
        extension->registerRpcForwarder(*this);
        extension->registerRpcReceiver(*this);
    }
}


ZoneCommunityServerProxy::~ZoneCommunityServerProxy()
{
}


void ZoneCommunityServerProxy::ready()
{
    isReadied_ = true;

    sendReadyMessage();
}


void ZoneCommunityServerProxy::sendReadyMessage()
{
    z2m_ready();
}

// = sne::server::ClientSideProxy overriding

void ZoneCommunityServerProxy::onAuthenticated()
{
    sne::server::ClientSideProxy::onAuthenticated();

    z2m_rendezvous(WORLD->getZoneId());
}


void ZoneCommunityServerProxy::onDisconnected()
{
    sne::server::ClientSideProxy::onDisconnected();

    // TODO: 커뮤니티 서버와의 연결이 끊긴 경우의 처리
    //ZONE_SERVICE->communityServerDisconnected();

    isRendezvoused_ = false;
}

// = sne::srpc::RpcForwarder overriding

void ZoneCommunityServerProxy::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneCommunityServerProxy::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void ZoneCommunityServerProxy::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneCommunityServerProxy::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::ZoneCommunityRpc overriding

FORWARD_SRPC_METHOD_1(ZoneCommunityServerProxy, z2m_rendezvous,
    ZoneId, zoneId);


RECEIVE_SRPC_METHOD_0(ZoneCommunityServerProxy, z2m_onRendezvous)
{
    isRendezvoused_ = true;

    ZONE_SERVICE->communityServerConnected();

    if (isReadied_) {
        sendReadyMessage();
    }
}


FORWARD_SRPC_METHOD_0(ZoneCommunityServerProxy, z2m_ready);


FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_worldMapOpened,
    MapCode, worldMapCode, ObjectId, mapId);


FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_worldMapClosed,
    MapCode, worldMapCode, ObjectId, mapId);


FORWARD_SRPC_METHOD_1(ZoneCommunityServerProxy, z2m_initPlayerInfo,
    PlayerInfo, playerInfo);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_changeCharacterClass,
    AccountId, accountId, ObjectId, characterId, CharacterClass, cc);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_updateCreatureLevel,
    AccountId, accountId, ObjectId, characterId, CreatureLevel, level);


FORWARD_SRPC_METHOD_4(ZoneCommunityServerProxy, z2m_worldMapEntered,
    ObjectId, mapId, AccountId, accountId, ObjectId, playerId,
    Position, position);


FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_worldMapLeft,
    AccountId, accountId, ObjectId, playerId);


FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_deleteCharacter,
	ObjectId, characterId, GuildId, guildId);

// = rpc::ZonePartyRpc overriding


FORWARD_SRPC_METHOD_1(ZoneCommunityServerProxy, z2m_createRandumDungeonPublicParty,
    GameObjectInfo, randumDungionInfo);


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_onCreateRandumDungeonPublicParty,
    GameObjectInfo, randumDungionInfo, PartyId, partyId)
{
    PartyPtr party = PARTY_SERVICE->getPartyByPartyId(partyId);
    if (party.get() == nullptr) {
        PARTY_SERVICE->createParty(partyId, ptPublic);
    }

    go::Entity* entity = WORLD->getGlobalWorldMap().getEntity(randumDungionInfo);
    if ((! entity) || (! entity->isDungeon())) {
        return;
    }
    
   static_cast<go::RandomDungeon&>(*entity).setPartyId(partyId);
}


FORWARD_SRPC_METHOD_1(ZoneCommunityServerProxy, z2m_destoryRandumDungeonPublicParty,
    PartyId, partyId);



FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_createWorldEventPublicParty,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode);


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_onCreateWorldEventPublicParty,
    WorldEventCode, worldEventCode, WorldEventMissionCode, missionCode, PartyId, partyId)
{
    PartyPtr party = PARTY_SERVICE->getPartyByPartyId(partyId);
    if (party.get() == nullptr) {
        PARTY_SERVICE->createParty(partyId, ptPublic);
    }

    WorldEvent* worldEvent = WORLD_EVENT_SERVICE->getWorldEvent(worldEventCode);
    if (worldEvent) {
        worldEvent->setParty(missionCode, partyId);
    }
}


FORWARD_SRPC_METHOD_1(ZoneCommunityServerProxy, z2m_destoryWorldEventPublicParty,
    PartyId, partyId);

FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_notifyPartyInfo,
    PartyId, partyId, AccountId, accountId);


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_evPartyMemberJoined,
    PartyId, partyId, ObjectId, playerId)
{
    PartyPtr party = PARTY_SERVICE->getPartyByPartyId(partyId);
    if (party.get() == nullptr) {
        PARTY_SERVICE->createParty(partyId, ptSolo);
        party = PARTY_SERVICE->getPartyByPartyId(partyId);
    }

    PARTY_SERVICE->addPartyMember(partyId, playerId);
    party->addMember(playerId); 

    go::Entity* player = WORLD->getPlayer(playerId);
    if (player) {
        go::Partyable* partyable = player->queryPartyable();
        if (partyable) {
            partyable->setParty(party);
        }
    }
}


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_evPartyMemberCharacterClassChanged,
    PartyId, partyId, ObjectId, playerId, CharacterClass, cc)
{
    PartyPtr party = PARTY_SERVICE->getPartyByPartyId(partyId);;
    if (party.get() != nullptr) {
        party->characterClassChanged(playerId, cc);
    }
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_evPartyMemberLeft,
    PartyId, partyId, ObjectId, playerId)
{
    PartyPtr party = PARTY_SERVICE->getPartyByPartyId(partyId);
    if (party.get() != nullptr) {
        go::Entity* player = party->getMember(playerId);
        if (player) {
            player->queryPartyable()->setParty(PartyPtr());
        }
        PARTY_SERVICE->removePartyMember(playerId);
        party->removeMember(playerId);
        if (party->shouldDestroy()) {
            PARTY_SERVICE->destoryParty(partyId);
        }
    }   
}


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_evPartyMemberRejoined,
    PartyId, partyId, AccountId, accountId, ObjectId, playerId)
{
    PartyPtr party = PARTY_SERVICE->getPartyByPartyId(partyId);
    if (! party.get()) {
        return;
    }

    go::Entity* player = party->getMember(playerId);
    if (! player) {
        return;
    }

    player->queryPartyable()->setParty(party);

    z2m_notifyPartyInfo(partyId, accountId);
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_evPartyTypeChanged,
    PartyId, partyId, PartyType, partyType)
{
    PartyPtr party = PARTY_SERVICE->getPartyByPartyId(partyId);
    if (! party.get()) {
        return;
    }

    party->setPartyType(partyType);
}

// = rpc::ZoneGuildRpc overriding

RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_createGuild,
    GuildMemberInfo, guildMemberInfo, BaseGuildInfo, guildInfo)
{
    GUILD_SERVICE->createGuild(guildInfo);
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildInfo.guildId_);
    if (guild.get()) {
        guild->addMember(guildMemberInfo);
        go::Entity* player = WORLD->getPlayer(guildMemberInfo.playerId_);
        if (player) {
            guild->online(*player);            
            player->queryGuildable()->setGuildInfo(guildInfo);
            player->getController().queryGuildCallback()->create(guild->getBaseGuildInfo());
            player->queryMoneyable()->downGameMoney(createGuildMoney);
        }
    }
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_addGuildMember,
    GuildId, guildId, GuildMemberInfo, guildMemberInfo)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->addMember(guildMemberInfo);
        go::Entity* player = WORLD->getPlayer(guildMemberInfo.playerId_);
        if (player) {
            guild->online(*player);
            player->queryGuildable()->setGuildInfo(guild->getBaseGuildInfo());
            player->getController().queryGuildCallback()->join(guild->getBaseGuildInfo());
        }
    }
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_removeGuildMember,
    GuildId, guildId, ObjectId, characterId)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->removeMember(characterId);
        go::Entity* player = WORLD->getPlayer(characterId);
        if (player) {
            player->queryGuildable()->setGuildInfo(BaseGuildInfo());
            player->getController().queryGuildCallback()->leave();
        }
		if (guild->shouldDestory()) {
			GUILD_SERVICE->removeGuild(guildId);
		}
    }
}


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_changeGuildMemberPosition,
	GuildId, guildId, ObjectId, characterId, GuildMemberPosition, position)
{
	GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
	if (guild.get()) {
		guild->changeGuildMemberPosition(characterId, position);		
	}
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_addGuildRelationship,
    GuildId, guildId, GuildRelationshipInfo, info)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->addRelationship(info);		
    }
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_removeGuildRelationship,
    GuildId, guildId, GuildId, targetGuildId)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->removeRelationship(targetGuildId);		
    }
}


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_purchaseVault,
    AccountId, accountId, BaseVaultInfo, vaultInfo, ObjectId, playerId);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_activateGuildSkill,
    AccountId, accountId, ObjectId, playerId, SkillCode, skillCode);


FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_deactivateGuildSkills,
    AccountId, accountId, ObjectId, playerId);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_addGuildExp,
    AccountId, accountId, ObjectId, playerId, GuildExp, exp);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_addCheatGuildExp,
    AccountId, accountId, ObjectId, playerId, GuildExp, exp);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_addGuildGameMoney,
    AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_onPurchaseVault,
    ErrorCode, errorCode, ObjectId, playerId, BaseVaultInfo, vaultInfo)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (player) {
        player->getController().queryGuildCallback()->purchaseVaultResponsed(errorCode, vaultInfo);
    }    
}


RECEIVE_SRPC_METHOD_4(ZoneCommunityServerProxy, z2m_onActivateGuildSkill,
    ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId, SkillCode, skillCode)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->activateSkill(errorCode, playerId, skillCode);
    }    
}



RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_onDeactivateGuildSkills,
    ErrorCode, errorCode, GuildId, guildId, ObjectId, playerId)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->deactivateSkills(errorCode, playerId);
    }    
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_guildLevelUpdated,
    GuildId, guildId, GuildLevel, guildLevel)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->setGuildLevel(guildLevel);
    }    
}


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_updateGuildExpAddibleState,
    GuildId, guildId, bool, guildExpAddible)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
    if (guild.get()) {
        guild->setAddibleDayExpState(guildExpAddible);
    }    
}



// = rpc::ZoneguildInventoryRpc
FORWARD_SRPC_METHOD_5(ZoneCommunityServerProxy, z2m_pushGuildInventoryItem,
    AccountId, accountId, ObjectId, playerId, ItemInfo, itemInfo, VaultId, vaultId, SlotId, slotId);


FORWARD_SRPC_METHOD_5(ZoneCommunityServerProxy, z2m_popGuildInventoryItem,
    AccountId, accountId, ObjectId, playerId, ObjectId, itemId, VaultId, vaultId, SlotId, slotId);


FORWARD_SRPC_METHOD_5(ZoneCommunityServerProxy, z2m_switchGuildInventoryItem,
    AccountId, accountId, ObjectId, playerId, VaultId, vaultId, ObjectId, guildItemId, ItemInfo, itemInfo);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_depositGuildGameMoney,
    AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_withdrawGuildGameMoney,
    AccountId, accountId, ObjectId, playerId, GameMoney, gameMoney);


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_onPushGuildInventoryItem,
    ErrorCode, errorCode, ObjectId, playerId, ObjectId, itemId)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (! player) {
        return;
    }

    player->queryItemManageState()->unlockLnventory();

    if (isSucceeded(errorCode)) {
        player->queryInventoryable()->getInventory().removeItem(itemId);
    }
    else {
        player->getController().queryOutsideInventoryCallback()->guildInventoryItemPushed(errorCode);
    }
}


RECEIVE_SRPC_METHOD_4(ZoneCommunityServerProxy, z2m_onPopGuildInventoryItem,
    ErrorCode, errorCode, ObjectId, playerId, ItemInfo, itemInfo, SlotId, slotId)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (! player) {
        return;
    }
    player->queryItemManageState()->unlockLnventory();

    if (isSucceeded(errorCode)) {
        player->queryInventoryable()->getInventory().addItem(createAddItemInfoByItemInfo(itemInfo), slotId);
    }
    else {
        player->getController().queryOutsideInventoryCallback()->guildInventoryItemPopped(errorCode);
    }
}


RECEIVE_SRPC_METHOD_4(ZoneCommunityServerProxy, z2m_onSwitchGuildInventoryItem,
    ErrorCode, errorCode, ObjectId, playerId, ObjectId, playerItemId, ItemInfo, itemInfo)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (! player) {
        return;
    }

    player->queryItemManageState()->unlockLnventory();

    if (isSucceeded(errorCode)) {
        ItemInfo playerItemInfo = player->queryInventoryable()->getInventory().getItemInfo(playerItemId);
        player->queryInventoryable()->getInventory().removeItem(playerItemId);
        player->queryInventoryable()->getInventory().addItem(createAddItemInfoByItemInfo(itemInfo), playerItemInfo.slotId_);
    }
    else {
        player->getController().queryOutsideInventoryCallback()->guildInventoryItemSwitched(errorCode);
    }
}


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_onDepositGuildGameMoney,
    ErrorCode, errorCode, ObjectId, playerId, GameMoney, gameMoney)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (! player) {
        return;
    }

    player->queryItemManageState()->unlockLnventory();
    player->getController().queryOutsideInventoryCallback()->guildGameMoneyDepositted(errorCode, gameMoney);    
}


RECEIVE_SRPC_METHOD_5(ZoneCommunityServerProxy, z2m_onWithdrawGuildGameMoney,
    ErrorCode, errorCode, ObjectId, playerId, sec_t, resetDayWithdrawTime,
    GameMoney, todayWithdraw, GameMoney, gameMoney)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (! player) {
        return;
    }
    player->queryItemManageState()->unlockLnventory();

    player->getController().queryOutsideInventoryCallback()->guildGameMoneyWithdrawed(errorCode, resetDayWithdrawTime, todayWithdraw, gameMoney);    
}


// = rpc::ZoneDominionRpc overriding

FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_addDominion,
	MapCode, worldMapCode, MoreBuildingDominionInfo, addInfo);


FORWARD_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_removeDominion,
	MapCode, worldMapCode, ObjectId, buildingId);


FORWARD_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_setActivateDominion,
	MapCode, worldMapCode, ObjectId, buildingId, bool, isActivate);

// = rpc::ZoneTeleportRpc overriding

FORWARD_SRPC_METHOD_4(ZoneCommunityServerProxy, z2m_requestRecall,
	AccountId, callerId, Nickname, callerName, Nickname, callee, WorldPosition, worldPosition);


RECEIVE_SRPC_METHOD_3(ZoneCommunityServerProxy, z2m_evRecallRequested,
	Nickname, callerName, Nickname, calleeName, WorldPosition, worldPosition)
{
	go::Entity* player = WORLD->getPlayer(calleeName);
	if (player) {
		gc::TeleportCallback* callback = player->getController().queryTeleportCallback();
		if (callback) {
			callback->recallRequested(callerName, worldPosition);
		}
	}
}


FORWARD_SRPC_METHOD_4(ZoneCommunityServerProxy, z2m_requestTeleportToBuilding,
	ObjectId, ownerId, MapCode, worldMapCode, GuildId, guildId, ObjectId, buildingId);


RECEIVE_SRPC_METHOD_5(ZoneCommunityServerProxy, z2m_onRequestTeleportToBuilding,
	ErrorCode, errorCode, ObjectId, ownerId, BuildingCode, toBuildingCode, 
	ObjectId, fromBuildingId, WorldPosition, buildingPosition)
{
	go::Entity* player = WORLD->getPlayer(ownerId);
	if (player) {
		gc::BuildingTeleportCallback* callback = player->getController().queryBuildingTeleportCallback();
		if (callback) {
			callback->teleportToOrderZone(errorCode, toBuildingCode, fromBuildingId, buildingPosition);
		}
	}
}

// = rpc::ZoneCheatRpc overriding

FORWARD_SRPC_METHOD_1(ZoneCommunityServerProxy, z2m_getWorldUserInfos,
    ObjectId, playerId);


RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy, z2m_onGetWorldUserInfos,
    ObjectId, playerId, WorldUserInfos, worldUserInfos)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (player) {
        player->getController().queryCheatCallback()->cheatWorldInUsers(worldUserInfos);
    }
}


// = rpc::ZoneBuddyRpc overridng
RECEIVE_SRPC_METHOD_2(ZoneCommunityServerProxy,z2m_onBuddyAdded,
    ObjectId, playerId, uint32_t, buddyCount)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (player) {
        player->queryAchievementable()->updateAchievement(acmtAddBuddy, buddyCount, 0);
    }
}

}} // namespace gideon { namespace zoneserver {
