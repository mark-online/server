#include "CommunityServerPCH.h"
#include "CommunityGuild.h"
#include "inventory/GuildInventory.h"
#include "../user/CommunityUser.h"
#include "../user/CommunityUserHelper.h"
#include "../s2s/CommunityServerSideProxyManager.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/cs/datatable/GuildLevelTable.h>
#include <gideon/cs/datatable/GuildSkillTable.h>
#include <gideon/cs/datatable/PropertyTable.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace communityserver {

namespace {

GuildSkillPoint getNeedSkillPoint(SkillCode /*skillCode*/)
{
    return 1;
}

}

#pragma warning(disable: 4355)

CommunityGuild::CommunityGuild() :
    gameMoney_(0),
    maxDayAddExp_(GIDEON_PROPERTY_TABLE->getPropertyValue<GuildExp>(L"guild_day_max_add_exp")),
    guildInventory_(std::make_unique<GuildInventory>(*this)),
    isChangeLevelInfo_(false)
{
}


CommunityGuild::~CommunityGuild()
{
}


void CommunityGuild::initialize(const BaseGuildInfo& baseGuildInfo, CommunityUser& user)
{
    const GuildMemberInfo guildMember(user.getPlayerId(), user.getNickname(), gmpGuildMaster,
        true, gdrGuildMaster);

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncCreateGuild(baseGuildInfo);
    }

    {
        std::lock_guard<LockType> lock(lock_);

        baseGuildInfo_ = baseGuildInfo;
        gameMoney_ = 0;
        fillDefaultGuildRanks(rankInfos_);
        addMember(guildMember, &user, user);
        guildLevel_ = glStart;
        logGuildEvent(geltCreateGuild, guildMember.playerId_, guildMember.playerNickname_);
    }
    COMMUNITYSERVERSIDEPROXY_MANAGER->guildCreated(guildMember, baseGuildInfo);
}


void CommunityGuild::initialize(const GuildInfo& guildInfo)
{
    std::lock_guard<LockType> lock(lock_);

    baseGuildInfo_ = guildInfo;
    
    for (const GuildMemberInfo& guildMember : guildInfo.guildMemberInfos_) {
        guildMemberInfoMap_.emplace(guildMember.playerId_, guildMember);
    }
    for (const GuildRelationshipInfo& relationship : guildInfo.relytionships_) {
        relationshipMap_.emplace(relationship.targetGuildId_, relationship);
    }
    rankInfos_ = guildInfo.rankInfos_;
    guildApplicantInfoMap_ = guildInfo.guildApplicantInfoMap_;
    gameMoney_ = guildInfo.gameMoney_;
    eventLogManager_ = guildInfo.eventLogManager_;
    
    eventBankLogManager_ = guildInfo.eventBankLogManager_;
    guildGameMoneyEventLogManager_ = guildInfo.guildGameMoneyEventLogManager_;
    
    guildLevelInfo_ = guildInfo.levelInfo_;
    guildSkillManager_.initialize(guildInfo.guildSkillCodes_);
    guildLevel_ = GUILD_LEVEL_TABLE->getGuildLevel(guildInfo.levelInfo_.guildExp_);
    introduction_ = guildInfo.introduction_;
    notice_ = guildInfo.notice_;
    guildInventory_->setInventoryInfo(guildInfo.inventoryInfo_);
    for (const VaultInfo& vaultInfo : guildInfo.inventoryInfo_.vaultInfos_) {
        verifyVaultRights(vaultInfo.id_);
    }

    updateAddibleDayExpInfo();
}


void CommunityGuild::handleTimeout()
{
    bool shouldSave = false;
    bool canNowAddibleDayExp = true;
    bool shouldNotifyAddibleExp = false;

    GuildLevelInfo guildLevelInfo;

    {
        std::lock_guard<LockType> lock(lock_);

        if (isChangeLevelInfo_) {
            shouldSave = true;
            isChangeLevelInfo_ = false;
            guildLevelInfo = guildLevelInfo_;
        }

        const bool canPreAddibleDayExp = guildLevelInfo_.canAddExp(maxDayAddExp_);        
        updateAddibleDayExpInfo();
        canNowAddibleDayExp = guildLevelInfo_.canAddExp(maxDayAddExp_);
        shouldNotifyAddibleExp = canPreAddibleDayExp != canNowAddibleDayExp;

    }

    if (shouldNotifyAddibleExp) {
        COMMUNITYSERVERSIDEPROXY_MANAGER->guildAddibleDayExpState(baseGuildInfo_.guildId_, canNowAddibleDayExp);
    }

    if (shouldSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildExpInfo(baseGuildInfo_.guildId_, guildLevelInfo);
    }  
}


ErrorCode CommunityGuild::join(ObjectId acceptorId, CommunityUser& user)
{
    CommunityUser* acceptor = nullptr;
    {
        std::lock_guard<LockType> lock(lock_);

        acceptor = getOnlineMember(acceptorId);
        if (! acceptor) {
            return ecGuildIsNotMember;
        }
    }

    const GuildMemberInfo guildMember(user.getPlayerId(), user.getNickname(),
        gmpNone, true, getLowestRankId());

    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(acceptorId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrInvite, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        uint32_t maxCount = GUILD_LEVEL_TABLE->getGuildMemberCount(guildLevel_);
        if (maxCount <= guildMemberInfoMap_.size()) {
            return ecGuildMaxOverUser;
        }

        addMember(guildMember, &user, *acceptor);
    }

    return ecOk;
}


ErrorCode CommunityGuild::leave(ObjectId memberId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }

        const Nickname nickname = memberInfo->playerNickname_;
        const bool isGuildMaster = memberInfo->isGuildMaster();

        guildMemberInfoMap_.erase(memberId);
        onlineUserMap_.erase(memberId);

        if (isGuildMaster) {
            changeMaster();
        }

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildMemberLeft(memberId);                
        } 

        logGuildEvent(geltLeaveMember, memberId, nickname);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveGuildMember(memberId);
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->guildMemberRemoved(baseGuildInfo_.guildId_, memberId);
    return ecOk;
}


