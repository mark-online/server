#pragma once

#include "../CommunityUser.h"
#include <gideon/cs/shared/data/PartyInfo.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace communityserver {

/**
 * @class CommunityUserImpl
 *
 * Community 사용자 실제 구현
 */
class CommunityServer_Export CommunityUserImpl :
    public CommunityUser,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(CommunityUserImpl);

    typedef std::mutex LockType;

public:
    CommunityUserImpl();
    virtual ~CommunityUserImpl();

public: // 테스트 편의를 위해
    virtual void initialize(const AccountInfo& accountInfo,
        CommunityClientSessionCallback& sessionCallback,
        CommunityUserHelper& userHelper);
    virtual void finalize();

    virtual void initPlayerInfo(sne::server::ServerId serverId, const PlayerInfo& playerInfo);
    virtual void characterClassChanged(ObjectId characterId, CharacterClass cc);
    virtual void updateCharacterLevel(ObjectId characterId, CreatureLevel level);

private:
    virtual void notifyPartyInfo(PartyId partyId);

private:
    virtual void expelledFromServer(ExpelReason expelReason);

private:
    virtual void channelEntered(ZoneId zoneId, ChannelId channelId, MapCode worldMapCode,
        ObjectId accountId, ObjectId playerId, const Position& position);
    virtual void channelLeft();

private:
    virtual bool isOnline() const;

    virtual bool isBlocked(ObjectId playerId) const;

private:
    virtual sne::server::ServerId getServerId() const {
        return serverId_;
    }

    virtual MapCode getWorldMapCode() const {
        return worldMapCode_;
    }

    virtual MapCode getSubMapCode() const {
        return subMapCode_;
    }

    virtual ZoneId getZoneId() const {
        return zoneId_;
    }

    virtual PartyId getPartyId() const {
        return partyId_;
    }

    virtual sne::server::ClientId getClientId() const {
        return clientId_;
    }

    virtual AccountId getAccountId() const {
        return accountInfo_.accountId_;
    }

    virtual const Nickname getNickname() const;
    virtual ObjectId getPlayerId() const;
    virtual CharacterClass getCharacterClass() const {
        return playerInfo_.characterClass_;
    }
    virtual CreatureLevel getLevel() const {
        return playerInfo_.level_;
    }

    virtual ChannelId getWorldMapChannelId() const {
        return worldMapChannelId_;
    }

private:
    // = 파티
    virtual bool isPartyMember() const;
    virtual bool canPartyInviteable() const;

    virtual void partyInvited(const Nickname& nickname);

    virtual ErrorCode partyInviteSucceesed(PartyId& partyId, CommunityUser* user);
    virtual void partyInviteFailed(const Nickname& nickname);

    virtual void partyMemberAdded(const PartyMemberInfo& memberInfo);
    virtual void partyJoined(const PartyMemberInfos& memberInfos, PartyType partyType);
    virtual void partyMemberLeft(ObjectId playerId, bool isDestroy);
    virtual void partyMemberKicked(ObjectId playerId);
    virtual void partyMasterChanged(ObjectId masterId);
    virtual void partyMemberMapMoved(ObjectId memberId, MapCode worldMapCode, MapCode subMapCode);
    virtual void partyMemberSaid(const PlayerInfo& playerInfo, const ChatMessage& message);
    virtual void partyTypeChanged(PartyType partyType);
    virtual void partyGroupPositionMoved(ObjectId memberId, const PartyPosition& position);
    virtual void partyGroupPositionSwitched(ObjectId srcId, ObjectId descId);
    virtual void partyGroupPositionInitialized(const PartyPositionMap& positionMap);

    /// 다른 파티로 가기위해 현재 파티에 가입되어 있으면 떠난다
    ErrorCode leavePartyBecauseMoveOrderParty();
    
