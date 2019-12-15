#include "CommunityServerPCH.h"
#include "CommunityUserImpl.h"
#include "../../CommunityService.h"
#include "../../s2s/CommunityLoginServerProxy.h"
#include "../../s2s/CommunityServerSideProxyManager.h"
#include "../../c2s/CommunityClientSessionCallback.h"
#include "../../channel/WorldMapChannelManager.h"
#include "../../channel/WorldMapChannel.h"
#include "../../Party/CommunityPartyManager.h"
#include "../../Party/CommunityParty.h"
#include "../../guild/CommunityGuildManager.h"
#include "../../dominion/CommunityDominionManager.h"
#include "../CommunityUserHelper.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/concurrent/Future.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace communityserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(CommunityUserImpl);

CommunityUserImpl::CommunityUserImpl()
{
    reset();
}


CommunityUserImpl::~CommunityUserImpl()
{
}


void CommunityUserImpl::initialize(const AccountInfo& accountInfo,
    CommunityClientSessionCallback& sessionCallback,
    CommunityUserHelper& userHelper)
{
    SNE_ASSERT(accountInfo.isValid());

    std::unique_lock<LockType> lock(lock_);

    accountInfo_ = accountInfo;
    isInitBuddyInfos_ = false;

    userHelper_ = &userHelper;
    sessionCallback_ = &sessionCallback;
    clientId_ = sessionCallback_->getCurrentClientId();

    sne::base::Session* session = sessionCallback_->getCurrentSession();
    if (session != nullptr) {
        sne::sgp::RpcingExtension* rpcExtension =
            session->getExtension<sne::sgp::RpcingExtension>();
        rpcExtension->registerRpcForwarder(*this);
        rpcExtension->registerRpcReceiver(*this);
    }
}


void CommunityUserImpl::initPlayerInfo(sne::server::ServerId serverId, const PlayerInfo& playerInfo)
{	
    bool shouldInitBuddyInfos = false;
    {
        std::unique_lock<LockType> lock(lock_);

        playerInfo_ = playerInfo;
        if (! isInitBuddyInfos_) {
            shouldInitBuddyInfos = true;
            isInitBuddyInfos_ = true;
        }
        serverId_ = serverId;
    }

    WorldMapChannel::Ref shardChannel = WORLDMAP_CHANNEL_MANAGER->getShardChannel();
    assert(shardChannel);
    shardChannel->enter(accountInfo_.accountId_);

    if (isGuildMember()) {
        CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo.guildId_);
        if (guild) {
            guild->online(this);
        }
    }

    if (shouldInitBuddyInfos) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncGetBuddies(getAccountId(), getPlayerId());
    }
}


void CommunityUserImpl::characterClassChanged(ObjectId characterId, CharacterClass cc)
{
    bool isChanged = false;
    {
        std::unique_lock<LockType> lock(lock_);

        if (playerInfo_.playerId_ = characterId) {
            playerInfo_.characterClass_ = cc;
            isChanged = true;
        }
    }

    if (isChanged) {
        CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
        if (! party) {
            return;
        }
        party->characterClassChanged(characterId, cc);
    }

}


void CommunityUserImpl::updateCharacterLevel(ObjectId characterId, CreatureLevel level)
{
    bool isChanged = false;
    {
        std::unique_lock<LockType> lock(lock_);

        if (playerInfo_.playerId_ = characterId) {
            playerInfo_.level_ = level;
            isChanged = true;
        }
    }
    if (isChanged) {
        if (isGuildMember()) {
            CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
            if (guild) {
                guild->updateMemberLevel(playerInfo_.playerId_, level);
            }
        }
    }
}


void CommunityUserImpl::finalize()
{
    WorldMapChannel::Ref shardChannel = WORLDMAP_CHANNEL_MANAGER->getShardChannel();
    assert(shardChannel);
    shardChannel->leave(accountInfo_.accountId_);

    notifyOnlineStateBuddy(false);

    if (isPartyMember()) {
        CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
        if (party) {
            party->leave(getPlayerId());
        }
    }

    if (isGuildMember()) {
        CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
        if (guild) {
            guild->offline(playerInfo_.playerId_);
        }
    }

    {
        std::unique_lock<LockType> lock(lock_);

        sne::base::Session* session = sessionCallback_->getCurrentSession();
        if (session != nullptr) {
            sne::sgp::RpcingExtension* rpcExtension =
                session->getExtension<sne::sgp::RpcingExtension>();
            rpcExtension->unregisterRpcForwarder(*this);
            rpcExtension->unregisterRpcReceiver(*this);
        }

        reset();
    }
}


void CommunityUserImpl::notifyPartyInfo(PartyId partyId)
{
    if (partyId != partyId_) {
        return;
    }

    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (! party) {
        return;
    }

    party->memberRejoined(*this);
}


void CommunityUserImpl::expelledFromServer(ExpelReason expelReason)
{
    expelReason;
    // TODO:
    //evExpelled(expelReason);

    std::unique_lock<LockType> lock(lock_);

    sessionCallback_->expelledFromServer();
}


void CommunityUserImpl::channelEntered(ZoneId zoneId, ChannelId channelId, MapCode worldMapCode,
    ObjectId accountId, ObjectId playerId, const Position& position)
{
    {
        std::unique_lock<LockType> lock(lock_);

        if (! isValid()) {
            return;
        }

        if (isWorldMap(getMapType(worldMapCode))) {
            worldMapCode_ = worldMapCode;
            subMapCode_ = invalidMapCode;
        }
        else {
            subMapCode_ = worldMapCode;
        }

        if (playerInfo_.accountId_ != accountId || playerInfo_.playerId_ != playerId) {
            // 에러 로그
        }

        zoneId_ = zoneId;
        worldPosition_.mapCode_ = worldMapCode;
        static_cast<Position&>(worldPosition_) = position;

        worldMapChannelId_ = channelId;

        evWorldMapChannelEntered(worldPosition_.mapCode_);
    }

    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (party) {
        party->moveMap(playerInfo_.playerId_, worldMapCode_, subMapCode_);
    }
}


void CommunityUserImpl::channelLeft()
{
    {
        std::unique_lock<LockType> lock(lock_);

        if (! isValid()) {
            return;
        }

        subMapCode_ = invalidMapCode;

        evWorldMapChannelLeft(worldPosition_.mapCode_);

        worldPosition_.reset();
        worldMapChannelId_ = invalidChannelId;  
    }

    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (party) {
        party->moveMap(playerInfo_.playerId_, worldMapCode_, subMapCode_);
    }
}


bool CommunityUserImpl::isOnline() const
{
    std::unique_lock<LockType> lock(lock_);

    return playerInfo_.isValid();
}


bool CommunityUserImpl::isBlocked(ObjectId playerId) const
{
    std::unique_lock<LockType> lock(lock_);

    return isBlocked_i(playerId);
}


const Nickname CommunityUserImpl::getNickname() const
{
    std::unique_lock<LockType> lock(lock_);

    return playerInfo_.nickname_;
}


ObjectId CommunityUserImpl::getPlayerId() const
{
    std::unique_lock<LockType> lock(lock_);

    return playerInfo_.playerId_;
}


bool CommunityUserImpl::isPartyMember() const
{
    std::unique_lock<LockType> lock(lock_);

    return isValidPartyId(partyId_);
}


bool CommunityUserImpl::canPartyInviteable() const
{
    std::unique_lock<LockType> lock(lock_);

    auto party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (! party) {
        return true;
    }

    return party->canMoveOtherParty();
}


void CommunityUserImpl::partyInvited(const Nickname& nickname)
{
    evPartyInvited(nickname);
}