ErrorCode CommunityGuild::kick(ObjectId kickerId, ObjectId kickeeId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* kickeeMemberInfo = getGuildMemberInfo(kickeeId);
        if (! kickeeMemberInfo) {
            return ecGuildIsNotMember;
        }

        const GuildMemberInfo* kickerMemberInfo = getGuildMemberInfo(kickerId);
        if (! kickerMemberInfo) {
            return ecGuildIsNotMember;
        }
        if (! kickerMemberInfo->hasRight(grrKick, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildMemberKicked(kickeeId);
        } 

        const Nickname kickeeNickname = kickeeMemberInfo->playerNickname_;
        guildMemberInfoMap_.erase(kickeeId);
        onlineUserMap_.erase(kickeeId);

        logGuildEvent(geltKickMember, kickerId, kickerMemberInfo->playerNickname_,
            kickeeId, kickeeNickname);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveGuildMember(kickeeId);
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->guildMemberRemoved(baseGuildInfo_.guildId_, kickeeId);

    return ecOk;
}


ErrorCode CommunityGuild::addRelationship(ObjectId memberId, const GuildRelationshipInfo& guildRelationship)
{
    {
        std::lock_guard<LockType> lock(lock_);

        if (guildRelationship.targetGuildId_ == baseGuildInfo_.guildId_) {
            return ecGuildSelfGuildNotAddRelationship;
        }

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrSetDiplomacy, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        if (hasRelationship(guildRelationship.targetGuildId_)) {
            return ecGuildAlrealyHasRelationship;
        }
        relationshipMap_.insert(
            GuildRelationshipInfoMap::value_type(guildRelationship.targetGuildId_, guildRelationship));

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildRelationshipAdded(guildRelationship);
            }
        }
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddGuildRelationship(baseGuildInfo_.guildId_, guildRelationship.targetGuildId_, 
            guildRelationship.relationship_);
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->guildRelationshipAdded(baseGuildInfo_.guildId_, guildRelationship);

    // TODO: 로그 추가

    return ecOk;
}


ErrorCode CommunityGuild::removeRelationship(ObjectId memberId, GuildId guildId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrSetDiplomacy, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        if (! hasRelationship(guildId)) {
            return ecGuildNotHasRelationship;
        }
        relationshipMap_.erase(guildId);

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildRelationshipRemoved(guildId);
            }
        }
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveGuildRelationship(baseGuildInfo_.guildId_, guildId);
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->guildRelationshipRemoved(baseGuildInfo_.guildId_, guildId);

    // TODO: 로그 추가

    return ecOk;
}


void CommunityGuild::addGuildApplicant(ObjectId playerId, const Nickname& nickname,
    CharacterClass cc, MapCode mapCode)
{
    {
        std::lock_guard<LockType> lock(lock_);

        GuildApplicantInfo info(nickname, cc, mapCode, true);
        guildApplicantInfoMap_.emplace(playerId, info);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddGuildApplicant(playerId, baseGuildInfo_.guildId_);
    }

    // TODO: 관리자 채널로 알림 메세지??
}


ErrorCode CommunityGuild::confirmGuildApplicant(CommunityUserHelper& userHelper,
    ObjectId memberId, ObjectId playerId, bool isAccept)
{
    CommunityUser* acceptor = getOnlineMember(memberId);
    if (! acceptor) {
        return ecGuildIsNotMember;
    }

    {
        std::lock_guard<LockType> lock(lock_);

        // TODO: 인원 검사
        //uint32_t userCount = static_cast<uint32_t>(guildMemberInfoMap_.size());

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrInvite, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        GuildApplicantInfo* applicantInfo = getApplicantInfo(playerId);
        if (! applicantInfo) {
            return ecGuildNotApplicant;
        }

        CommunityUser* user = userHelper.getUser(applicantInfo->nickname_);
        if (isAccept) {     
            const GuildMemberInfo guildMember(playerId, applicantInfo->nickname_,
                gmpNone, true, getLowestRankId());

            addMember(guildMember, user, *acceptor);
        }

        if (user) {
            user->guildSignUpResponded(baseGuildInfo_.guildName_, isAccept);
        }
    
        guildApplicantInfoMap_.erase(playerId);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveGuildApplicant(playerId);
    }
    return ecOk;
}


ErrorCode CommunityGuild::removeGuildApplicant(ObjectId playerId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        if (! getApplicantInfo(playerId)) {
            return ecGuildNotApplicant;
        }
        guildApplicantInfoMap_.erase(playerId);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveGuildApplicant(playerId);
    }
    return ecOk;
}


ErrorCode CommunityGuild::modifyGuildIntroduction(ObjectId memberId, const GuildIntroduction& introduction)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrEditIntroduction, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        introduction_ = introduction;
        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildIntroductionModified(introduction);            
        }
    }
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncModifyGuildIntroduction(baseGuildInfo_.guildId_, introduction);
    }

    return ecOk;
}


ErrorCode CommunityGuild::modifyGuildNotice(ObjectId memberId, const GuildNotice& notice)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrEditMotd, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        notice_ = notice;
        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildNoticeModified(notice);            
        }
    }
    
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncModifyGuildNotice(baseGuildInfo_.guildId_, notice);
    }
    return ecOk;
}


void CommunityGuild::say(const PlayerInfo& playerInfo, const ChatMessage& message)
{
    std::lock_guard<LockType> lock(lock_);

    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildMemberSaid(playerInfo, message);
    }
}


void CommunityGuild::online(CommunityUser* user)
{
    ObjectId playerId = user->getPlayerId();

    std::lock_guard<LockType> lock(lock_);

    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        if (memberUser->getPlayerId() != playerId) {
            memberUser->guildMemberOnline(playerId);
        }
    }

    GuildMemberInfo* memberInfo = getGuildMemberInfo(playerId);
    if (memberInfo != nullptr) {
        memberInfo->isOnline_ = true;
    }

    onlineUserMap_.emplace(playerId, user);
}


void CommunityGuild::offline(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        if (memberUser->getPlayerId() != playerId) {
            memberUser->guildMemberOffline(playerId);
        }
    }

    GuildMemberInfo* memberInfo = getGuildMemberInfo(playerId);
    if (memberInfo != nullptr) {
        memberInfo->isOnline_ = false;
    }
    onlineUserMap_.erase(playerId);
}


void CommunityGuild::updateMemberLevel(ObjectId playerId, CreatureLevel level)
{
    std::lock_guard<LockType> lock(lock_);

    GuildMemberInfo* memberInfo = getGuildMemberInfo(playerId);
    if (memberInfo == nullptr) {
        return;    
    }
    memberInfo->level_ = level;
    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildMemberLevelUpdated(playerId, level);
    }
}


void CommunityGuild::queryGuildInfo(CommunityUser& user)
{
    std::lock_guard<LockType> lock(lock_);

    user.evGuildInfo(baseGuildInfo_, guildMemberInfoMap_, relationshipMap_,
        introduction_, notice_, rankInfos_, guildLevelInfo_);
}


void CommunityGuild::querySkillInfo(CommunityUser& user)
{
    std::lock_guard<LockType> lock(lock_);

    user.evGuildSkillInfos(baseGuildInfo_.guildId_, guildSkillManager_.getGuildSkills());
}


