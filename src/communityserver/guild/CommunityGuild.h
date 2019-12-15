#pragma once

#include "inventory/GuildInventoryCallback.h"
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/PlayerInfo.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/CharacterClass.h>
#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/server/data/ServerGuildInfo.h>

namespace gideon {
class GuildSkills;
}

namespace gideon { namespace communityserver {

class CommunityUser;
class CommunityUserHelper;
class GuildInventory;

/***
 * class CommunityGuild
 ***/
class CommunityGuild : public GuildInventoryCallback
{
    typedef sne::core::Map<ObjectId, CommunityUser*> OnlineUserMap;
    typedef std::mutex LockType;
public:
    CommunityGuild();
    ~CommunityGuild();
    

    void initialize(const BaseGuildInfo& baseGuildInfo, CommunityUser& user);
    void initialize(const GuildInfo& baseGuildInfo);

    void handleTimeout();

    ErrorCode join(ObjectId acceptorId, CommunityUser& user);
    ErrorCode leave(ObjectId memberId);
    ErrorCode kick(ObjectId kickerId, ObjectId kickeeId);
	ErrorCode dropout(ObjectId characterId) {
        return leave(characterId);
    }

    ErrorCode addRelationship(ObjectId memberId, const GuildRelationshipInfo& guildRelationship);
    ErrorCode removeRelationship(ObjectId memberId, GuildId guildId);

    void addGuildApplicant(ObjectId playerId, const Nickname& nickname, CharacterClass cc,
        MapCode mapCode);
    ErrorCode removeGuildApplicant(ObjectId playerId);
    
    ErrorCode confirmGuildApplicant(CommunityUserHelper& userHelper, ObjectId memberId, ObjectId playerId, bool isAccept);
    ErrorCode modifyGuildIntroduction(ObjectId memberId, const GuildIntroduction& introduction);
    ErrorCode modifyGuildNotice(ObjectId memberId, const GuildNotice& notice);
    
    void say(const PlayerInfo& playerInfo, const ChatMessage& message);

    void online(CommunityUser* user);
    void offline(ObjectId playerId);
    void updateMemberLevel(ObjectId playerId, CreatureLevel level);

    void queryGuildInfo(CommunityUser& user);
    void querySkillInfo(CommunityUser& user);
    void queryLevelInfo(CommunityUser& user);

    ErrorCode addGuildRank(GuildRankInfo& rankInfo, ObjectId memberId, const GuildRankName& rankName);
    ErrorCode deleteGuildRank(ObjectId memberId, GuildRankId rankId);
    ErrorCode swapGuildRank(ObjectId memberId, GuildRankId rankId1, GuildRankId rankId2);
    ErrorCode setGuildRankName(ObjectId memberId, GuildRankId rankId, const GuildRankName& rankName);
    ErrorCode updateGuildRankRights(ObjectId memberId, GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay);
    ErrorCode updateGuildBankRights(ObjectId memberId, GuildRankId rankId,
        VaultId id, uint32_t rights, uint32_t itemWithdrawalPerDay);
    ErrorCode updateGuildMemberRank(ObjectId memberId, ObjectId targetMemberId, GuildRankId rankId);
    
public:
    ErrorCode purchaseVault(const BaseVaultInfo& vaultInfo, ObjectId memberId);
    
    ErrorCode activateGuildSkill(SkillCode skillCode, ObjectId memberId);
    ErrorCode deactivateGuildSkills(ObjectId memberId);

    void addExp(GuildExp exp);
    void addCheatExp(GuildExp exp);

    ErrorCode openGuildInventory(VaultId vaultId, InventoryInfo& inventoryInfo, CommunityUser* user);
    void closeGuildInventory(ObjectId playerId);
    ErrorCode pushGuildInventoryItem(ObjectId playerId, const ItemInfo& itemInfo, VaultId vaultId, SlotId slotId);
    ErrorCode popGuildInventoryItem(ItemInfo& guildItemInfo, ObjectId playerId, VaultId vaultId, ObjectId itemId);
    
    // 플래이어 <-> 길드
    ErrorCode switchGuildInventoryItem(ItemInfo& guildItemInfo, ObjectId playerId, VaultId vaultId, 
        ObjectId guildItemId, const ItemInfo& playerItemInfo);

    ErrorCode moveGuildInventoryItem(ObjectId playerId, VaultId vaultId, ObjectId itemId, SlotId slotId);
    ErrorCode switchGuildInventoryItem(ObjectId playerId, VaultId vaultId, ObjectId itemId1, ObjectId itemId2);

    /// 필드에서 얻은 이벤트
    void upGuildGameMoney(GameMoney gameMoney);

    ErrorCode deposit(ObjectId playerId, GameMoney gameMoney);
    ErrorCode withdraw(sec_t& resetDayWithdrawTime, GameMoney& dayWithdraw, ObjectId playerId, GameMoney gameMoney);
    ErrorCode updateVaultName(ObjectId playerId, VaultId vaultId, const VaultName& name);