ErrorCode CommunityUserImpl::partyInviteSucceesed(PartyId& partyId, CommunityUser* user)
{    
    partyId = invalidPartyId;
    if (! isPartyMember()) {
        const ErrorCode errorCode = COMMUNITYPARTY_MANAGER->createParty(partyId, ptSolo, this);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        partyId_ = partyId;
    }
    
    auto party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (!party) {
        return ecPartyNotFindParty;
    }
    
    const ErrorCode errorCode =  party->invite(user, getPlayerId());
    if (isSucceeded(errorCode) && ! isValidPartyId(partyId)) {        
        partyId = partyId_;
    }

    evPartyInvitationReponded(user->getNickname(), true);

    return errorCode;
}


void CommunityUserImpl::partyInviteFailed(const Nickname& nickname)
{
    evPartyInvitationReponded(nickname, false);
}


void CommunityUserImpl::partyMemberAdded(const PartyMemberInfo& memberInfo)
{
    evPartyMeberAdded(memberInfo);
}


void CommunityUserImpl::partyJoined(const PartyMemberInfos& memberInfos, PartyType partyType)
{
    evPartyMemberInfos(memberInfos, partyType);
}


void CommunityUserImpl::partyMemberLeft(ObjectId playerId, bool isDestroy)
{
    evPartyMemberLeft(playerId);
    if (isDestroy) {
        partyId_ = invalidPartyId;
    }
}


void CommunityUserImpl::partyMemberKicked(ObjectId playerId)
{
    evPartyMemberKicked(playerId);
    if (getPlayerId() == playerId) {
        partyId_ = invalidPartyId;
    }
}


void CommunityUserImpl::partyMasterChanged(ObjectId masterId)
{
    evPartyMasterChanged(masterId);
}


void CommunityUserImpl::partyMemberMapMoved(ObjectId memberId, MapCode worldMapCode, MapCode subMapCode)
{
    evPartyMemberMapMoved(memberId, worldMapCode, subMapCode);
}


void CommunityUserImpl::partyMemberSaid(const PlayerInfo& playerInfo, const ChatMessage& message)
{
    evPartyMemberSaid(playerInfo.playerId_, playerInfo.nickname_, message);
}


void CommunityUserImpl::partyTypeChanged(PartyType partyType)
{
    evPartyTypeChanged(partyType);
}


void CommunityUserImpl::partyGroupPositionMoved(ObjectId memberId, const PartyPosition& position)
{
    evRaidPartyGroupPositionMoved(memberId, position);
}


void CommunityUserImpl::partyGroupPositionSwitched(ObjectId srcId, ObjectId descId)
{
    evRaidPartyGroupPositionSwitched(srcId, descId);
}


void CommunityUserImpl::partyGroupPositionInitialized(const PartyPositionMap& positionMap)
{
    evPartyGroupPositionInitialized(positionMap);
}


ErrorCode CommunityUserImpl::leavePartyBecauseMoveOrderParty()
{
    if (! isPartyMember()) {
        return ecOk;
    }

    auto party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (party) {
        if (! party->canMoveOtherParty()) {
            return ecPartySelfJoined;
        }

        party->leave(getPlayerId());
        if (party->shouldDestroy()) {
            COMMUNITYPARTY_MANAGER->destroyParty(partyId_);
        }
        onLeaveParty(ecOk);
        partyId_ = invalidPartyId;
    }
    
    return ecOk;
}



void CommunityUserImpl::guildMemberOnline(ObjectId playerId)
{
    evGuildMemberOnline(playerId);
}


void CommunityUserImpl::guildMemberOffline(ObjectId playerId)
{
    evGuildMemberOffline(playerId);
}


ErrorCode CommunityUserImpl::guildInvitationAccepted(CommunityUser& user)
{    
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
    if (!guild) {
        return ecGuildNotFindGuild;
    }

    const ErrorCode errorCode = guild->join(getPlayerId(), user);    

    evGuildInvitationResponded(user.getNickname(), true);

    return errorCode;  
}


void CommunityUserImpl::guildInvitationRejected(const Nickname& nickname)
{
    evGuildInvitationResponded(nickname, false);
}


void CommunityUserImpl::guildMasterChanged(ObjectId masterId)
{
    evGuildMasterChanged(masterId);
}


void CommunityUserImpl::guildInvited(const Nickname& nickname)
{
    evGuildInvited(nickname);
}


void CommunityUserImpl::guildMemberJoined(const GuildMemberInfo& guildMemberInfo)
{
    evGuildMemberJoined(guildMemberInfo);
}


void CommunityUserImpl::guildMemberLeft(ObjectId playerId)
{
    if (playerInfo_.playerId_ == playerId) {
        playerInfo_.guildId_ = invalidGuildId;
    }
    evGuildMemberLeft(playerId);
}


void CommunityUserImpl::guildMemberKicked(ObjectId playerId)
{
    if (playerInfo_.playerId_ == playerId) {
        playerInfo_.guildId_ = invalidGuildId;
    }
    evGuildMemberKicked(playerId);
}


void CommunityUserImpl::guildMemberSaid(const PlayerInfo& playerInfo, const ChatMessage& message)
{
    evGuildMemberSaid(playerInfo.playerId_, playerInfo.nickname_, message);
}


void CommunityUserImpl::guildRelationshipAdded(const GuildRelationshipInfo& guildRelationship)
{
    evGuildRelationshipAdded(guildRelationship);
}


void CommunityUserImpl::guildRelationshipRemoved(GuildId targetGuildId)
{
    evGuildRelationshipRemoved(targetGuildId);
}


void CommunityUserImpl::guildMemberPositionChanged(ObjectId playerId, GuildMemberPosition position)
{
    evGuildMemberPositionChanged(playerId, position);
}


void CommunityUserImpl::guildJoined(const BaseGuildInfo& guildInfo, 
    const GuildMemberInfoMap& guildMembers,
    const GuildRelationshipInfoMap& relaytionMap,
    const GuildIntroduction& introduction, const GuildNotice& guildNotice,
    const GuildRankInfos& rankInfos, const GuildLevelInfo& guildLevelInfo)
{
    playerInfo_.guildId_ = guildInfo.guildId_;

    evGuildInfo(guildInfo, guildMembers, relaytionMap, introduction, guildNotice, rankInfos, guildLevelInfo);
}


void CommunityUserImpl::guildIntroductionModified(const GuildIntroduction& introduction)
{
    evGuildIntroductionModified(introduction);
}


void CommunityUserImpl::guildNoticeModified(const GuildNotice& guildNotice)
{
    evGuildNoticeModified(guildNotice);
}


void CommunityUserImpl::guildSignUpResponded(const GuildName& guildName, bool isAccept)
{
    evGuildSignUpResponded(guildName, isAccept);
}


void CommunityUserImpl::guildRankAdded(const GuildRankInfo& rankInfo)
{
    evGuildRankAdded(rankInfo);
}


void CommunityUserImpl::guildRankDeleted(GuildRankId rankId)
{
    evGuildRankDeleted(rankId);
}


void CommunityUserImpl::guildRankSwapped(GuildRankId rankId1, GuildRankId rankId2)
{
    evGuildRankSwapped(rankId1, rankId2);
}


void CommunityUserImpl::guildRankNameUpdated(GuildRankId rankId, const GuildRankName& rankName)
{
    evGuildRankNameUpdated(rankId, rankName);
}


void CommunityUserImpl::guildRankRightsUpdated(GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay)
{
    evGuildRankRightsUpdated(rankId, rights, goldWithdrawalPerDay);
}


void CommunityUserImpl::guildBankRightsUpdated(GuildRankId rankId, VaultId id, uint32_t rights, uint32_t itemWithdrawalPerDay)
{
    evGuildBankRightsUpdated(rankId, id, rights, itemWithdrawalPerDay);
}


void CommunityUserImpl::guildMemberRankUpdated(ObjectId memberId, GuildRankId rankId)
{
    evGuildMemberRankUpdated(memberId, rankId);
}