private:
    virtual void guildMasterChanged(ObjectId masterId);

    virtual void guildInvited(const Nickname& nickname);
    virtual void guildMemberLeft(ObjectId playerId);
    virtual void guildMemberKicked(ObjectId playerId);
    virtual ErrorCode guildInvitationAccepted(CommunityUser& user);
    virtual void guildInvitationRejected(const Nickname& nickname);
    virtual void guildMemberJoined(const GuildMemberInfo& guildMemberInfo);
    virtual void guildMemberOnline(ObjectId playerId);
    virtual void guildMemberOffline(ObjectId playerId);

    virtual void guildRelationshipAdded(const GuildRelationshipInfo& guildRelationship);
    virtual void guildRelationshipRemoved(GuildId targetGuildId);
    virtual void guildMemberPositionChanged(ObjectId playerId, GuildMemberPosition position);

    virtual void guildMemberSaid(const PlayerInfo& playerInfo, const ChatMessage& message);
    virtual void guildJoined(const BaseGuildInfo& guildInfo, 
        const GuildMemberInfoMap& guildMembers,
        const GuildRelationshipInfoMap& relaytionMap,
        const GuildIntroduction& introduction, const GuildNotice& guildNotice,
        const GuildRankInfos& rankInfos, const GuildLevelInfo& guildLevelInfo);
    virtual void guildIntroductionModified(const GuildIntroduction& introduction);
    virtual void guildNoticeModified(const GuildNotice& guildNotice);
    virtual void guildSignUpResponded(const GuildName& guildName, bool isAccept);

    virtual void guildRankAdded(const GuildRankInfo& rankInfo);
    virtual void guildRankDeleted(GuildRankId rankId);
    virtual void guildRankSwapped(GuildRankId rankId1, GuildRankId rankId2);
    virtual void guildRankNameUpdated(GuildRankId rankId, const GuildRankName& rankName);
    virtual void guildRankRightsUpdated(GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay);
    virtual void guildBankRightsUpdated(GuildRankId rankId, VaultId id, uint32_t rights, uint32_t itemWithdrawalPerDay);
    virtual void guildMemberRankUpdated(ObjectId memberId, GuildRankId rankId);
    virtual void guildMemberLevelUpdated(ObjectId memberId, CreatureLevel level);

    virtual void guildVaultAdded(const BaseVaultInfo& vaultInfo);

    virtual void guildInventoryItemCountUpdated(VaultId vaultId, ObjectId itemId, uint8_t count);
    virtual void guildInventoryItemAdded(VaultId vaultId, const ItemInfo& itemInfo);
    virtual void guildInventoryItemRemoved(VaultId vaultId, ObjectId itemId);
    virtual void guildInventoryItemSwitched(VaultId vaultId, ObjectId  itemId1, ObjectId itemId2);
    virtual void guildInventoryItemMoved(VaultId vaultId, ObjectId itemId, SlotId slotId);
    virtual void guildGameMoneyUpdated(GameMoney gameMoney);
    virtual void guildVaultNameUpdated(VaultId vaultId, const VaultName& name);

    virtual GuildId getGuildId() const {
        return playerInfo_.guildId_;
    }

private:
    virtual void whispered(ObjectId playerId, const Nickname& nickname, const ChatMessage& message);

private:
    void recall(const Nickname& callerName, const Nickname& calleeName,
        const WorldPosition& worldPosition);

private:
    virtual void initBuddyInfo(const BuddyInfos& buddyInfos, const BlockInfos& blockInfos);
    virtual void buddyRequested(const Nickname& nickname);
    virtual void buddyAdded(const BuddyInfo& buddyInfo);
    virtual void buddyRemoved(ObjectId playerId);
    virtual void buddyOnlineStateChanged(ObjectId playerId, bool isOnlineState);

    void notifyOnlineStateBuddy(bool isOnlineState);
    
private:
    ErrorCode checkCreateGuild() const;
    ErrorCode checkInviteGuild(const Nickname& nickname, CommunityUser*& user) const;
    ErrorCode checkRespondInvite(const Nickname& nickname, CommunityUser*& user) const;
    ErrorCode checkRequestBuddy(const Nickname& nickname, CommunityUser*& user) const;
    ErrorCode checkResponseBuddy(const Nickname& nickname, CommunityUser*& user) const;

    ErrorCode blockPlayer(BlockInfo& blockInfo, const Nickname& nickname);
    ErrorCode unblockPlayer(ObjectId playerId);