void CommunityGuild::queryLevelInfo(CommunityUser& user)
{
    std::lock_guard<LockType> lock(lock_);

    user.evGuildLevelInfo(baseGuildInfo_.guildId_, guildLevelInfo_);
}


ErrorCode CommunityGuild::addGuildRank(GuildRankInfo& rankInfo, ObjectId memberId,
    const GuildRankName& rankName)
{
    uint8_t vaultCount = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        if (rankName.empty()) {
            return ecGuild_RankNameIsEmpty;
        }

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrEditRanks, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        const GuildRankId newRankId = getLowestRankId() + 1;
        if (newRankId >= maxGuildRankCount) {
            return ecGuild_TooManyRank;
        }

        rankInfo = GuildRankInfo::forGreen();
        rankInfo.rankId_ = newRankId;
        rankInfo.name_ = rankName;
        rankInfos_.push_back(rankInfo);

        vaultCount = static_cast<uint8_t>(guildInventory_->getVaultCount());
        if (0 < vaultCount) {
            for (int i = 0; i < vaultCount; ++i) {
                rankInfo.vaultRights_.push_back(GuildBankVaultRightInfo());
            }
        }

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildRankAdded(rankInfo);                
            }
        }
    }

    // TODO: 로그 기록???

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddGuildRank(baseGuildInfo_.guildId_, rankInfo, vaultCount);        
    }

    return ecOk;
}


ErrorCode CommunityGuild::deleteGuildRank(ObjectId memberId, GuildRankId rankId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrEditRanks, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        if (rankId == gdrGuildMaster) {
            return ecGuild_InvalidRankId; // TODO: 길드장 등급은 삭제할 수 없다
        }

        if ((rankId <= invalidGuildRankId) || (rankId > getLowestRankId())) {
            return ecGuild_InvalidRankId;
        }

        deleteGuildRankInfo(rankInfos_, rankId);

        promoteGuildRank(guildMemberInfoMap_, rankId);

        // TODO: 클라이언트 - 이벤트를 받은 길드원은 해당 rankid 보다 낮은 등급으로 설정된 것을 모두 수정해야 한다
        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildRankDeleted(rankId);
            }
        } 
    }

    // TODO: 로그 기록???

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncDeleteGuildRank(baseGuildInfo_.guildId_, rankId);
    }

    return ecOk;
}


ErrorCode CommunityGuild::swapGuildRank(ObjectId memberId, GuildRankId rankId1, GuildRankId rankId2)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrEditRanks, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        if ((rankId1 == gdrGuildMaster) || (rankId2 == gdrGuildMaster)) {
            return ecGuild_InvalidRankId; // TODO: 길드장 등급은 삭제할 수 없다
        }

        if (! swapGuildRankInfo(rankInfos_, rankId1, rankId2)) {
            return ecGuild_InvalidRankId;
        }

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildRankSwapped(rankId1, rankId2);
            }
        } 
    }

    // TODO: 로그 기록???

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncSwapGuildRank(baseGuildInfo_.guildId_, rankId1, rankId2);
    }

    return ecOk;
}


ErrorCode CommunityGuild::setGuildRankName(ObjectId memberId,
    GuildRankId rankId, const GuildRankName& rankName)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrEditRanks, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        GuildRankInfo* rankInfo = getGuildRankInfo(rankInfos_, rankId);
        if (! rankInfo) {
            return ecGuild_InvalidRankId;
        }

        rankInfo->name_ = rankName;

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildRankNameUpdated(rankId, rankName);
            }
        } 
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildRankName(baseGuildInfo_.guildId_, rankId, rankName);
    }

    return ecOk;
}


ErrorCode CommunityGuild::updateGuildRankRights(ObjectId memberId, GuildRankId rankId,
    uint32_t rights, uint32_t goldWithdrawalPerDay)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(grrEditRanks, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        GuildRankInfo* rankInfo = getGuildRankInfo(rankInfos_, rankId);
        if (! rankInfo) {
            return ecGuild_InvalidRankId;
        }

        rankInfo->rights_ = static_cast<GuildRankRight>(rights);
        rankInfo->goldWithdrawalPerDay_ = goldWithdrawalPerDay;

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildRankRightsUpdated(rankId, rights, goldWithdrawalPerDay);
            }
        } 
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildRankRights(baseGuildInfo_.guildId_, rankId, rights, goldWithdrawalPerDay);
    }

    return ecOk;
}


ErrorCode CommunityGuild::updateGuildBankRights(ObjectId memberId, GuildRankId rankId,
    VaultId id, uint32_t rights, uint32_t itemWithdrawalPerDay)
{
    {
        if (! isValidVaultId(id)) {
            return ecInventoryInvalidVault;
        }

        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }

        if (! memberInfo->hasRight(grrEditRanks, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        GuildRankInfo* rankInfo = getGuildRankInfo(rankInfos_, rankId);
        if (! rankInfo) {
            return ecGuild_InvalidRankId;
        }

        if (id >= rankInfo->vaultRights_.size()) {
            return ecInventoryInvalidVault;
        }

        rankInfo->vaultRights_[id].rights_ = static_cast<GuildBankRights>(rights); 
        rankInfo->vaultRights_[id].itemWithdrawalPerDay_ = itemWithdrawalPerDay;

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildBankRightsUpdated(rankId, id, rights, itemWithdrawalPerDay);
            }
        } 
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildBankRights(baseGuildInfo_.guildId_, rankId, id, rights, itemWithdrawalPerDay);
    }

    return ecOk;
}


ErrorCode CommunityGuild::updateGuildMemberRank(ObjectId memberId, ObjectId targetMemberId, GuildRankId rankId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        GuildMemberInfo* targetMemberInfo = getGuildMemberInfo(targetMemberId);
        if (! targetMemberInfo) {
            return ecGuildIsNotMember;
        }

        if (rankId == targetMemberInfo->rankId_) {
            return ecOk;
        }

        const bool isPromote = (rankId < targetMemberInfo->rankId_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        if (! memberInfo->hasRight(isPromote ? grrPromote : grrDemote, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        if (targetMemberInfo->isGuildMaster()) {
            return ecGuild_InvalidRankId; // TODO: 길드장의 등급은 변경할 수 없습니다
        }

        if (rankId == gdrGuildMaster) {
            return ecGuild_InvalidRankId; // TODO: 길드장 등급으로 변경할 수 없습니다
        }

        if ((rankId <= invalidGuildRankId) || (rankId > getLowestRankId())) {
            return ecGuild_InvalidRankId;
        }

        targetMemberInfo->rankId_ = rankId;

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildMemberRankUpdated(targetMemberId, rankId);
            }
        } 
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildRank(baseGuildInfo_.guildId_, targetMemberId, rankId);
    }

    return ecOk;
}