void CommunityUserImpl::guildMemberLevelUpdated(ObjectId memberId, CreatureLevel level)
{
    evGuildMemberLevelUpdated(memberId, level);
}


void CommunityUserImpl::guildVaultAdded(const BaseVaultInfo& vaultInfo)
{
    evGuildVaultAdded(vaultInfo);
}


void CommunityUserImpl::guildInventoryItemCountUpdated(VaultId vaultId, ObjectId itemId, uint8_t count)
{
    evGuildInventoryItemCountUpdated(vaultId, itemId, count);
}


void CommunityUserImpl::guildInventoryItemAdded(VaultId vaultId, const ItemInfo& itemInfo)
{
    evGuildInventoryItemAdded(vaultId, itemInfo);
}


void CommunityUserImpl::guildInventoryItemRemoved(VaultId vaultId, ObjectId itemId)
{
    evGuildInventoryItemRemoved(vaultId, itemId);
}


void CommunityUserImpl::guildInventoryItemSwitched(VaultId vaultId, ObjectId  itemId1, ObjectId itemId2)
{
    evGuildInventoryItemSwitched(vaultId, itemId1, itemId2);
}


void CommunityUserImpl::guildInventoryItemMoved(VaultId vaultId, ObjectId itemId, SlotId slotId)
{
    evGuildInventoryItemMoved(vaultId, itemId, slotId);
}


void CommunityUserImpl::guildGameMoneyUpdated(GameMoney gameMoney)
{
    evGuildGameMoneyUpdated(gameMoney);
}


void CommunityUserImpl::guildVaultNameUpdated(VaultId vaultId, const VaultName& name)
{
    evGuildVaultNameUpdated(vaultId, name);
}
    

ErrorCode CommunityUserImpl::checkCreateGuild() const
{
    if (COMMUNITYGUILD_MANAGER->isGuildApplicant(getPlayerId())) {
        return ecGuildApplicant;
    }

    if (! playerInfo_.isValid()) {
        return ecServerInternalError;
    }

    if (isValidGuildId(playerInfo_.guildId_)) {
        return ecGuildAlreadyMember;
    }

    return ecOk;
}


ErrorCode CommunityUserImpl::checkInviteGuild(const Nickname& nickname, CommunityUser*& user) const
{
    user = userHelper_->getUser(nickname);
    if (! user) {
        return ecGuildIsNotMember;
    }

    if (COMMUNITYGUILD_MANAGER->isGuildApplicant(user->getPlayerId())) {
        return ecGuildApplicant;
    }
    
    if (! isValidGuildId(getGuildId())) {
        return ecGuildIsNotMember;
    }

    if (getPlayerId() == user->getPlayerId()) {
        return ecGuildNotInviteSelf;        
    }

    if (isValidGuildId(user->getGuildId())) {
        return ecGuildAlreadyMember;
    }

    return ecOk;
}


ErrorCode CommunityUserImpl::checkRespondInvite(const Nickname& nickname, CommunityUser*& user) const
{
    if (isGuildMember()) {        
        return ecGuildSelfJoined;
    }

    user = userHelper_->getUser(nickname);
    if (! user) {
        return ecGuildIsNotMember;
    }

    if (getPlayerId() == user->getPlayerId()) {
        return ecGuildNotInviteSelf;        
    }

    return ecOk;
}


ErrorCode CommunityUserImpl::checkRequestBuddy(const Nickname& nickname, CommunityUser*& user) const
{
    {
        std::unique_lock<LockType> lock(lock_);

        for (const BuddyInfo& info : buddyInfos_) {
            if (info.nickname_ == nickname) {
                return ecMessengerAlreadyBuddy;
            }
        }

        if (playerInfo_.nickname_ == nickname) {
            return ecMessengerSelfNotBuddy;
        }
    }

    user = userHelper_->getUser(nickname);
    if (! user) {
        return ecMessengerNotFindUser;
    }

    return ecOk;
}


ErrorCode CommunityUserImpl::checkResponseBuddy(const Nickname& nickname, CommunityUser*& user) const
{
    {
        std::unique_lock<LockType> lock(lock_);

        for (const BuddyInfo& info : buddyInfos_) {
            if (info.nickname_ == nickname) {
                return ecMessengerAlreadyBuddy;
            }
        }

        if (playerInfo_.nickname_ == nickname) {
            return ecMessengerSelfNotBuddy;
        }
    }

    user = userHelper_->getUser(nickname);
    if (! user) {
        return ecMessengerNotFindUser;
    }

    return ecOk;
}


ErrorCode CommunityUserImpl::blockPlayer(BlockInfo& blockInfo, const Nickname& nickname)
{
    {
        std::unique_lock<LockType> lock(lock_);

        for (const BlockInfo& info : blockInfos_) {
            if (info.nickname_ == nickname) {
                return ecMessengerAlreadyBlocked;
            }
        }

        if (playerInfo_.nickname_ == nickname) {
            return ecMessengerCannotBlockSelf;
        }
    }

    CommunityUser* user = userHelper_->getUser(nickname);
    if (! user) {
        return ecMessengerNotFindUser;
    }

    blockInfo.accountId_ = user->getAccountId();
    blockInfo.playerId_ = user->getPlayerId();
    blockInfo.nickname_ = user->getNickname();

    {
        std::unique_lock<LockType> lock(lock_);

        blockInfos_.push_back(blockInfo);
    }

    return ecOk;
}


ErrorCode CommunityUserImpl::unblockPlayer(ObjectId playerId)
{
    std::unique_lock<LockType> lock(lock_);

    auto pos = blockInfos_.begin();
    const auto end = blockInfos_.end();
    for (; pos != end; ++pos) {
        const BlockInfo& info = (*pos);
        if (info.playerId_ == playerId) {
            blockInfos_.erase(pos);
            return ecOk;
        }
    }

    return ecMessengerNotFindUser;
}


void CommunityUserImpl::whispered(ObjectId playerId, const Nickname& nickname, const ChatMessage& message)
{
    evWhispered(playerId, nickname, message);
}


void CommunityUserImpl:: recall(const Nickname& callerName, const Nickname& calleeName,
    const WorldPosition& worldPosition)
{
    const CommunityUser* user = userHelper_->getUser(calleeName);
    if (! user) {
        return;
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->recallRequested(user->getServerId(),
        callerName, calleeName, worldPosition);
}


void CommunityUserImpl::initBuddyInfo(const BuddyInfos& buddyInfos, const BlockInfos& blockInfos)
{
    BuddyInfos newBuddyInfos;
    for (const BuddyInfo& info : buddyInfos) {
        const bool isOnline = (userHelper_->getUser(info.accountId_) != nullptr);
        newBuddyInfos.emplace_back(info.accountId_, info.playerId_, info.nickname_, isOnline);
    }

    {
        std::unique_lock<LockType> lock(lock_);

        buddyInfos_ = newBuddyInfos;
        blockInfos_ = blockInfos;
    }

    notifyOnlineStateBuddy(true);

    evBuddyInfos(newBuddyInfos, blockInfos);
}


void CommunityUserImpl::buddyRequested(const Nickname& nickname)
{
    evBuddyRequested(nickname);
}


void CommunityUserImpl::buddyAdded(const BuddyInfo& buddyInfo)
{
    {
        std::unique_lock<LockType> lock(lock_);

        if (buddyInfo.isValid()) {
            buddyInfos_.push_back(buddyInfo);
        }
    }
    if (buddyInfo.isValid()) {
        evBuddyAdded(buddyInfo);
        COMMUNITYSERVERSIDEPROXY_MANAGER->buddyAdded(serverId_, getPlayerId(),
            static_cast<uint32_t>(buddyInfos_.size()));
    }
}


void CommunityUserImpl::buddyRemoved(ObjectId playerId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        BuddyInfos::iterator pos = buddyInfos_.begin();
        const BuddyInfos::iterator end = buddyInfos_.end();
        for (; pos != end; ++pos) {
            BuddyInfo& info = (*pos);
            if (info.playerId_ == playerId) {
                buddyInfos_.erase(pos);
                break;
            }
        }
    }
    evBuddyRemoved(playerId);
}