private:
    bool isGuildMember() const {
        return isValidGuildId(playerInfo_.guildId_);
    }

    bool isBlocked_i(ObjectId playerId) const {
        for (const BlockInfo& info : blockInfos_) {
            if (info.playerId_ == playerId) {
                return true;
            }
        }
        return false;
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    // = rpc::ShardChannelRpc overriding
    OVERRIDE_SRPC_METHOD_1(sayInShard,
        ChatMessage, message);

    OVERRIDE_SRPC_METHOD_3(evShardSaid,
        ObjectId, playerId, Nickname, nickname, ChatMessage, message);

public:
    // = rpc::WorldMapChannelRpc overriding
    OVERRIDE_SRPC_METHOD_1(sayInWorldMap,
        ChatMessage, message);

    OVERRIDE_SRPC_METHOD_1(evWorldMapChannelEntered,
        MapCode, worldMapCode);
    OVERRIDE_SRPC_METHOD_1(evWorldMapChannelLeft,
        MapCode, worldMapCode);
    OVERRIDE_SRPC_METHOD_3(evWorldMapSaid,
        ObjectId, playerId, Nickname, nickname, ChatMessage, message);

public:
    // = rpc::CommunityPartyRpc overriding
    OVERRIDE_SRPC_METHOD_1(inviteParty,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(respondPartyInvitation,
        Nickname, nickname, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_0(leaveParty);
    OVERRIDE_SRPC_METHOD_1(kickPartyMember,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(sayPartyMember,
        ChatMessage, message);
    OVERRIDE_SRPC_METHOD_1(setWaypoint,
        Waypoint, waypoint);
    OVERRIDE_SRPC_METHOD_1(delegatePartyMaster,
        ObjectId, newMasterId);
    OVERRIDE_SRPC_METHOD_1(responsePublicParty,
        PartyId, partyId);
    OVERRIDE_SRPC_METHOD_1(changePartyType,
        PartyType, partyType);
    OVERRIDE_SRPC_METHOD_2(moveRaidPartyGroupPosition,
        ObjectId, playerId, PartyPosition, position); 
    OVERRIDE_SRPC_METHOD_2(switchRaidPartyGroupPosition,
        ObjectId, srcId, ObjectId, descId); 

    OVERRIDE_SRPC_METHOD_2(onInviteParty,
        ErrorCode, errorCode, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_1(onRespondPartyInvitation,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onLeaveParty,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onKickPartyMember,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onDelegatePartyMaster,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onResponsePublicParty,
        ErrorCode, errorCode, PartyId, partyId);
    OVERRIDE_SRPC_METHOD_2(onChangePartyType,
        ErrorCode, errorCode, PartyType, partyType);
    OVERRIDE_SRPC_METHOD_3(onMoveRaidPartyGroupPosition,
        ErrorCode, errorCode, ObjectId, playerId, PartyPosition, position); 
    OVERRIDE_SRPC_METHOD_3(onSwitchRaidPartyGroupPosition,
        ErrorCode, errorCode, ObjectId, srcId, ObjectId, descId); 

    OVERRIDE_SRPC_METHOD_1(evPartyInvited,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(evPartyInvitationReponded,
        Nickname, nickname, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_1(evPartyMeberAdded,
        PartyMemberInfo, memberInfo);
    OVERRIDE_SRPC_METHOD_2(evPartyMemberInfos,
        PartyMemberInfos, memberInfos, PartyType, partyType);
    OVERRIDE_SRPC_METHOD_1(evPartyMemberLeft,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evPartyMemberKicked,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evPartyMasterChanged,
        ObjectId, masterId);
    OVERRIDE_SRPC_METHOD_3(evPartyMemberMapMoved,
        ObjectId, memberId, MapCode, worldMapCode, MapCode, subMapCode);
    OVERRIDE_SRPC_METHOD_3(evPartyMemberSaid,
        ObjectId, playerId, Nickname, nickname, ChatMessage, message);
    OVERRIDE_SRPC_METHOD_2(evWaypointSet,
        ObjectId, playerId, Waypoint, waypoint);
    OVERRIDE_SRPC_METHOD_1(evPartyTypeChanged,
        PartyType, partyType);
    OVERRIDE_SRPC_METHOD_2(evRaidPartyGroupPositionMoved,
        ObjectId, playerId, PartyPosition, position); 
    OVERRIDE_SRPC_METHOD_2(evRaidPartyGroupPositionSwitched,
        ObjectId, srcId, ObjectId, descId); 
    OVERRIDE_SRPC_METHOD_1(evPartyGroupPositionInitialized,
        PartyPositionMap, positionMap);
public:
    // = WhisperRpc overriding
    OVERRIDE_SRPC_METHOD_2(whisper,
        Nickname, nickname, ChatMessage, message);

    OVERRIDE_SRPC_METHOD_3(onWhisper,
        ErrorCode, errorCode, Nickname, nickname, ChatMessage, message);

    OVERRIDE_SRPC_METHOD_3(evWhispered,
        ObjectId, playerId, Nickname, nickname, ChatMessage, message);

public:
    // = rpc::NoticeRpc overriding
    OVERRIDE_SRPC_METHOD_1(noticeToShard,
        ChatMessage, message);

    OVERRIDE_SRPC_METHOD_1(evShardNoticed,
        ChatMessage, message);

public:
    // = rpc::CommunityGuildRpc overriding
    OVERRIDE_SRPC_METHOD_2(createGuild,
        GuildName, guildName, GuildMarkCode, guildMarkCode);
    OVERRIDE_SRPC_METHOD_1(inviteGuild,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_0(leaveGuild);
    OVERRIDE_SRPC_METHOD_2(respondGuildInvitation,
        Nickname, nickname, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_1(sayGuildMember,
        ChatMessage, message);
    OVERRIDE_SRPC_METHOD_1(kickGuildMember, 
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(addGuildRelationship,
        GuildId, targetGuildId, GuildRelatioshipType, type);
    OVERRIDE_SRPC_METHOD_1(removeGuildRelationship,
        GuildId, targetGuildId);
    OVERRIDE_SRPC_METHOD_1(searchGuildInfo,
        GuildName, searchGuildName);
    OVERRIDE_SRPC_METHOD_1(signupGuild,
        GuildId, guildId);    
    OVERRIDE_SRPC_METHOD_0(queryGuildApplicants);
    OVERRIDE_SRPC_METHOD_2(confirmGuildApplicant,
        ObjectId, playerId, bool, isAccept);
    OVERRIDE_SRPC_METHOD_1(queryGuildMember,
        GuildId, guildId);
    OVERRIDE_SRPC_METHOD_0(queryMyGuildApplicantId);
    OVERRIDE_SRPC_METHOD_1(queryDetailSearchGuildInfo,
        GuildId, guildId);
    OVERRIDE_SRPC_METHOD_0(cancelGuildApplicant);
    OVERRIDE_SRPC_METHOD_1(modifyGuildIntroduction,
        GuildIntroduction, introduction);
    OVERRIDE_SRPC_METHOD_1(modifyGuildNotice,
        GuildNotice, notice);
    OVERRIDE_SRPC_METHOD_0(queryMyGuildInfo);
    OVERRIDE_SRPC_METHOD_0(queryMyGuildSkillInfo);
    OVERRIDE_SRPC_METHOD_0(queryMyGuildLevelInfo);

    OVERRIDE_SRPC_METHOD_2(queryGuildEventLogs, 
        uint8_t, beginLogIndex, uint8_t, logCount);
    OVERRIDE_SRPC_METHOD_1(addGuildRank, 
        GuildRankName, rankName);
    OVERRIDE_SRPC_METHOD_1(deleteGuildRank, 
        GuildRankId, rankId);
    OVERRIDE_SRPC_METHOD_2(swapGuildRank,
        GuildRankId, rankId1, GuildRankId, rankId2);
    OVERRIDE_SRPC_METHOD_2(setGuildRankName, 
        GuildRankId, rankId, GuildRankName, rankName);
    OVERRIDE_SRPC_METHOD_3(updateGuildRankRights, 
        GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_4(updateGuildBankRights,
        GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_2(updateGuildRank,
        ObjectId, memberId, GuildRankId, rankId);

    OVERRIDE_SRPC_METHOD_2(onInviteGuild,
        ErrorCode, errorCode, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_1(onLeaveGuild,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onCreateGuild,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onRespondGuildInvitation,
        ErrorCode, errorCode, GuildId, guildId);
    OVERRIDE_SRPC_METHOD_2(onKickGuildMember, 
        ErrorCode, errorCode, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(onAddGuildRelationship,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onRemoveGuildRelationship,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onSearchGuildInfo,
        ErrorCode, errorCode, SearchGuildInfos, searchGuildInfos);
    OVERRIDE_SRPC_METHOD_1(onSignupGuild,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onGuildApplicants,
        ErrorCode, errorCode, GuildApplicantInfoMap, guildApplicantInfoMap);
    OVERRIDE_SRPC_METHOD_2(onConfirmGuildApplicant,
        ErrorCode, errorCode, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(onGuildMemberInfos,
        ErrorCode, errorCode, GuildMemberInfoMap, guildMemberInfoMap);
    OVERRIDE_SRPC_METHOD_1(onMyGuildApplicantId,
        GuildId, guildId);
    OVERRIDE_SRPC_METHOD_2(onDetailSearchGuildInfo,
        ErrorCode, errorCode, DetailSearchGuildInfo, info);
    OVERRIDE_SRPC_METHOD_1(onCancelGuildApplicant,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onModifyGuildIntroduction,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onModifyGuildNotice,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_5(onGuildEventLogs, 
        ErrorCode, errorCode, uint8_t, beginLogIndex, uint8_t, logCount, 
        uint8_t, totalLogCount, GuildEventLogInfoList, bankLogList);
    OVERRIDE_SRPC_METHOD_2(onAddGuildRank, 
        ErrorCode, errorCode, GuildRankInfo, rankInfo);
    OVERRIDE_SRPC_METHOD_2(onDeleteGuildRank,
        ErrorCode, errorCode, GuildRankId, rankId);
    OVERRIDE_SRPC_METHOD_3(onSwapGuildRank, 
        ErrorCode, errorCode, GuildRankId, rankId1, GuildRankId, rankId2);
    OVERRIDE_SRPC_METHOD_3(onSetGuildRankName, 
        ErrorCode, errorCode, GuildRankId, rankId, GuildRankName, rankName);
    OVERRIDE_SRPC_METHOD_4(onUpdateGuildRankRights, 
        ErrorCode, errorCode, GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_5(onUpdateGuildBankRights, 
        ErrorCode, errorCode, GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_3(onUpdateGuildRank, 
        ErrorCode, errorCode, ObjectId, memberId, GuildRankId, rankId);

    OVERRIDE_SRPC_METHOD_7(evGuildInfo,
        BaseGuildInfo, guildInfo, GuildMemberInfoMap, guildMembers,
        GuildRelationshipInfoMap, relaytionMap,
        GuildIntroduction, introduction, GuildNotice, guildNotice,
        GuildRankInfos, rankInfos, GuildLevelInfo, guildLevelInfo);
    OVERRIDE_SRPC_METHOD_2(evGuildSkillInfos,
        GuildId, guildId, SkillCodes, guildSkills);
    OVERRIDE_SRPC_METHOD_2(evGuildLevelInfo,
        GuildId, guildId, GuildLevelInfo, guildLevelInfo);
    OVERRIDE_SRPC_METHOD_1(evGuildInvited,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(evGuildInvitationResponded,
        Nickname, nickname, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_3(evGuildMemberSaid,
        ObjectId, playerId, Nickname, nickname, ChatMessage, message);
    OVERRIDE_SRPC_METHOD_1(evGuildMemberJoined,
        GuildMemberInfo, guildMemberInfo);
    OVERRIDE_SRPC_METHOD_1(evGuildMemberLeft,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evGuildMemberKicked,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evGuildMemberOnline,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evGuildMemberOffline,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evGuildRelationshipAdded,
        GuildRelationshipInfo, relationshipInfo);
    OVERRIDE_SRPC_METHOD_1(evGuildRelationshipRemoved,
        GuildId, targetGuildId);
    OVERRIDE_SRPC_METHOD_2(evGuildMemberPositionChanged,
        ObjectId, playerId, GuildMemberPosition, position);
    OVERRIDE_SRPC_METHOD_1(evGuildIntroductionModified,
        GuildIntroduction, introduction);
    OVERRIDE_SRPC_METHOD_1(evGuildNoticeModified,
        GuildNotice, notice);
    OVERRIDE_SRPC_METHOD_2(evGuildSignUpResponded,
        GuildName, guildName, bool, isAccept);
    OVERRIDE_SRPC_METHOD_1(evGuildMasterChanged,
        ObjectId, masterId);
    OVERRIDE_SRPC_METHOD_1(evGuildRankAdded,
        GuildRankInfo, rankInfo);
    OVERRIDE_SRPC_METHOD_1(evGuildRankDeleted,
        GuildRankId, rankId);
    OVERRIDE_SRPC_METHOD_2(evGuildRankSwapped,
        GuildRankId, rankId1, GuildRankId, rankId2);
    OVERRIDE_SRPC_METHOD_2(evGuildRankNameUpdated,
        GuildRankId, rankId, GuildRankName, rankName);
    OVERRIDE_SRPC_METHOD_3(evGuildRankRightsUpdated,
        GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_4(evGuildBankRightsUpdated,
        GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay);
    OVERRIDE_SRPC_METHOD_2(evGuildMemberRankUpdated,
        ObjectId, memberId, GuildRankId, rankId);
    OVERRIDE_SRPC_METHOD_2(evGuildMemberLevelUpdated,
        ObjectId, memberId, CreatureLevel, level);
public:
    // = CommunityGuildInventoryRpc overriding
    OVERRIDE_SRPC_METHOD_0(queryGuildBankInfo);

    OVERRIDE_SRPC_METHOD_1(openGuildInventory,
        VaultId, vaultId);
    OVERRIDE_SRPC_METHOD_0(closeGuildInventory);
    OVERRIDE_SRPC_METHOD_3(moveGuildInventoryItem,
        VaultId, vaultId, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_3(switchGuildInventoryItem,
        VaultId, vaultId, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_2(queryGuildGameMoneyEventLogs, 
        uint8_t, beginLogIndex, uint8_t, logCount);
    OVERRIDE_SRPC_METHOD_2(queryGuildBankEventLogs,
        uint8_t, beginLogIndex, uint8_t, logCount);
    OVERRIDE_SRPC_METHOD_2(updateGuildVaultName,
        VaultId, vaultId, VaultName, name);

    OVERRIDE_SRPC_METHOD_3(onGuildBankInfo,
        ErrorCode, errorCode, BaseVaultInfos, infos, GameMoney, gameMoney);

    OVERRIDE_SRPC_METHOD_3(onOpenGuildInventory,
        ErrorCode, errorCode, VaultId, vaultId, InventoryInfo, inventoryInfo);
    OVERRIDE_SRPC_METHOD_1(onMoveGuildInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onSwitchGuildInventoryItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_5(onGuildGameMoneyEventLogs, 
        ErrorCode, errorCode, uint8_t, beginLogIndex, uint8_t, logCount, 
        uint8_t, totalLogCount, GuildGameMoneyEventLogInfoList, bankLogList);
    OVERRIDE_SRPC_METHOD_5(onGuildBankEventLogs, 
        ErrorCode, errorCode, uint8_t, beginLogIndex, uint8_t, logCount, 
        uint8_t, totalLogCount, GuildBankEventLogInfoList, bankLogList);

    OVERRIDE_SRPC_METHOD_1(onUpdateGuildVaultName,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_1(evGuildVaultAdded,
        BaseVaultInfo, vaultInfo);
    OVERRIDE_SRPC_METHOD_3(evGuildInventoryItemCountUpdated,
        VaultId, vaultId, ObjectId, itemId, uint8_t, ItemCount);
    OVERRIDE_SRPC_METHOD_2(evGuildInventoryItemAdded,
        VaultId, vaultId, ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_2(evGuildInventoryItemRemoved,
        VaultId, vaultId, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_3(evGuildInventoryItemMoved,
        VaultId, vaultId, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_3(evGuildInventoryItemSwitched,
        VaultId, vaultId, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_1(evGuildGameMoneyUpdated,
        GameMoney, currentGameMoney);
    OVERRIDE_SRPC_METHOD_2(evGuildVaultNameUpdated,
        VaultId, vaultId, VaultName, name);

public:
    // = CommunityDominionRpc overriding
    OVERRIDE_SRPC_METHOD_0(queryWorldDominion);
    OVERRIDE_SRPC_METHOD_1(queryZoneDominion,
        MapCode, worldMapCode);

    OVERRIDE_SRPC_METHOD_1(onWorldDominion,
        DominionWorldInfoMap, infoMap);
    OVERRIDE_SRPC_METHOD_1(onZoneDominion,
        BuildingDominionInfos, infos);

public:
    // = CommunityMessengerRpc overriding
    OVERRIDE_SRPC_METHOD_1(requestBuddy,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(repsonseBuddy,
        Nickname, requester, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_1(removeBuddy,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(addBlock,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_1(removeBlock,
        ObjectId, playerId);

    OVERRIDE_SRPC_METHOD_2(onRequestBuddy,
        ErrorCode, errorCode, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(onRepsonseBuddy,
        ErrorCode, errorCode, BuddyInfo, buddyInfo);
    OVERRIDE_SRPC_METHOD_2(onRemoveBuddy,
        ErrorCode, errorCode, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(onAddBlock,
        ErrorCode, errorCode, BlockInfo, blockInfo);
    OVERRIDE_SRPC_METHOD_2(onRemoveBlock,
        ErrorCode, errorCode, ObjectId, playerId);

    OVERRIDE_SRPC_METHOD_2(evBuddyInfos,
        BuddyInfos, buddyInfos, BlockInfos, blockInfos);
    OVERRIDE_SRPC_METHOD_1(evBuddyRequested,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_1(evBuddyAdded,
        BuddyInfo, buddy);
    OVERRIDE_SRPC_METHOD_2(evBuddyOnlineStateChanged,
        ObjectId, playerId, bool, isOnline);
    OVERRIDE_SRPC_METHOD_1(evBuddyRemoved,
        ObjectId, playerId);

private:
    void reset();

private:
    bool isValid() const {
        return sne::server::isValid(clientId_) && accountInfo_.isValid();
    }

    bool canRequest() const;

private:
    CommunityClientSessionCallback* sessionCallback_;
    CommunityUserHelper* userHelper_;

    sne::server::ServerId serverId_;
    PartyId partyId_;
    ZoneId zoneId_;
    MapCode worldMapCode_;
    MapCode subMapCode_;

    sne::server::ClientId clientId_;
    AccountInfo accountInfo_;

    PlayerInfo playerInfo_;
    WorldPosition worldPosition_;

    ChannelId worldMapChannelId_;
    
    BuddyInfos buddyInfos_;
    BlockInfos blockInfos_;
    bool isInitBuddyInfos_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace communityserver {