ErrorCode CommunityGuild::purchaseVault(const BaseVaultInfo& vaultInfo, ObjectId memberId)
{
    Nickname nickname;
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        nickname = memberInfo->playerNickname_;
        if (! memberInfo->hasRight(grrVaultPurchase, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }

        const GameMoney vaultCost = getGuildVaultCost(vaultInfo.id_);
        if (gameMoney_ < vaultCost) {
            return ecGuild_NotEnoughMoney;
        }

        const ErrorCode errorCode = guildInventory_->addVault(vaultInfo);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        gameMoney_ -= vaultCost;

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            if (memberUser->getPlayerId() != memberId) {
                memberUser->guildVaultAdded(vaultInfo);
            }
            memberUser->guildGameMoneyUpdated(gameMoney_);
        }
    }

    logGuildGameMoneyEvent(geltPurchaseVault, memberId, nickname, getGuildVaultCost(vaultInfo.id_));

    return ecOk;
}


ErrorCode CommunityGuild::activateGuildSkill(SkillCode skillCode, ObjectId memberId)
{
    SkillCodes removeSkillCodes;
    Nickname nickname;
    {
        std::lock_guard<LockType> lock(lock_);

        GuildSkillPoint needSkillPoint = 0;
        const ErrorCode errorCode = checkActivateSkill(needSkillPoint, skillCode, memberId);
        if (isFailed(errorCode)) {
            return errorCode;
        }
        const GuildMemberInfo* guildMemberInfo = getGuildMemberInfo(memberId);
        if (guildMemberInfo) {
            nickname = guildMemberInfo->playerNickname_;
        }

        isChangeLevelInfo_ = true;
        guildLevelInfo_.skillPoint_ -= needSkillPoint;

        SkillCode removeSkillCode =guildSkillManager_.getCurrentSkillCode(getSkillTableType(skillCode), getSkillIndex(skillCode));
        if (isValidSkillCode(removeSkillCode)) {
            removeSkillCodes.push_back(removeSkillCode);
            guildSkillManager_.removeSkill(removeSkillCode);
        }            
        guildSkillManager_.updateSkill(skillCode);        
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        if (! removeSkillCodes.empty()) {
            db->asyncRemoveGuildSkills(baseGuildInfo_.guildId_, removeSkillCodes);
        }
        db->asyncAddGuildSkill(baseGuildInfo_.guildId_, skillCode);
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->guildSkillActivated(ecOk, baseGuildInfo_.guildId_, memberId, skillCode);

    logGuildEvent(geltActivateSkill, memberId, nickname, invalidObjectId, L"", skillCode);
     
    return ecOk;    
}


ErrorCode CommunityGuild::deactivateGuildSkills(ObjectId memberId)
{
    Nickname nickname;
    GameMoney gameMoney = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        nickname = memberInfo->playerNickname_;
        
        if (! memberInfo->hasRight(grrActivateSkill, rankInfos_)) {
            return ecGuildNotEnoughMemberGrade;
        }
        
        if (gameMoney_ < deActivateSkillFee) {
            return ecBankWithdrawNotEnoughGameMoney;
        }

        gameMoney_ -= deActivateSkillFee;
        gameMoney = gameMoney_;
        isChangeLevelInfo_ = true;
        guildSkillManager_.finalize();
        guildLevelInfo_.skillPoint_ = guildLevel_ - 1;
        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildGameMoneyUpdated(gameMoney_);
        }
    }
    COMMUNITYSERVERSIDEPROXY_MANAGER->guildSkillsDeactivated(ecOk, baseGuildInfo_.guildId_, memberId);    
    logGuildEvent(geltDeactivateSkills, memberId, nickname, invalidObjectId, L"", 0);
    logGuildGameMoneyEvent(geltDeactivateSkills, memberId, nickname, deActivateSkillFee);
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveAllGuildSkills(baseGuildInfo_.guildId_);
        db->asyncUpdateGuildGameMoney(baseGuildInfo_.guildId_, gameMoney);        
    }
    return ecOk;    
}


void CommunityGuild::addExp(GuildExp exp)
{
    bool isChangeLevel = false;
    bool canNowAddibleDayExp = true;
    bool shouldNotifyAddibleExp = false;
    {
        std::lock_guard<LockType> lock(lock_);

        if (isMaxGuildLevel(guildLevel_)) {
            return;
        }

        const bool canPreAddibleDayExp = guildLevelInfo_.canAddExp(maxDayAddExp_);
        updateAddibleDayExpInfo();
        canNowAddibleDayExp = guildLevelInfo_.canAddExp(maxDayAddExp_);
        shouldNotifyAddibleExp = canPreAddibleDayExp != canNowAddibleDayExp;
        if (guildLevelInfo_.addExp(maxDayAddExp_, exp)) {
            isChangeLevelInfo_ = true;
            guildLevelInfo_.lastUpdateExpTime_ = getTime();
            GuildLevel newLevel = GUILD_LEVEL_TABLE->getGuildLevel(guildLevelInfo_.guildExp_);
            if (isMaxGuildLevel(newLevel)) {
                const GuildExp guildLevelExp = GUILD_LEVEL_TABLE->getGuildLevelExp(newLevel);
                if (guildLevelExp < guildLevelInfo_.guildExp_) {
                    guildLevelInfo_.guildExp_ = guildLevelExp;
                }
            }

            if (newLevel != guildLevel_) {
                // 현재 레벨 1당 1스킬 포인트
                GuildSkillPoint addSkillPoint = newLevel - guildLevel_;
                guildLevelInfo_.skillPoint_ += addSkillPoint;
                guildLevel_ = newLevel;
                isChangeLevel = true;
            }
        }

    }

    if (isChangeLevel) {
        COMMUNITYSERVERSIDEPROXY_MANAGER->guildLevelUpdated(baseGuildInfo_.guildId_, guildLevel_);
        logGuildEvent(geltLevelUp, invalidObjectId, L"", invalidObjectId, L"", guildLevel_);
    }
    /*    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
    CommunityUser* memberUser = value.second;
    memberUser->guildExpUpdated(guildLevelInfo_.guildExp_);
    }

    if (isChangeLevel) {
    for (const OnlineUserMap::value_type& value = onlineUserMap_) {
    CommunityUser* memberUser = value.second;
    memberUser->guildLevelUpdated(guildLevel_, guildLevelInfo_.skillPoint_);
    }
    }*/
}