void CommunityUserImpl::buddyOnlineStateChanged(ObjectId playerId, bool isOnlineState)
{
    {
        std::unique_lock<LockType> lock(lock_);

        for (BuddyInfo& info : buddyInfos_) {
            if (info.playerId_ == playerId) {
                info.isOnline_ = isOnlineState;
                break;
            }
        }
    }

    evBuddyOnlineStateChanged(playerId, isOnlineState);
}


void CommunityUserImpl::notifyOnlineStateBuddy(bool isOnlineState)
{
    BuddyInfos infos;
    {
        std::unique_lock<LockType> lock(lock_);

        infos = buddyInfos_;
    }
    for (const BuddyInfo& info : infos) {
        CommunityUser* user = userHelper_->getUser(info.accountId_);
        if (user) {
            user->buddyOnlineStateChanged(getPlayerId(), isOnlineState);
        }	
    }
}


void CommunityUserImpl::reset()
{
    sessionCallback_ = nullptr;
    clientId_ = sne::server::ClientId::invalid;
    accountInfo_.reset();

    playerInfo_.reset();
    worldPosition_.reset();
    
    zoneId_ = invalidZoneId;
    partyId_ = invalidPartyId;
    worldMapCode_ = invalidMapCode;
    subMapCode_ = invalidMapCode;
    serverId_ = sne::server::ServerId::invalid;
    worldMapChannelId_ = invalidChannelId;
    isInitBuddyInfos_ = false;
    buddyInfos_.clear();
}


bool CommunityUserImpl::canRequest() const
{
    std::unique_lock<LockType> lock(lock_);

    if (! isValid()) {
        return false;
    }

    if (! sessionCallback_) {
        return false;
    }

    return true;
}

// = sne::srpc::RpcForwarder overriding

void CommunityUserImpl::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG(__FUNCTION__ "(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void CommunityUserImpl::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG(__FUNCTION__ "(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::ShardChannelRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityUserImpl, sayInShard,
    ChatMessage, message)
{
    WorldMapChannel::Ref channel = WORLDMAP_CHANNEL_MANAGER->getShardChannel();
    if (! channel) {
        return;
    }

    PlayerInfo playerInfo;
    {
        std::unique_lock<LockType> lock(lock_);

        if (! isValid()) {
            return;
        }

        playerInfo = playerInfo_;
    }

    channel->say(playerInfo, message);
    SNE_LOG_INFO("GameLog shard say(PID:%" PRIu64 ", Say:[%W]))",
        playerInfo.playerId_, message.c_str());
}


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evShardSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message);

// = rpc::WorldMapChannelRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityUserImpl, sayInWorldMap,
    ChatMessage, message)
{
    WorldMapChannel::Ref channel = WORLDMAP_CHANNEL_MANAGER->getChannel(worldMapChannelId_);
    if (! channel) {
        return;
    }

    PlayerInfo playerInfo;
    {
        std::unique_lock<LockType> lock(lock_);

        if (! isValid()) {
            return;
        }

        playerInfo = playerInfo_;
    }

    channel->say(playerInfo, message);
    SNE_LOG_INFO("GameLog world say(PID:%" PRIu64 ", Say:[%W]))",
        playerInfo.playerId_, message.c_str());
}


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evWorldMapChannelEntered,
    MapCode, worldMapCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evWorldMapChannelLeft,
    MapCode, worldMapCode);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evWorldMapSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message);

// = rpc::CommunityPartyRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityUserImpl, inviteParty,
    Nickname, nickname)
{
    PartyId partyId = invalidPartyId;
    {
        std::unique_lock<LockType> lock(lock_);

        if (! isValid()) {
            return;
        }

        partyId = partyId_;
    }

    if (playerInfo_.nickname_ == nickname) {
        onInviteParty(ecPartyNotInviteSelf, nickname);
        return;
    }

    if (isValidPartyId(partyId)) {
        const CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId);
        if (party) {
            const ErrorCode errorCode = party->canInvite(getPlayerId());
            if (isFailed(errorCode)) {
                onInviteParty(errorCode, nickname);
                return;
            }            
        }
    }
    
    CommunityUser* user = userHelper_->getUser(nickname);
    if (! user) {
        onInviteParty(ecPartyNotFindUser, nickname);
        return;
    }

    if (! user->canPartyInviteable()) {
        onInviteParty(ecPartyAlreadyMember, nickname);
        return;
    }

    if (! user->isBlocked(playerInfo_.playerId_)) {
        user->partyInvited(getNickname());
    }

    onInviteParty(ecOk, nickname);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, respondPartyInvitation,
    Nickname, nickname, bool, isAnswer)
{
    ErrorCode errorCode = leavePartyBecauseMoveOrderParty();
    if (isFailed(errorCode)) {
        onInviteParty(ecPartySelfJoined, nickname);
        return;
    }

    CommunityUser* user = userHelper_->getUser(nickname);
    if (! user) {
        onRespondPartyInvitation(ecPartyNotFindUser);
        return;
    }

    if (isAnswer) {       
        PartyId partyId = invalidPartyId;
        errorCode = user->partyInviteSucceesed(partyId, this);
        if (isSucceeded(errorCode)) {
            partyId_ = partyId;
        }
    }
    else {
        user->partyInviteFailed(getNickname());
    }

    onRespondPartyInvitation(errorCode);    
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, leaveParty)
{
    if (! isPartyMember()) {
        onLeaveParty(ecPartyIsNotMember);
        return;
    }

    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (!party) {
        onLeaveParty(ecPartyNotFindParty);
        return;
    }

    party->leave(getPlayerId());
    if (party->shouldDestroy()) {
        COMMUNITYPARTY_MANAGER->destroyParty(partyId_);
    }

    partyId_ = invalidPartyId;
    onLeaveParty(ecOk);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, kickPartyMember,
    ObjectId, playerId)
{
    if (! isPartyMember()) {
        onKickPartyMember(ecPartyIsNotMember);
        return;
    }

    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (!party) {
        onKickPartyMember(ecPartyNotFindParty);
        return;
    }
    
    const ErrorCode errorCode = party->kick(getPlayerId(), playerId);
    if (isSucceeded(errorCode)) {
        if (party->shouldDestroy()) {
            COMMUNITYPARTY_MANAGER->destroyParty(partyId_);
            partyId_ = invalidPartyId;
        }
    }
    else {
        onKickPartyMember(errorCode);
    }
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, delegatePartyMaster,
    ObjectId, newMasterId)
{
    if (! isPartyMember()) {
        onDelegatePartyMaster(ecPartyIsNotMember);
        return;
    }

    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (!party) {
        onDelegatePartyMaster(ecPartyNotFindParty);
        return;
    }

    const ErrorCode errorCode = party->delegateMaster(getPlayerId(), newMasterId);
    if (isFailed(errorCode)) {
        onDelegatePartyMaster(errorCode);
    }
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, sayPartyMember,
    ChatMessage, message)
{
    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (party) {
        party->say(playerInfo_, message);
    }
    SNE_LOG_DEBUG("GameLog party say(PID:%" PRIu64 ", Say:[%W]))",
        playerInfo_.playerId_, message.c_str());
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, setWaypoint,
    Waypoint, waypoint)
{
    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
    if (party) {
        party->setWaypoint(playerInfo_.playerId_, waypoint);
    }
    else {
        evWaypointSet(playerInfo_.playerId_, waypoint);
    }
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, responsePublicParty,
    PartyId, partyId)
{
    ErrorCode errorCode = leavePartyBecauseMoveOrderParty();
    if (isFailed(errorCode)) {
        onResponsePublicParty(ecPartyAlreadyMember, partyId_);
        return;
    } 
    
    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId);
    if (party) {
        errorCode = party->invite(this, invalidObjectId);
        if (isSucceeded(errorCode)) {
            partyId_ = partyId;
        }
    }
    onResponsePublicParty(errorCode, partyId_);
}



