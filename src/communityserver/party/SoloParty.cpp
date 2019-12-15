#include "CommunityServerPCH.h"
#include "SoloParty.h"
#include "../user/CommunityUser.h"
#include "../s2s/CommunityServerSideProxyManager.h"

namespace gideon { namespace communityserver {


SoloParty::SoloParty(PartyId partyId) :
    Party(partyId)
{
}


void SoloParty::initialize(const OnlineUserMap& onlineUserMap, const PartyMemberInfoMap& memberMap)
{
    onlineUserMap_ = onlineUserMap;
    memberMap_ = memberMap;

    notifyInitializePosition(group_);
}


ErrorCode SoloParty::addMember(const PartyMemberInfo& memberInfo, CommunityUser* user)
{
    if (! canAddMember()) {
        return ecPartyMaxOverUser;
    }

    const ErrorCode errorCode = Party::addMember(memberInfo, user); 
    if (isSucceeded(errorCode)) {
        group_.toggle(memberInfo.partyPosition_.position_);
    }
    return errorCode;
}


void SoloParty::removeMember(ObjectId memberId)
{
    PartyPosition position;
    const PartyMemberInfo* memberInfo = getMemberInfo(memberId);
    if (memberInfo) {
        group_.toggle(memberInfo->partyPosition_.position_);

        Party::removeMember(memberId);
    }
}



PartyPosition SoloParty::getJoinPartyPosition() const
{
    int8_t position = group_.getEmptyPosition();
    return PartyPosition(startPartyGroupIndex, position);
}


bool SoloParty::shouldDestroy() const
{
    return onlineUserMap_.size() <= 1;
}


bool SoloParty::canAddMember() const
{
    return maxPartyGroupMember > memberMap_.size();
}


ErrorCode SoloParty::canChangeParty(PartyType partyType) const
{
    if (! isRaidParty(partyType)) {
        return ecPartyInvalidPartyType;
    }
    return ecOk;
}


bool SoloParty::canInviteGrade(ObjectId playerId) const
{
    if (master_ && master_->getPlayerId() == playerId) {
        return true;
    }
    return false;
}

}} // namespace gideon { namespace communityserver {