void CommunityGuild::addCheatExp(GuildExp exp)
{
    bool isChangeLevel = false;
    {    
        std::lock_guard<LockType> lock(lock_);

        if (isMaxGuildLevel(guildLevel_)) {
            return;
        }
        
        guildLevelInfo_.guildExp_ += exp;
        GuildLevel newLevel = GUILD_LEVEL_TABLE->getGuildLevel(guildLevelInfo_.guildExp_);
        if (isMaxGuildLevel(newLevel)) {
            const GuildExp guildLevelExp = GUILD_LEVEL_TABLE->getGuildLevelExp(newLevel);
            if (guildLevelExp < guildLevelInfo_.guildExp_) {
                guildLevelInfo_.guildExp_ = guildLevelExp;
            }
        }

        if (newLevel != guildLevel_) {
            isChangeLevelInfo_ = true;
            GuildSkillPoint addSkillPoint = newLevel - guildLevel_;
            guildLevelInfo_.skillPoint_ += addSkillPoint;
            guildLevel_ = newLevel;
            isChangeLevel = true;
        }
    }

    if (isChangeLevel) {
        COMMUNITYSERVERSIDEPROXY_MANAGER->guildLevelUpdated(baseGuildInfo_.guildId_, guildLevel_);
    }
}


ErrorCode CommunityGuild::openGuildInventory(VaultId vaultId, InventoryInfo& inventoryInfo, CommunityUser* user)
{
    std::lock_guard<LockType> lock(lock_);

    // TODO: 권한 검사

    const GuildMemberInfo* guildMemberInfo = getGuildMemberInfo(user->getPlayerId());
    if (! guildMemberInfo) {
        return ecGuildIsNotMember;
    }

    //if (! guildMemberInfo->hasRight(vaultId, gbrViewVault, rankInfos_)) {
    //    return ecGuildNotEnoughMemberGrade;
    //}
  
    if (inventoryViewerMap_.find(user->getPlayerId()) == inventoryViewerMap_.end()) {
        inventoryViewerMap_.emplace(user->getPlayerId(), user);
    }
    guildInventory_->fillInventoryInfo(inventoryInfo, vaultId);
    return ecOk;
}


void CommunityGuild::closeGuildInventory(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    removeInventoryViewer(playerId);
}


ErrorCode CommunityGuild::pushGuildInventoryItem(ObjectId playerId, const ItemInfo& itemInfo, VaultId vaultId, SlotId slotId)
{
    std::lock_guard<LockType> lock(lock_);

    // TODO: 권한 검사

    const CommunityUser* viewer = getInventoryViewer(playerId);
    if (! viewer) {
        return ecGuildNotEnoughMemberGrade;
    }

    const GuildMemberInfo* guildMemberInfo = getGuildMemberInfo(playerId);
    if (! guildMemberInfo) {
        return ecGuildIsNotMember;
    }

    if (! guildMemberInfo->hasRight(vaultId, gbrItemDeposit, rankInfos_)) {
        return ecGuildNotEnoughMemberGrade;
    }

    const ErrorCode errorCode = guildInventory_->pushItem(itemInfo, vaultId, slotId);
    if (isSucceeded(errorCode)) {
        logGuildBankEvent(geltPushInvenItem, vaultId, playerId, viewer->getNickname(),
            itemInfo.itemCode_, itemInfo.count_);
    }

    return errorCode;
}


ErrorCode CommunityGuild::popGuildInventoryItem(ItemInfo& guildItemInfo,
    ObjectId playerId, VaultId vaultId, ObjectId itemId)
{
    std::lock_guard<LockType> lock(lock_);

    // TODO: 권한 검사

    const CommunityUser* viewer = getInventoryViewer(playerId);
    if (! viewer) {
        return ecGuildNotEnoughMemberGrade;
    }

    const GuildMemberInfo* guildMemberInfo = getGuildMemberInfo(playerId);
    if (! guildMemberInfo) {
        return ecGuildIsNotMember;
    }

    if (! guildMemberInfo->hasRight(vaultId, gbrItemWithdrawal, rankInfos_)) {
        return ecGuildNotEnoughMemberGrade;
    }

    guildItemInfo = guildInventory_->getItemInfo(vaultId, itemId);
    if (! guildItemInfo.isValid()) {
        return ecGuildNotFindInventoryItem;
    }

    const ErrorCode errorCode = guildInventory_->popItem(vaultId, itemId); 
    if (isSucceeded(errorCode)) {
        logGuildBankEvent(geltPopInvenItem, vaultId, playerId, viewer->getNickname(),
            guildItemInfo.itemCode_, guildItemInfo.count_);
    }

    return errorCode;
}


ErrorCode CommunityGuild::switchGuildInventoryItem(ItemInfo& guildItemInfo,
    ObjectId playerId, VaultId vaultId, ObjectId guildItemId, const ItemInfo& playerItemInfo)
{
    std::lock_guard<LockType> lock(lock_);

    const CommunityUser* viewer = getInventoryViewer(playerId);
    if (! viewer) {
        return ecGuildNotEnoughMemberGrade;
    }

    const GuildMemberInfo* guildMemberInfo = getGuildMemberInfo(playerId);
    if (! guildMemberInfo) {
        return ecGuildIsNotMember;
    }

    if (! guildMemberInfo->hasRight(vaultId, gbrItemWithdrawal, rankInfos_)) {
        return ecGuildNotEnoughMemberGrade;
    }

    guildItemInfo = guildInventory_->getItemInfo(vaultId, guildItemId);
    if (! guildItemInfo.isValid()) {
        return ecGuildNotFindInventoryItem;
    }

    (void)guildInventory_->popItem(vaultId, guildItemId);

    const ErrorCode errorCode = guildInventory_->pushItem(playerItemInfo, vaultId, guildItemInfo.slotId_);
    if (isSucceeded(errorCode)) {
        logGuildBankEvent(geltPopInvenItem, vaultId, playerId, viewer->getNickname(),
            guildItemInfo.itemCode_, guildItemInfo.count_);
        logGuildBankEvent(geltPushInvenItem, vaultId, playerId, viewer->getNickname(),
            playerItemInfo.itemCode_, playerItemInfo.count_);
    }
    return errorCode;

}