RECEIVE_SRPC_METHOD_1(CommunityUserImpl, changePartyType,
    PartyType, partyType)
{
    ErrorCode errorCode = ecPartyNotFindParty;

    if (isPartyMember()) {
        CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
  
        if (party) {
            errorCode = party->changePartyType(getPlayerId(), partyType);
        }
    }

    if (isFailed(errorCode)) {
        onChangePartyType(errorCode, partyType);
    }
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, moveRaidPartyGroupPosition,
    ObjectId, playerId, PartyPosition, position)
{
    ErrorCode errorCode = ecPartyNotFindParty;

    if (isPartyMember()) {
        CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
        if (party) {
            errorCode = party->moveRaidPartyGroupPosition(getPlayerId(), playerId, position);
        }
    }

    if (isFailed(errorCode)) {
        onMoveRaidPartyGroupPosition(errorCode, playerId, position);
    }
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, switchRaidPartyGroupPosition,
    ObjectId, srcId, ObjectId, descId)
{
    ErrorCode errorCode = ecPartyNotFindParty;

    if (isPartyMember()) {
        CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId_);
        if (party) {
            errorCode = party->switchRaidPartyGroupPosition(getPlayerId(), srcId, descId);
        }
    }

    if (isFailed(errorCode)) {
        onSwitchRaidPartyGroupPosition(errorCode, srcId, descId);
    }
}


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onInviteParty,
    ErrorCode, errorCode, Nickname, nickname);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onRespondPartyInvitation,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onLeaveParty,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onKickPartyMember,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onDelegatePartyMaster,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onResponsePublicParty,
    ErrorCode, errorCode, PartyId, partyId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onChangePartyType,
    ErrorCode, errorCode, PartyType, partyType);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onMoveRaidPartyGroupPosition,
    ErrorCode, errorCode, ObjectId, playerId, PartyPosition, position); 


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onSwitchRaidPartyGroupPosition,
    ErrorCode, errorCode, ObjectId, srcId, ObjectId, descId); 


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evPartyInvited,
    Nickname, nickname);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evPartyInvitationReponded,
    Nickname, nickname, bool, isAnswer);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evPartyMeberAdded,
    PartyMemberInfo, memberInfo);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evPartyMemberInfos,
    PartyMemberInfos, memberInfos, PartyType, partyType);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evPartyMemberLeft,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evPartyMemberKicked,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evPartyMasterChanged,
    ObjectId, masterId);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evPartyMemberMapMoved,
    ObjectId, memberId, MapCode, worldMapCode, MapCode, subMapCode);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evPartyMemberSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evPartyTypeChanged,
    PartyType, partyType);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evRaidPartyGroupPositionMoved,
    ObjectId, playerId, PartyPosition, position); 


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evRaidPartyGroupPositionSwitched,
    ObjectId, srcId, ObjectId, descId); 


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evPartyGroupPositionInitialized,
    PartyPositionMap, positionMap);

// = WhisperRpc overriding

RECEIVE_SRPC_METHOD_2(CommunityUserImpl, whisper,
    Nickname, nickname, ChatMessage, message)
{
    CommunityUser* user = userHelper_->getUser(nickname);
    if (! user) {
        onWhisper(ecChattingNotFindPlayer, nickname, message);
        return;
    }

    if (! user->isBlocked(playerInfo_.playerId_)) {
        user->whispered(playerInfo_.playerId_, playerInfo_.nickname_, message);
    }

    onWhisper(ecOk, nickname, message);

    SNE_LOG_INFO("GameLog whisper say(PID:%" PRIu64 ", To:[%W], Say:[%W]))",
        playerInfo_.playerId_, nickname.c_str(), message.c_str());
}


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onWhisper,
    ErrorCode, errorCode, Nickname, nickname, ChatMessage, message);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evWhispered,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message);

// = rpc::NoticeRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityUserImpl, noticeToShard,
    ChatMessage, message)
{
    if (! canCheat(accountInfo_.accountGrade_)) {
        return;
    }

    WORLDMAP_CHANNEL_MANAGER->noticeToShard(message);
}


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evShardNoticed,
    ChatMessage, message);

// = rpc::CommunityGuildRpc overriding


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, createGuild,
    GuildName, guildName, GuildMarkCode, guildMarkCode)
{
    ErrorCode errorCode = checkCreateGuild();
    if (isSucceeded(errorCode)) {
        errorCode = COMMUNITYGUILD_MANAGER->createGuild(*this, guildName, guildMarkCode);
    }

    onCreateGuild(errorCode);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, inviteGuild,
    Nickname, nickname)
{
    CommunityUser* user = nullptr;
    const ErrorCode errorCode = checkInviteGuild(nickname, user);
    if (isSucceeded(errorCode)) {
        if (user) {
            if (! user->isBlocked(playerInfo_.playerId_)) {
                user->guildInvited(getNickname());
            }
        } 
    }
    onInviteGuild(errorCode, nickname);
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, leaveGuild)
{
    if (! isGuildMember()) {
        onLeaveGuild(ecGuildIsNotMember);
        return;
    }

    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
    if (!guild) {
        onLeaveGuild(ecGuildNotFindGuild);
        return;
    }

    const ErrorCode errorCode = guild->leave(getPlayerId());
    if (isSucceeded(errorCode)) {
        if (guild->shouldDestory()) {
            COMMUNITYGUILD_MANAGER->removeGuild(playerInfo_.guildId_);
        }
        playerInfo_.guildId_ = invalidGuildId;
    }
    onLeaveGuild(errorCode);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, respondGuildInvitation,
    Nickname, nickname, bool, isAnswer)
{
    CommunityUser* user = nullptr;
    ErrorCode errorCode = checkRespondInvite(nickname, user);
    if (isSucceeded(errorCode)) {
        if (isAnswer) {       
            errorCode = user->guildInvitationAccepted(*this);            
        }
        else {
            user->guildInvitationRejected(getNickname());
        }
    }

    onRespondGuildInvitation(errorCode, getGuildId());
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, sayGuildMember,
    ChatMessage, message)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
    if (guild) {
        guild->say(playerInfo_, message);
    }
    SNE_LOG_INFO("GameLog guild say(PID:%" PRIu64 ", Say:[%W]))",
        playerInfo_.playerId_, message.c_str());
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, kickGuildMember, 
    ObjectId, playerId)
{
    if (getPlayerId() == playerId) {
        onKickGuildMember(ecGuildNotFindGuild, playerId);
        return;
    }

    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
    if (!guild) {
        onKickGuildMember(ecGuildNotKickSelf, playerId);
        return;
    }
    
    const ErrorCode errorCode = guild->kick(getPlayerId(), playerId);
    onKickGuildMember(errorCode, playerId);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, addGuildRelationship,
    GuildId, targetGuildId, GuildRelatioshipType, type)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
    if (!guild) {
        onAddGuildRelationship(ecGuildIsNotMember);
        return;
    }

    CommunityGuildPtr targetGuild = COMMUNITYGUILD_MANAGER->getGuild(targetGuildId);
    if (! targetGuild) {
        onAddGuildRelationship(ecGuildNotFindGuild);
        return;
    }

    GuildRelationshipInfo relationShip;
    BaseGuildInfo baseInfo = targetGuild->getBaseGuildInfo();
    relationShip.targetGuildId_ = targetGuildId;
    relationShip.guildName_ = baseInfo.guildName_;
    relationShip.guildMarkCode_ = baseInfo.guildMarkCode_;
    relationShip.relationship_ = type;
    relationShip.relationshipTime_ = getTime();

    const ErrorCode errorCode = guild->addRelationship(getPlayerId(), relationShip);
    if (isFailed(errorCode)) {
        onAddGuildRelationship(errorCode);
    }
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, removeGuildRelationship,
    GuildId, targetGuildId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(playerInfo_.guildId_);
    if (!guild) {
        onRemoveGuildRelationship(ecGuildNotFindGuild);
        return;
    }
    const ErrorCode errorCode = guild->removeRelationship(getPlayerId(), targetGuildId);
    if (isFailed(errorCode)) {
        onRemoveGuildRelationship(errorCode);
    }
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, searchGuildInfo,
    GuildName, searchGuildName)
{
    SearchGuildInfos infos = COMMUNITYGUILD_MANAGER->searchGuilds(searchGuildName,
        playerInfo_.guildId_);
    onSearchGuildInfo(ecOk, infos);
}