    ErrorCode fillBankInfo(BaseVaultInfos& baseVaultInfos, GameMoney& gameMoney, ObjectId playerId);
    ErrorCode fillEventLogInfo(GuildEventLogInfoList& infoList, uint8_t& totalLogCount, 
        uint8_t beginLogIndex, uint8_t logCount);
    ErrorCode fillGameMoneyEventLogInfo(GuildGameMoneyEventLogInfoList& infoList, uint8_t& totalLogCount, 
        uint8_t beginLogIndex, uint8_t logCount);
    ErrorCode fillBankEventLogInfo(GuildBankEventLogInfoList& infoList, uint8_t& totalLogCount, 
        uint8_t beginLogIndex, uint8_t logCount);

public:
    bool shouldDestory() const;

public:
    GuildId getGuildId() const {
        return baseGuildInfo_.guildId_;
    }

    BaseGuildInfo getBaseGuildInfo() const;
    GuildMemberInfoMap getGuildMembers() const;
    GuildRelationshipInfoMap getGuildRelationships() const;
    GuildApplicantInfoMap getGuildApplicantInfoMap() const;
    DetailSearchGuildInfo getDetailSearchGuildInfo() const;
    Nickname getGuildMasterName() const;
    
    const GuildIntroduction& getGuildIntroduction() const {
        return introduction_;
    }

    const GuildNotice& getGuildNotice() const {
        return notice_;
    }

    GuildInventory& getGuildInventory() {
        return *guildInventory_.get();
    }

    GameMoney getGameMoney() const {
        return gameMoney_;
    }

    const GuildLevelInfo& getGuildLevelInfo() const {
        return guildLevelInfo_;
    }


private:
    void addMember(const GuildMemberInfo& guildMember, CommunityUser* user, CommunityUser& acceptor);

    void removeInventoryViewer(ObjectId playerId);
	void changeMaster();

    void updateAddibleDayExpInfo();

    void verifyVaultRights(VaultId vaultId);

    void logGuildEvent(GuildEventLogType logType,
        ObjectId playerId1 = invalidObjectId, const Nickname& nickname1 = L"",
        ObjectId playerId2 = invalidObjectId, const Nickname& nickname2 = L"",
        uint64_t param1 = 0);
    void logGuildBankEvent(GuildEventLogType logType, VaultId vaultId = invalidVaultId,
        ObjectId playerId = invalidObjectId,
        const Nickname& nickname = L"", uint32_t param1 = 0, uint32_t param2 = 0);
    void logGuildGameMoneyEvent(GuildEventLogType logType = geltUnknown,
        ObjectId playerId = invalidObjectId, const Nickname& nickname = L"",
        GameMoney gameMoney = gameMoneyMin);

private:
    bool isMember(ObjectId playerId) const {
        return getGuildMemberInfo(playerId) != nullptr;
    }
    
    CommunityUser* getOnlineMember(ObjectId playerId);

    const GuildMemberInfo* getGuildMemberInfo(ObjectId playerId) const;
    GuildMemberInfo* getGuildMemberInfo(ObjectId playerId);

    const CommunityUser* getInventoryViewer(ObjectId playerId) const;
    CommunityUser* getInventoryViewer(ObjectId playerId);

    GuildRankId getLowestRankId() const;

    GuildMemberInfo* getGuildMasterCandidate();

    bool hasRelationship(GuildId guildId) const {
        return relationshipMap_.find(guildId) != relationshipMap_.end();
    }

    GuildApplicantInfo* getApplicantInfo(ObjectId playerId) {
        const GuildApplicantInfoMap::iterator pos = guildApplicantInfoMap_.find(playerId);
        if (pos != guildApplicantInfoMap_.end()) {
            return &(*pos).second;
        }
        return nullptr;
    }

    ErrorCode checkActivateSkill(GuildSkillPoint& needSkillPoint, SkillCode skillCode, ObjectId memberId) const;

private:
    // = GuildInventoryCallback overriding
    virtual void vaultAdded(const BaseVaultInfo& info);
    virtual void vaultNameUpdated(VaultId vaultId, const VaultName& name);

    virtual void inventoryItemCountUpdated(VaultId vaultId, ObjectId itemId, uint8_t count);
    virtual void inventoryItemAdded(VaultId vaultId, const ItemInfo& itemInfo);

    virtual void inventoryItemRemoved(VaultId vaultId, ObjectId itemId);
    virtual void inventoryItemSwitched(VaultId vaultId, ObjectId itemId1, SlotId slotId1,
        ObjectId itemId2, SlotId slotId2);
    virtual void inventoryItemMoved(VaultId vaultId, ObjectId itemId, SlotId slotId);

private:
    mutable LockType lock_;
    const GuildExp maxDayAddExp_;

    BaseGuildInfo baseGuildInfo_;
    GuildMemberInfoMap guildMemberInfoMap_;
    GuildRelationshipInfoMap relationshipMap_;
    GuildApplicantInfoMap guildApplicantInfoMap_;
    GuildRankInfos rankInfos_;
    OnlineUserMap inventoryViewerMap_;
    OnlineUserMap onlineUserMap_;
    GuildNotice notice_;
    GuildIntroduction introduction_;
    GameMoney gameMoney_;
    GuildEventLogManager eventLogManager_;
    GuildBankEventLogManager eventBankLogManager_;
    GuildGameMoneyEventLogManager guildGameMoneyEventLogManager_;
    
    GuildLevelInfo guildLevelInfo_;
    GuildSkillManager guildSkillManager_;
    GuildLevel guildLevel_;
    
    std::unique_ptr<GuildInventory> guildInventory_;

    bool isChangeLevelInfo_;
};

typedef std::shared_ptr<CommunityGuild> CommunityGuildPtr;

}} // namespace gideon { namespace communityserver {