ErrorCode CommunityGuild::moveGuildInventoryItem(ObjectId playerId, VaultId vaultId, ObjectId itemId,
    SlotId slotId)
{
    std::lock_guard<LockType> lock(lock_);

    const CommunityUser* viewer = getInventoryViewer(playerId);
    if (! viewer) {
        return ecGuildNotEnoughMemberGrade;
    }

    const GuildMemberInfo* guildMemberInfo = getGuildMemberInfo(playerId);
    if (! guildMemberInfo) {
        return ecGuildIsNotMember;
    }

    if (! guildMemberInfo->hasRight(vaultId, gbrItemWithdrawal, rankInfos_)) {
        return ecGuildNotEnoughMemberGrade;
    }

    return guildInventory_->moveItem(vaultId, itemId, slotId);
}


ErrorCode CommunityGuild::switchGuildInventoryItem(ObjectId playerId, VaultId vaultId, ObjectId itemId1, ObjectId itemId2)
{
    std::lock_guard<LockType> lock(lock_);

    const GuildMemberInfo* guildMemberInfo = getGuildMemberInfo(playerId);
    if (! guildMemberInfo) {
        return ecGuildIsNotMember;
    }
    if (! guildMemberInfo->hasRight(vaultId, gbrItemWithdrawal, rankInfos_)) {
        return ecGuildNotEnoughMemberGrade;
    }
    const CommunityUser* viewer = getInventoryViewer(playerId);
    if (! viewer) {
        return ecGuildNotEnoughMemberGrade;
    }
    return guildInventory_->switchItem(vaultId, itemId1, itemId2);
}


void CommunityGuild::upGuildGameMoney(GameMoney gameMoney)
{
    {
        std::lock_guard<LockType> lock(lock_);

        gameMoney_ += gameMoney;

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildGameMoneyUpdated(gameMoney_);
        }
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildGameMoney(baseGuildInfo_.guildId_, gameMoney_);
    }
}


ErrorCode CommunityGuild::deposit(ObjectId playerId, GameMoney gameMoney)
{
    {
        std::lock_guard<LockType> lock(lock_);

        const CommunityUser* member = getOnlineMember(playerId);
        if (! member) {
            return ecGuildNotEnoughMemberGrade;
        }

        gameMoney_ += gameMoney;
        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildGameMoneyUpdated(gameMoney_);
        }

        logGuildGameMoneyEvent(geltDeposit, playerId, member->getNickname(), gameMoney);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildGameMoney(baseGuildInfo_.guildId_, gameMoney_);
    }

    return ecOk;
}


ErrorCode CommunityGuild::withdraw(sec_t& resetDayWithdrawTime, GameMoney& dayWithdraw, ObjectId playerId, GameMoney gameMoney)
{
    {
        std::lock_guard<LockType> lock(lock_);

        if (gameMoney_ < gameMoney) {
            return ecBankWithdrawNotEnoughGameMoney;
        }

        GuildMemberInfo* memberInfo = getGuildMemberInfo(playerId);
        if (! memberInfo) {
            return ecGuildIsNotMember;
        }
        const GuildRankInfo* rankInfo = getGuildRankInfo(rankInfos_, memberInfo->rankId_);
        if ((! rankInfo) || (! rankInfo->hasRight(grrMoneyWithdrawal))) {
            return ecGuildNotEnoughMemberGrade;
        }

        if (rankInfo->goldWithdrawalPerDay_ != unlimitedGoldWithdrawlPerDay) {
            // TODO: 정리를 해야 할 듯 --kcando
            const sec_t now = getTime();
            resetDayWithdrawTime = memberInfo->resetDayWithdrawTime_;
            if (memberInfo->resetDayWithdrawTime_  < now) {
                memberInfo->resetDayWithdrawTime_ = now + getDaySec();
                memberInfo->dayWithdraw_ = 0;
            }

            if (byGold(rankInfo->goldWithdrawalPerDay_) < memberInfo->dayWithdraw_ + gameMoney) {
                return ecBankOverTodayWithdraw;
            }

            memberInfo->dayWithdraw_ += gameMoney;
            dayWithdraw = memberInfo->dayWithdraw_;
            resetDayWithdrawTime = memberInfo->resetDayWithdrawTime_;
        }

        gameMoney_ -= gameMoney;

        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            CommunityUser* memberUser = value.second;
            memberUser->guildGameMoneyUpdated(gameMoney_);
        }

        logGuildGameMoneyEvent(geltWidthdraw, playerId, memberInfo->playerNickname_, gameMoney);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildGameMoney(baseGuildInfo_.guildId_, gameMoney_);
        db->asyncUpdateGuildMemberWithdraw(playerId, dayWithdraw, resetDayWithdrawTime);
    }

    return ecOk;
}


ErrorCode CommunityGuild::updateVaultName(ObjectId playerId, VaultId vaultId, const VaultName& name)
{
    std::lock_guard<LockType> lock(lock_);

    const GuildMemberInfo* memberInfo = getGuildMemberInfo(playerId);
    if (! memberInfo) {
        return ecGuildIsNotMember;
    }
    if (! memberInfo->hasRight(grrEditVaultName, rankInfos_)) {
        return ecGuildNotEnoughMemberGrade;
    }

    return guildInventory_->updateVaultName(vaultId, name);
}


ErrorCode CommunityGuild::fillBankInfo(BaseVaultInfos& baseVaultInfos, GameMoney& gameMoney, ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    if (! isMember(playerId)) {
        return ecGuildIsNotMember;
    }

    const VaultInfos& vaultInfos = guildInventory_->getVaults();
    baseVaultInfos.reserve(vaultInfos.size());
    baseVaultInfos.assign(vaultInfos.begin(), vaultInfos.end());

    gameMoney = gameMoney_;
    return ecOk;
}


ErrorCode CommunityGuild::fillEventLogInfo(GuildEventLogInfoList& infoList, uint8_t& totalLogCount,
    uint8_t beginLogIndex, uint8_t logCount)
{
    std::lock_guard<LockType> lock(lock_);

    eventLogManager_.fillPageLogList(infoList, totalLogCount, beginLogIndex, logCount);

    return ecOk;
}


ErrorCode CommunityGuild::fillGameMoneyEventLogInfo(GuildGameMoneyEventLogInfoList& infoList, uint8_t& totalLogCount, 
    uint8_t beginLogIndex, uint8_t logCount)
{
    std::lock_guard<LockType> lock(lock_);

    guildGameMoneyEventLogManager_.fillPageLogList(infoList, totalLogCount, beginLogIndex, logCount);

    return ecOk;
}


ErrorCode CommunityGuild::fillBankEventLogInfo(GuildBankEventLogInfoList& infoList, uint8_t& totalLogCount,
    uint8_t beginLogIndex, uint8_t logCount)
{
    std::lock_guard<LockType> lock(lock_);

    eventBankLogManager_.fillPageLogList(infoList, totalLogCount, beginLogIndex, logCount);

    return ecOk;
}