RECEIVE_SRPC_METHOD_1(CommunityUserImpl, signupGuild,
    GuildId, guildId)
{
    if (isValidGuildId(playerInfo_.guildId_)) {
        onSignupGuild(ecGuildAlreadyMember);
        return;
    }

    if (COMMUNITYGUILD_MANAGER->isGuildApplicant(playerInfo_.playerId_)) {
        onSignupGuild(ecGuildAlreadyMember);
        return;
    }

    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(guildId);
    if (!guild) {
        onSignupGuild(ecGuildNotFindGuild);
        return;
    }

    guild->addGuildApplicant(playerInfo_.playerId_, playerInfo_.nickname_, 
        playerInfo_.characterClass_, worldMapCode_);
    COMMUNITYGUILD_MANAGER->addGuildApplicant(playerInfo_.playerId_, guildId);
    onSignupGuild(ecOk);
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, queryGuildApplicants)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onGuildApplicants(ecGuildNotFindGuild, GuildApplicantInfoMap());
        return;
    }

    GuildApplicantInfoMap guildApplicantInfoMap = guild->getGuildApplicantInfoMap();
    onGuildApplicants(ecOk, guildApplicantInfoMap);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, confirmGuildApplicant,
    ObjectId, playerId, bool, isAccept)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onConfirmGuildApplicant(ecGuildNotFindGuild, playerId);
        return;
    }

    ErrorCode errorCode = guild->confirmGuildApplicant(*userHelper_, getPlayerId(), playerId, isAccept);
    if (isSucceeded(errorCode)) {
        COMMUNITYGUILD_MANAGER->removeGuildApplicant(playerId);
    }
    
    onConfirmGuildApplicant(errorCode, playerId);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, queryGuildMember,
    GuildId, guildId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(guildId);
    if (!guild) {
        onGuildMemberInfos(ecGuildNotFindGuild, GuildMemberInfoMap());
        return;
    }

    onGuildMemberInfos(ecOk, guild->getGuildMembers());
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, queryMyGuildApplicantId)
{
    onMyGuildApplicantId(COMMUNITYGUILD_MANAGER->getGuildApplicantId(getPlayerId()));
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, queryDetailSearchGuildInfo,
    GuildId, guildId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(guildId);
    if (!guild) {
        onDetailSearchGuildInfo(ecGuildNotFindGuild, DetailSearchGuildInfo());
        return;
    }

    onDetailSearchGuildInfo(ecOk, guild->getDetailSearchGuildInfo());
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, cancelGuildApplicant)
{
    const ObjectId playerId = getPlayerId();
    const GuildId guildId = COMMUNITYGUILD_MANAGER->getGuildApplicantId(playerId);
    if (! isValidGuildId(guildId)) {
        onCancelGuildApplicant(ecGuildNotApplicant);
        return;
    }

    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(guildId);
    if (!guild) {
        onCancelGuildApplicant(ecGuildNotFindGuild);
        return;
    }

    const ErrorCode errorCode = guild->removeGuildApplicant(playerId);
    if (isSucceeded(errorCode)) {
        COMMUNITYGUILD_MANAGER->removeGuildApplicant(playerId);
    }
    onCancelGuildApplicant(errorCode);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, modifyGuildIntroduction,
    GuildIntroduction, introduction)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onModifyGuildIntroduction(ecGuildNotFindGuild);
        return;
    }
    const ErrorCode errorCode = guild->modifyGuildIntroduction(getPlayerId(), introduction);
    if (isFailed(errorCode)) {
        onModifyGuildIntroduction(errorCode);
    }
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, modifyGuildNotice,
    GuildNotice, notice)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onModifyGuildNotice(ecGuildNotFindGuild);
        return;
    }
    const ErrorCode errorCode = guild->modifyGuildNotice(getPlayerId(), notice);
    if (isFailed(errorCode)) {
        onModifyGuildNotice(errorCode);
    }
}



RECEIVE_SRPC_METHOD_0(CommunityUserImpl, queryMyGuildInfo)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (guild) {
        guild->queryGuildInfo(*this);
    }
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, queryMyGuildSkillInfo)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (guild) {
        guild->querySkillInfo(*this);
    }
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, queryMyGuildLevelInfo)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (guild) {
        guild->queryLevelInfo(*this);
    }
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, queryGuildEventLogs, 
    uint8_t, beginLogIndex, uint8_t, logCount)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onGuildEventLogs(ecGuildNotFindGuild, beginLogIndex, logCount,0, GuildEventLogInfoList());
        return;    
    }
    uint8_t totalLogCount = 0;
    GuildEventLogInfoList infoList;
    const ErrorCode errorCode = guild->fillEventLogInfo(infoList, totalLogCount, beginLogIndex, logCount);
    onGuildEventLogs(errorCode, beginLogIndex, logCount,totalLogCount, infoList);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, addGuildRank, 
    GuildRankName, rankName)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onAddGuildRank(ecGuildNotFindGuild, GuildRankInfo());
        return;    
    }

    GuildRankInfo rankInfo;
    const ErrorCode errorCode = guild->addGuildRank(rankInfo, getPlayerId(), rankName);
    onAddGuildRank(errorCode, rankInfo);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, deleteGuildRank,
    GuildRankId, rankId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onDeleteGuildRank(ecGuildNotFindGuild, rankId);
        return;    
    }

    const ErrorCode errorCode = guild->deleteGuildRank(getPlayerId(), rankId);
    onDeleteGuildRank(errorCode, rankId);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, swapGuildRank,
    GuildRankId, rankId1, GuildRankId, rankId2)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onSwapGuildRank(ecGuildNotFindGuild, rankId1, rankId2);
        return;    
    }

    const ErrorCode errorCode = guild->swapGuildRank(getPlayerId(), rankId1, rankId2);
    onSwapGuildRank(errorCode, rankId1, rankId2);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, setGuildRankName, 
    GuildRankId, rankId, GuildRankName, rankName)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onSetGuildRankName(ecGuildNotFindGuild, rankId, GuildRankName());
        return;    
    }

    const ErrorCode errorCode = guild->setGuildRankName(getPlayerId(), rankId, rankName);
    onSetGuildRankName(errorCode, rankId, rankName);
}


RECEIVE_SRPC_METHOD_3(CommunityUserImpl, updateGuildRankRights,
    GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onUpdateGuildRankRights(ecGuildNotFindGuild, rankId, 0, 0);
        return;    
    }

    const ErrorCode errorCode = guild->updateGuildRankRights(getPlayerId(), rankId, rights, goldWithdrawalPerDay);
    onUpdateGuildRankRights(errorCode, rankId, rights, goldWithdrawalPerDay);
}


