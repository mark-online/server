#pragma once

#include "../communityserver_export.h"
#include <gideon/cs/shared/rpc/CommunityRpc.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/PlayerInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/ChannelInfo.h>
#include <gideon/cs/shared/data/LevelInfo.h>
#include <sne/server/session/ClientId.h>
#include <sne/server/s2s/ServerId.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace communityserver {

class CommunityClientSessionCallback;
class CommunityUserHelper;

/**
 * @class CommunityUser
 *
 * Community 내의 사용자 클래스.
 * - 사용자 요청/응답 진입점.
 */
class CommunityServer_Export CommunityUser :
    public rpc::CommunityRpc
{
public:
    virtual ~CommunityUser() {}

    virtual void initialize(const AccountInfo& accountInfo,
        CommunityClientSessionCallback& sessionCallback, 
        CommunityUserHelper& userHelper) = 0;
    virtual void finalize() = 0;

    virtual void initPlayerInfo(sne::server::ServerId serverId, const PlayerInfo& playerInfo) = 0;

    virtual void characterClassChanged(ObjectId characterId, CharacterClass cc) = 0;
    virtual void updateCharacterLevel(ObjectId characterId, CreatureLevel level) = 0;

    virtual void notifyPartyInfo(PartyId partyId) = 0;

public:
    /// 강제 퇴장 당했다
    virtual void expelledFromServer(ExpelReason expelReason) = 0;

public:
    /// 채널에 입장하였다
    virtual void channelEntered(ZoneId zoneId, ChannelId channelId, MapCode worldMapCode,
        ObjectId accountId, ObjectId playerId, const Position& position) = 0;

    /// 월드맵에서 퇴장하였다
    virtual void channelLeft() = 0;

public:
    /// 플레이어가 월드맵에 입장하였는가?
    virtual bool isOnline() const = 0;

    virtual bool isBlocked(ObjectId playerId) const = 0;

public:
    virtual sne::server::ServerId getServerId() const = 0;
    
    virtual MapCode getWorldMapCode() const = 0;

    virtual MapCode getSubMapCode() const = 0;

    virtual ZoneId getZoneId() const = 0;

    virtual PartyId getPartyId() const = 0;

    virtual sne::server::ClientId getClientId() const = 0;

    virtual AccountId getAccountId() const = 0;

    virtual ObjectId getPlayerId() const = 0;

    virtual CharacterClass getCharacterClass() const = 0;

    virtual const Nickname getNickname() const = 0;

    virtual CreatureLevel getLevel() const = 0;

    virtual ChannelId getWorldMapChannelId() const = 0;

public:
    bool isValid() const {
        return isValidAccountId(getAccountId());
    }

public:
    // = 파티
    virtual bool isPartyMember() const = 0;
    virtual bool canPartyInviteable() const = 0;

    virtual void partyInvited(const Nickname& nickname) = 0;
    virtual ErrorCode partyInviteSucceesed(PartyId& partyId, CommunityUser* user) = 0;
    virtual void partyInviteFailed(const Nickname& nickname) = 0;

    virtual void partyMemberAdded(const PartyMemberInfo& memberInfo) = 0;
    virtual void partyJoined(const PartyMemberInfos& memberInfos, PartyType partyType) = 0;
    virtual void partyMemberLeft(ObjectId playerId, bool isDestroy) = 0;
    virtual void partyMemberKicked(ObjectId playerId) = 0;
    virtual void partyMasterChanged(ObjectId masterId) = 0;
    virtual void partyMemberMapMoved(ObjectId memberId, MapCode worldMapCode, MapCode subMapCode) = 0;
    virtual void partyMemberSaid(const PlayerInfo& playerInfo, const ChatMessage& message) = 0;
    virtual void partyTypeChanged(PartyType partyType) = 0;
    virtual void partyGroupPositionMoved(ObjectId memberId, const PartyPosition& position) = 0;
    virtual void partyGroupPositionSwitched(ObjectId srcId, ObjectId descId) = 0;
    virtual void partyGroupPositionInitialized(const PartyPositionMap& positionMap) = 0;

    virtual void guildMasterChanged(ObjectId masterId) = 0;

    virtual void guildInvited(const Nickname& nickname) = 0;
    virtual void guildMemberJoined(const GuildMemberInfo& guildMemberInfo) = 0;
    virtual void guildMemberLeft(ObjectId playerId) = 0;
    virtual void guildMemberKicked(ObjectId playerId) = 0;
    virtual ErrorCode guildInvitationAccepted(CommunityUser& user) = 0;
    virtual void guildInvitationRejected(const Nickname& nickname) = 0;
    virtual void guildMemberSaid(const PlayerInfo& playerInfo, const ChatMessage& message) = 0;
    virtual void guildMemberOnline(ObjectId playerId) = 0;
    virtual void guildMemberOffline(ObjectId playerId) = 0;
    virtual GuildId getGuildId() const = 0;

    virtual void guildRelationshipAdded(const GuildRelationshipInfo& guildRelationship) = 0;
    virtual void guildRelationshipRemoved(GuildId targetGuildId) = 0;
	virtual void guildMemberPositionChanged(ObjectId playerId, GuildMemberPosition position) = 0;
    virtual void guildJoined(const BaseGuildInfo& guildInfo, 
        const GuildMemberInfoMap& guildMembers,
        const GuildRelationshipInfoMap& relaytionMap,
        const GuildIntroduction& introduction, const GuildNotice& guildNotice,
        const GuildRankInfos& rankInfos, const GuildLevelInfo& guildLevelInfo) = 0;
    virtual void guildIntroductionModified(const GuildIntroduction& introduction) = 0;
    virtual void guildNoticeModified(const GuildNotice& guildNotice) = 0;
    virtual void guildSignUpResponded(const GuildName& guildName, bool isAccept) = 0;

    virtual void guildRankAdded(const GuildRankInfo& rankInfo) = 0;
    virtual void guildRankDeleted(GuildRankId rankId) = 0;
    virtual void guildRankSwapped(GuildRankId rankId1, GuildRankId rankId2) = 0;
    virtual void guildRankNameUpdated(GuildRankId rankId, const GuildRankName& rankName) = 0;
    virtual void guildRankRightsUpdated(GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay) = 0;
    virtual void guildBankRightsUpdated(GuildRankId rankId, VaultId id, uint32_t rights, uint32_t itemWithdrawalPerDay) = 0;
    virtual void guildMemberRankUpdated(ObjectId memberId, GuildRankId rankId) = 0;
    virtual void guildMemberLevelUpdated(ObjectId memberId, CreatureLevel level) = 0;

    virtual void guildVaultAdded(const BaseVaultInfo& vaultInfo) = 0;

    virtual void guildInventoryItemCountUpdated(VaultId vaultId, ObjectId itemId, uint8_t count) = 0;
    virtual void guildInventoryItemAdded(VaultId vaultId, const ItemInfo& itemInfo) = 0;
    virtual void guildInventoryItemRemoved(VaultId vaultId, ObjectId itemId) = 0;
    virtual void guildInventoryItemSwitched(VaultId vaultId, ObjectId itemId1, ObjectId itemId2) = 0;
    virtual void guildInventoryItemMoved(VaultId vaultId, ObjectId itemId, SlotId slotId) = 0;
    virtual void guildGameMoneyUpdated(GameMoney gameMoney) = 0;
    virtual void guildVaultNameUpdated(VaultId vaultId, const VaultName& name) = 0;

public:
    virtual void whispered(ObjectId playerId, const Nickname& nickname, const ChatMessage& message) = 0;

public:
	virtual void recall(const Nickname& callerName, const Nickname& calleeName,
		const WorldPosition& worldPosition) = 0;

public:
	virtual void buddyRequested(const Nickname& nickname) = 0;
	virtual void buddyAdded(const BuddyInfo& buddyInfo) = 0;
	virtual void buddyRemoved(ObjectId playerId) = 0;
	virtual void buddyOnlineStateChanged(ObjectId playerId, bool isOnlineState) = 0;
	virtual void initBuddyInfo(const BuddyInfos& buddyInfos, const BlockInfos& blockInfos) = 0;
	
};

}} // namespace gideon { namespace communityserver {