Nickname CommunityGuild::getGuildMasterName() const
{
    std::lock_guard<LockType> lock(lock_);

    for (const GuildMemberInfoMap::value_type& value : guildMemberInfoMap_) {
        const GuildMemberInfo& memberUser = value.second;
        if (memberUser.isGuildMaster()) {
            return memberUser.playerNickname_;
        }
    }
    return Nickname();
}


bool CommunityGuild::shouldDestory() const
{
    std::lock_guard<LockType> lock(lock_);

    return guildMemberInfoMap_.empty();
}


BaseGuildInfo CommunityGuild::getBaseGuildInfo() const
{
    std::lock_guard<LockType> lock(lock_);

    return baseGuildInfo_;
}


GuildMemberInfoMap CommunityGuild::getGuildMembers() const
{
    std::lock_guard<LockType> lock(lock_);

    return guildMemberInfoMap_;
}


GuildRelationshipInfoMap CommunityGuild::getGuildRelationships() const
{
    std::lock_guard<LockType> lock(lock_);

    return relationshipMap_;
}


GuildApplicantInfoMap CommunityGuild::getGuildApplicantInfoMap() const
{
    std::lock_guard<LockType> lock(lock_);

    return guildApplicantInfoMap_;
}


DetailSearchGuildInfo CommunityGuild::getDetailSearchGuildInfo() const
{
    std::lock_guard<LockType> lock(lock_);

    DetailSearchGuildInfo info(baseGuildInfo_.guildId_, baseGuildInfo_.guildName_,
        baseGuildInfo_.guildMarkCode_, getGuildMasterName(), 0, introduction_);
    return info;
}


void CommunityGuild::addMember(const GuildMemberInfo& guildMember, CommunityUser* user,
    CommunityUser& acceptor)
{
    const ObjectId playerId = guildMember.playerId_;
    const GuildId guildId = baseGuildInfo_.guildId_;

    guildMemberInfoMap_.emplace(playerId, guildMember);

    if (user != nullptr) {
        onlineUserMap_.emplace(playerId, user);
        user->guildJoined(baseGuildInfo_, guildMemberInfoMap_, relationshipMap_,
            introduction_, notice_, rankInfos_, guildLevelInfo_);
    }

    if (! guildMember.isGuildMaster()) {
        for (const OnlineUserMap::value_type& value : onlineUserMap_) {
            const ObjectId memberId = value.first;
            if (memberId != playerId) {
                CommunityUser* memberUser = value.second;
                memberUser->guildMemberJoined(guildMember);
            }
        }
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        const AccountId accountId = (user != nullptr ) ? user->getAccountId() : invalidAccountId;
        db->asyncAddGuildMember(accountId, guildId, guildMember);
    }

    if (! guildMember.isGuildMaster()) {
        COMMUNITYSERVERSIDEPROXY_MANAGER->guildMemberAdded(guildId, guildMember);

        logGuildEvent(geltJoinMember, acceptor.getPlayerId(), acceptor.getNickname(),
            playerId, guildMember.playerNickname_);
    }
}


void CommunityGuild::removeInventoryViewer(ObjectId playerId)
{
    inventoryViewerMap_.erase(playerId);
}


void CommunityGuild::changeMaster()
{
    if (guildMemberInfoMap_.empty()) {
        return;
    }

    GuildMemberInfo* memberInfo = getGuildMasterCandidate();
    if (! memberInfo) {
        assert(false);
        guildMemberInfoMap_.clear(); // 길드 파괴
        return;
    }

    memberInfo->position_ = gmpGuildMaster;
    memberInfo->rankId_ = gdrGuildMaster;

    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildMasterChanged(memberInfo->playerId_);
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncChangeGuildMemberPosition(memberInfo->playerId_, gmpGuildMaster);
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->guildMemberPositionChanged(baseGuildInfo_.guildId_,
        memberInfo->playerId_, gmpGuildMaster);

    // TODO: 로그 기록
}


void CommunityGuild::updateAddibleDayExpInfo()
{
    if (isMaxGuildLevel(guildLevel_)) {
        return;
    }

    const sec_t now = getTime();
    const sec_t nowDaySec = (now / getDaySec()) * getDaySec();
    const sec_t lastGuildDaySec = (guildLevelInfo_.lastUpdateExpTime_ / getDaySec()) * getDaySec();

    if (nowDaySec != lastGuildDaySec) {
        guildLevelInfo_.dayGuildAddExp_ = 0;
        guildLevelInfo_.lastUpdateExpTime_ = now;
    }
}


void CommunityGuild::verifyVaultRights(VaultId vaultId)
{
    const GuildBankVaultRightInfo defaultVaultRightInfo;

    DatabaseGuard db(SNE_DATABASE_MANAGER);

    for (GuildRankInfo& rankInfo : rankInfos_) {
        if (rankInfo.hasVaultRight(vaultId)) {
            continue;
        }
        if (rankInfo.isMasterRank()) {
            rankInfo.vaultRights_.push_back(GuildBankVaultRightInfo::forGuildMaster());
        }
        else {
            rankInfo.vaultRights_.push_back(defaultVaultRightInfo);
        }
        db->asyncAddGuildBankRights(baseGuildInfo_.guildId_, rankInfo.rankId_, vaultId,
            rankInfo.vaultRights_.back());
    }
}


void CommunityGuild::logGuildEvent(GuildEventLogType logType, ObjectId playerId1,
    const Nickname& nickname1, ObjectId playerId2, const Nickname& nickname2,
    uint64_t param1)
{
    const GuildEventLogInfo logInfo(eventLogManager_.getNextLogId(), logType, playerId1,
        playerId2, nickname1, nickname2, param1, getTime());
    eventLogManager_.addLog(logInfo);

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddGuildEventLog(baseGuildInfo_.guildId_, logInfo);
    }
}


void CommunityGuild::logGuildBankEvent(GuildEventLogType logType, VaultId vaultId,
    ObjectId playerId, const Nickname& nickname, uint32_t param1, uint32_t param2)
{
    const GuildBankEventLogInfo logInfo(eventBankLogManager_.getNextLogId(), vaultId, logType,
        playerId, nickname, param1, param2, getTime());
    eventBankLogManager_.addLog(logInfo);

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddGuildBankEventLog(baseGuildInfo_.guildId_, logInfo);
    }
}


void CommunityGuild::logGuildGameMoneyEvent(GuildEventLogType logType, ObjectId playerId,
    const Nickname& nickname, GameMoney gameMoney)
{
    const GuildGameMoneyEventLogInfo logInfo(guildGameMoneyEventLogManager_.getNextLogId(),
        logType, playerId, nickname, gameMoney, getTime());
    guildGameMoneyEventLogManager_.addLog(logInfo);

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddGuildGameMoneyEventLog(baseGuildInfo_.guildId_, logInfo);
    }
}