RECEIVE_SRPC_METHOD_4(CommunityUserImpl, updateGuildBankRights,
    GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onUpdateGuildBankRights(ecGuildNotFindGuild, rankId, vaultId, 0, 0);
        return;    
    }

    const ErrorCode errorCode = guild->updateGuildBankRights(getPlayerId(), rankId, vaultId, rights, itemWithdrawalPerDay);
    onUpdateGuildBankRights(errorCode, rankId, vaultId, rights, itemWithdrawalPerDay);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, updateGuildRank,
    ObjectId, memberId, GuildRankId, rankId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onUpdateGuildRank(ecGuildNotFindGuild, memberId, rankId);
        return;    
    }

    const ErrorCode errorCode = guild->updateGuildMemberRank(getPlayerId(), memberId, rankId);
    onUpdateGuildRank(errorCode, memberId, rankId);
}


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onInviteGuild,
    ErrorCode, errorCode, Nickname, nickname);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onCreateGuild,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onRespondGuildInvitation,
    ErrorCode, errorCode, GuildId, guildId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onSignupGuild,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onLeaveGuild,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onKickGuildMember, 
    ErrorCode, errorCode, ObjectId, playerId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onAddGuildRelationship,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onRemoveGuildRelationship,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onSearchGuildInfo,
    ErrorCode, errorCode, SearchGuildInfos, searchGuildInfos);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onGuildApplicants,
    ErrorCode, errorCode, GuildApplicantInfoMap, guildApplicantInfoMap);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onConfirmGuildApplicant,
    ErrorCode, errorCode, ObjectId, playerId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onGuildMemberInfos,
    ErrorCode, errorCode, GuildMemberInfoMap, guildMemberInfoMap);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onMyGuildApplicantId,
    GuildId, guildId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onDetailSearchGuildInfo,
    ErrorCode, errorCode, DetailSearchGuildInfo, info);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onCancelGuildApplicant,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onModifyGuildIntroduction,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onModifyGuildNotice,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_5(CommunityUserImpl, onGuildEventLogs, 
    ErrorCode, errorCode, uint8_t, pageIndex,
    uint8_t, pageCount, uint8_t, totalLogCount, GuildEventLogInfoList, bankLogList);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onAddGuildRank, 
    ErrorCode, errorCode, GuildRankInfo, rankInfo);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onDeleteGuildRank,
    ErrorCode, errorCode, GuildRankId, rankId);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onSwapGuildRank, 
    ErrorCode, errorCode, GuildRankId, rankId1, GuildRankId, rankId2);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onSetGuildRankName, 
    ErrorCode, errorCode, GuildRankId, rankId, GuildRankName, rankName);


FORWARD_SRPC_METHOD_4(CommunityUserImpl, onUpdateGuildRankRights, 
    ErrorCode, errorCode, GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);


FORWARD_SRPC_METHOD_5(CommunityUserImpl, onUpdateGuildBankRights, 
    ErrorCode, errorCode, GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay)


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onUpdateGuildRank, 
    ErrorCode, errorCode, ObjectId, memberId, GuildRankId, rankId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildInvited,
    Nickname, nickname);


FORWARD_SRPC_METHOD_7(CommunityUserImpl, evGuildInfo,
    BaseGuildInfo, guildInfo, GuildMemberInfoMap, guildMembers,
    GuildRelationshipInfoMap, relaytionMap,
    GuildIntroduction, introduction, GuildNotice, guildNotice,
    GuildRankInfos, rankInfos, GuildLevelInfo, guildLevelInfo);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildSkillInfos,
    GuildId, guildId, SkillCodes, guildSkills);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildLevelInfo,
    GuildId, guildId, GuildLevelInfo, guildLevelInfo);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildInvitationResponded,
    Nickname, nickname, bool, isAnswer);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evGuildMemberSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildMemberJoined,
    GuildMemberInfo, guildMemberInfo);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildMemberLeft,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildMemberKicked,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildMemberOnline,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildMemberOffline,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildRelationshipAdded,
    GuildRelationshipInfo, relationshipInfo);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildRelationshipRemoved,
    GuildId, targetGuildId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildMemberPositionChanged,
    ObjectId, playerId, GuildMemberPosition, position);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildIntroductionModified,
    GuildIntroduction, introduction);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildNoticeModified,
    GuildNotice, notice);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildSignUpResponded,
    GuildName, guildName, bool, isAccept);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildMasterChanged,
    ObjectId, masterId);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildRankAdded,
    GuildRankInfo, rankInfo);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildRankDeleted,
    GuildRankId, rankId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildRankSwapped,
    GuildRankId, rankId1, GuildRankId, rankId2);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildRankNameUpdated,
    GuildRankId, rankId, GuildRankName, rankName);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evGuildRankRightsUpdated,
    GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);


FORWARD_SRPC_METHOD_4(CommunityUserImpl, evGuildBankRightsUpdated,
    GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay)


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildMemberRankUpdated,
    ObjectId, memberId, GuildRankId, rankId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildMemberLevelUpdated,
    ObjectId, memberId, CreatureLevel, level);

// = CommunityGuildInventoryRpc overriding

RECEIVE_SRPC_METHOD_0(CommunityUserImpl, queryGuildBankInfo)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onGuildBankInfo(ecGuildNotFindGuild, BaseVaultInfos(), gameMoneyMin);
        return;
    }

    GameMoney gameMoney = gameMoneyMin;
    BaseVaultInfos vaultInfos;
    const ErrorCode errorCode = guild->fillBankInfo(vaultInfos, gameMoney, getPlayerId());
    onGuildBankInfo(errorCode, vaultInfos, guild->getGameMoney());
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, openGuildInventory,
    VaultId, vaultId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onOpenGuildInventory(ecGuildNotFindGuild, vaultId, InventoryInfo());
        return;
    }

    InventoryInfo inventoryInfo;
    const ErrorCode errorCode = guild->openGuildInventory(vaultId, inventoryInfo, this);
    onOpenGuildInventory(errorCode, vaultId, inventoryInfo);
}


RECEIVE_SRPC_METHOD_0(CommunityUserImpl, closeGuildInventory)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (guild) {
        guild->closeGuildInventory(getPlayerId());
    } 
}


RECEIVE_SRPC_METHOD_3(CommunityUserImpl, moveGuildInventoryItem,
    VaultId, vaultId, ObjectId, itemId, SlotId, slotId)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onMoveGuildInventoryItem(ecGuildNotFindGuild);
        return;
    }

    const ErrorCode errorCode = guild->moveGuildInventoryItem(getPlayerId(), vaultId, itemId, slotId);
    if (isFailed(errorCode)) {
        onMoveGuildInventoryItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_3(CommunityUserImpl, switchGuildInventoryItem,
    VaultId, vaultId, ObjectId, itemId1, ObjectId, itemId2)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onSwitchGuildInventoryItem(ecGuildNotFindGuild);
        return;
    }

    const ErrorCode errorCode = guild->switchGuildInventoryItem(getPlayerId(), vaultId, itemId1, itemId2);
    if (isFailed(errorCode)) {
        onSwitchGuildInventoryItem(errorCode);
    }
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, queryGuildGameMoneyEventLogs, 
    uint8_t, beginLogIndex, uint8_t, logCount)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onGuildGameMoneyEventLogs(ecGuildNotFindGuild, beginLogIndex, logCount,0, GuildGameMoneyEventLogInfoList());
        return;    
    }
    uint8_t totalLogCount = 0;
    GuildGameMoneyEventLogInfoList infoList;
    const ErrorCode errorCode = guild->fillGameMoneyEventLogInfo(infoList, totalLogCount, beginLogIndex, logCount);
    onGuildGameMoneyEventLogs(errorCode, beginLogIndex, logCount,totalLogCount, infoList);
}



RECEIVE_SRPC_METHOD_2(CommunityUserImpl, queryGuildBankEventLogs, 
    uint8_t, beginLogIndex, uint8_t, logCount)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onGuildBankEventLogs(ecGuildNotFindGuild, beginLogIndex, logCount,0, GuildBankEventLogInfoList());
        return;
    }
    uint8_t totalLogCount = 0;
    GuildBankEventLogInfoList infoList;
    const ErrorCode errorCode = guild->fillBankEventLogInfo(infoList, totalLogCount, beginLogIndex, logCount);
    onGuildBankEventLogs(errorCode, beginLogIndex, logCount,totalLogCount, infoList);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, updateGuildVaultName,
    VaultId, vaultId, VaultName, name)
{
    CommunityGuildPtr guild = COMMUNITYGUILD_MANAGER->getGuild(getGuildId());
    if (!guild) {
        onUpdateGuildVaultName(ecGuildNotFindGuild);
        return;
    }
    const ErrorCode errorCode = guild->updateVaultName(getPlayerId(), vaultId, name);
    if (isFailed(errorCode)) {
        onUpdateGuildVaultName(errorCode);
    }
}


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onGuildBankInfo,
    ErrorCode, errorCode, BaseVaultInfos, infos, GameMoney, gameMoney);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, onOpenGuildInventory,
    ErrorCode, errorCode, VaultId, vaultId, InventoryInfo, inventoryInfo);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onMoveGuildInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onSwitchGuildInventoryItem,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_5(CommunityUserImpl, onGuildGameMoneyEventLogs, 
    ErrorCode, errorCode, uint8_t, pageIndex, uint8_t, pageCount, 
    uint8_t, totalLogCount, GuildGameMoneyEventLogInfoList, logList);


FORWARD_SRPC_METHOD_5(CommunityUserImpl, onGuildBankEventLogs, 
    ErrorCode, errorCode, uint8_t, pageIndex,  uint8_t, pageCount, 
    uint8_t, totalLogCount, GuildBankEventLogInfoList, bankLogList);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onUpdateGuildVaultName,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildVaultAdded,
    BaseVaultInfo, vaultInfo);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evGuildInventoryItemCountUpdated,
    VaultId, vaultId, ObjectId, itemId, uint8_t, ItemCount);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildInventoryItemAdded,
    VaultId, vaultId, ItemInfo, itemInfo);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildInventoryItemRemoved,
    VaultId, vaultId, ObjectId, itemId);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evGuildInventoryItemMoved,
    VaultId, vaultId, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_3(CommunityUserImpl, evGuildInventoryItemSwitched,
    VaultId, vaultId, ObjectId, itemId1, ObjectId, itemId2);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evGuildGameMoneyUpdated,
    GameMoney, currentGameMoney);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evGuildVaultNameUpdated,
    VaultId, vaultId, VaultName, name);

// = CommunityDominionRpc overriding

RECEIVE_SRPC_METHOD_0(CommunityUserImpl, queryWorldDominion)
{
    onWorldDominion(COMMUNITYDOMINON_MANAGER->getDominionWorldInfoMap());
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, queryZoneDominion,
    MapCode, worldMapCode)
{
    const BuildingDominionInfos* infos = 
        COMMUNITYDOMINON_MANAGER->getDominionZoneInfos(worldMapCode);
    if (infos) {
        onZoneDominion(*infos);
    }
    else {
        onZoneDominion(BuildingDominionInfos());
    }
}



FORWARD_SRPC_METHOD_1(CommunityUserImpl, onWorldDominion,
    DominionWorldInfoMap, infoMap);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, onZoneDominion,
    BuildingDominionInfos, infos);

// = CommunityMessengerRpc overriding

RECEIVE_SRPC_METHOD_1(CommunityUserImpl, requestBuddy,
    Nickname, nickname)
{
    CommunityUser* user = nullptr;
    const ErrorCode ec = checkRequestBuddy(nickname, user);
    if (isSucceeded(ec)) {
        if (! user->isBlocked(playerInfo_.playerId_)) {
            user->buddyRequested(getNickname());
        }
    }

    onRequestBuddy(ec, nickname);
}


RECEIVE_SRPC_METHOD_2(CommunityUserImpl, repsonseBuddy,
    Nickname, requester, bool, isAnswer)
{
    BuddyInfo buddyInfo;
    ErrorCode ec = ecOk;
    if (isAnswer) {
        CommunityUser* user = nullptr;
        ec = checkResponseBuddy(requester, user);
        if (isSucceeded(ec) && user) {
            buddyInfo.accountId_ = user->getAccountId();
            buddyInfo.playerId_ = user->getPlayerId();
            buddyInfo.nickname_ = user->getNickname();
            buddyInfo.isOnline_ = true;
            {
                std::unique_lock<LockType> lock(lock_);

                buddyInfos_.push_back(buddyInfo);
            }

            COMMUNITYSERVERSIDEPROXY_MANAGER->buddyAdded(serverId_, getPlayerId(), buddyInfos_.size());
            BuddyInfo selfInfo(getAccountId(), getPlayerId(), getNickname(), true);
            user->buddyAdded(selfInfo);

            {
                DatabaseGuard db(SNE_DATABASE_MANAGER);
                db->asyncAddBuddy(user->getPlayerId(), getPlayerId());
            }
        }
    }

    onRepsonseBuddy(ec, buddyInfo);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, removeBuddy,
    ObjectId, playerId)
{
    ErrorCode ec = ecMessengerNotBuddy;
    AccountId accountId = invalidAccountId;
    {	
        std::unique_lock<LockType> lock(lock_);

        auto pos = buddyInfos_.begin();
        const auto end = buddyInfos_.end();
        for (; pos != end; ++pos) {
            BuddyInfo& info = (*pos);
            if (info.playerId_ == playerId) {
                accountId = info.accountId_;
                buddyInfos_.erase(pos);
                ec = ecOk;
                break;
            }
        }
    }
    if (isSucceeded(ec)) {
        CommunityUser* user = userHelper_->getUser(accountId, playerId);
        if (user) {
            user->buddyRemoved(getPlayerId());
        }
        {
            DatabaseGuard db(SNE_DATABASE_MANAGER);
            db->asyncRemoveBuddy(getPlayerId(), playerId);
        }
    }
    onRemoveBuddy(ec, playerId);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, addBlock,
    Nickname, nickname)
{
    BlockInfo blockInfo;

    const ErrorCode errorCode = blockPlayer(blockInfo, nickname);
    if (isSucceeded(errorCode)) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddBlock(getPlayerId(), blockInfo.playerId_);
    }

    onAddBlock(errorCode, blockInfo);
}


RECEIVE_SRPC_METHOD_1(CommunityUserImpl, removeBlock,
    ObjectId, playerId)
{
    const ErrorCode errorCode = unblockPlayer(playerId);
    if (isSucceeded(errorCode)) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveBlock(getPlayerId(), playerId);
    }

    onRemoveBlock(errorCode, playerId);
}


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onRequestBuddy,
    ErrorCode, errorCode, Nickname, nickname);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onRepsonseBuddy,
    ErrorCode, errorCode, BuddyInfo, buddyInfo);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onRemoveBuddy,
    ErrorCode, errorCode, ObjectId, playerId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onAddBlock,
    ErrorCode, errorCode, BlockInfo, blockInfo);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, onRemoveBlock,
    ErrorCode, errorCode, ObjectId, playerId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evBuddyInfos,
    BuddyInfos, buddyInfos, BlockInfos, blockInfos);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evBuddyRequested,
    Nickname, nickname);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evBuddyAdded,
    BuddyInfo, buddy);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evBuddyOnlineStateChanged,
    ObjectId, playerId, bool, isOnline);


FORWARD_SRPC_METHOD_1(CommunityUserImpl, evBuddyRemoved,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_2(CommunityUserImpl, evWaypointSet,
    ObjectId, playerId, Waypoint, waypoint);

}} // namespace gideon { namespace communityserver {