CommunityUser* CommunityGuild::getOnlineMember(ObjectId playerId)
{
    const OnlineUserMap::iterator pos = onlineUserMap_.find(playerId);
    if (pos != onlineUserMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


const GuildMemberInfo* CommunityGuild::getGuildMemberInfo(ObjectId playerId) const
{
    GuildMemberInfoMap::const_iterator pos = guildMemberInfoMap_.find(playerId);
    if (pos != guildMemberInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


GuildMemberInfo* CommunityGuild::getGuildMemberInfo(ObjectId playerId)
{
    GuildMemberInfoMap::iterator pos = guildMemberInfoMap_.find(playerId);
    if (pos != guildMemberInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


const CommunityUser* CommunityGuild::getInventoryViewer(ObjectId playerId) const
{
    OnlineUserMap::const_iterator pos = inventoryViewerMap_.find(playerId);
    if (pos != inventoryViewerMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


CommunityUser* CommunityGuild::getInventoryViewer(ObjectId playerId)
{
    OnlineUserMap::iterator pos = inventoryViewerMap_.find(playerId);
    if (pos != inventoryViewerMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


GuildRankId CommunityGuild::getLowestRankId() const
{
    const size_t rankSize = rankInfos_.size();
    return (rankSize > 0) ? static_cast<GuildRankId>(rankInfos_.size() - 1) : gdrGreen;
}


GuildMemberInfo* CommunityGuild::getGuildMasterCandidate()
{
    if (guildMemberInfoMap_.empty()) {
        return nullptr;
    }

    GuildMemberInfo* candidateInfo = nullptr;
    GuildMemberPosition highestPosition = gmpNone;
    for (GuildMemberInfoMap::value_type& value : guildMemberInfoMap_) {
        GuildMemberInfo& memberInfo = value.second;
        if (memberInfo.position_ < highestPosition) {
            candidateInfo = &memberInfo;
            highestPosition = memberInfo.position_;
        }
    }

    if (! candidateInfo) {
        // TODO: 참여한 순서로 위임해야 하나? --kcando
        candidateInfo = &(*guildMemberInfoMap_.begin()).second;
    }
    return candidateInfo;
}


ErrorCode CommunityGuild::checkActivateSkill(GuildSkillPoint& needSkillPoint, SkillCode skillCode, ObjectId memberId) const
{
    needSkillPoint = toSkillPoint(0);

    if (! isValidSkillCode(skillCode)) {
        return ecSkillInvalidSkill;
    }

    const gdt::guild_skill_t* guildSkill = GUILD_SKILL_TABLE->getGuildSkill(skillCode);
    if (! guildSkill) {
        return ecGuildInvalidSkill;
    }
    if (guildLevel_ < toGuildLevel(guildSkill->need_level())) {
        return ecGuildNotEnoughLevel;
    }

    const GuildMemberInfo* memberInfo = getGuildMemberInfo(memberId);
    if (! memberInfo) {
        return ecGuildIsNotMember;
    }

    if (! memberInfo->hasRight(grrActivateSkill, rankInfos_)) {
        return ecGuildNotEnoughMemberGrade;
    }

    const SkillTableType learnSkillTableType = getSkillTableType(skillCode);
    const SkillIndex learnSkillIndex = getSkillIndex(skillCode);
    const SkillLevel learnSkillLevel = getSkillLevel(skillCode);

    const SkillCode currentSkillCode = guildSkillManager_.getCurrentSkillCode(learnSkillTableType, learnSkillIndex);
    const SkillLevel currentSkillLevel = getSkillLevel(currentSkillCode);
    if (learnSkillLevel != (currentSkillLevel + 1)) {
        return ecSkillMaxSkillNotLearn;
    }

    needSkillPoint = guildSkill->consume_sp(); 
    if (guildLevelInfo_.skillPoint_ < needSkillPoint) {
        return ecGuildNotEnoughSkillPoint;
    }

    return ecOk;
}

// = GuildInventoryCallback overriding

void CommunityGuild::vaultAdded(const BaseVaultInfo& info)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);

        db->asyncAddGuildVault(baseGuildInfo_.guildId_, info);
    }

    verifyVaultRights(info.id_);
}


void CommunityGuild::vaultNameUpdated(VaultId vaultId, const VaultName& name)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildVaultName(baseGuildInfo_.guildId_, vaultId, name);
    }

    for (const OnlineUserMap::value_type& value : inventoryViewerMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildVaultNameUpdated(vaultId, name);
    }
}


void CommunityGuild::inventoryItemCountUpdated(VaultId vaultId, ObjectId itemId, uint8_t count)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateGuildInventoryItemCount(baseGuildInfo_.guildId_, itemId, count);
    }

    for (const OnlineUserMap::value_type& value : inventoryViewerMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildInventoryItemCountUpdated(vaultId, itemId, count);
    }
}


void CommunityGuild::inventoryItemAdded(VaultId vaultId, const ItemInfo& itemInfo)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddGuildInventoryItem(baseGuildInfo_.guildId_, vaultId, itemInfo);
    }

    for (const OnlineUserMap::value_type& value : inventoryViewerMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildInventoryItemAdded(vaultId, itemInfo);
    }
}


void CommunityGuild::inventoryItemRemoved(VaultId vaultId, ObjectId itemId)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveGuildInventoryItem(baseGuildInfo_.guildId_, itemId); 
    }

    for (const OnlineUserMap::value_type& value : inventoryViewerMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildInventoryItemRemoved(vaultId, itemId);
    }
}


void CommunityGuild::inventoryItemSwitched(VaultId vaultId, ObjectId itemId1, SlotId slotId1,
    ObjectId itemId2, SlotId slotId2)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncSwitchGuildInventoryItem(baseGuildInfo_.guildId_, vaultId, itemId1, slotId1, itemId2, slotId2); 
    }
    
    for (const OnlineUserMap::value_type& value : inventoryViewerMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildInventoryItemSwitched(vaultId, itemId1, itemId2);
    }
}


void CommunityGuild::inventoryItemMoved(VaultId vaultId, ObjectId itemId, SlotId slotId)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncMoveGuildInventoryItem(baseGuildInfo_.guildId_, vaultId, itemId, slotId);
    }
    
    for (const OnlineUserMap::value_type& value : inventoryViewerMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->guildInventoryItemMoved(vaultId, itemId, slotId);
    }
}

}} // namespace gideon { namespace communityserver {
