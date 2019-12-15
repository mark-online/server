#include "CommunityServerPCH.h"
#include "GroupParty.h"
#include "../user/CommunityUser.h"

namespace gideon { namespace communityserver {

GroupParty::GroupParty(PartyId partyId, PartyType partyType) :
    Party(partyId),
    partyType_(partyType),
    isClosePublicEvent_(false)
{
}


void GroupParty::initialize(const OnlineUserMap& onlineUserMap, const PartyMemberInfoMap& memberMap)
{
    assert(onlineUserMap.size() <= maxPartyGroupMember);
    assert(memberMap.size() <= maxPartyGroupMember);
    
    groups_.fill(PartyGroup());

    onlineUserMap_ = onlineUserMap;
    memberMap_ = memberMap;
    
    notifyInitializePosition(groups_[startPartyGroupIndex]);
}


PartyPosition GroupParty::getJoinPartyPosition() const
{
    for (int8_t i = 0; i < maxPartyGroupCount; ++i) {
        int8_t position = groups_[i].getEmptyPosition();
        if (position != -1) {
            return PartyPosition(i, position);
        }
    }
    return PartyPosition();
}


bool GroupParty::shouldDestroy() const
{
    if (isRaidParty(partyType_)) {
        return onlineUserMap_.size() <= 1;
    }
    else if (isPublicParty(partyType_)) {
        return isClosePublicEvent_ && onlineUserMap_.size() < 2;
    }
    assert(false);
    return true;
}


bool GroupParty::canAddMember() const
{
    return maxTotalGroupPartyMember > memberMap_.size();
}


ErrorCode GroupParty::canChangeParty(PartyType partyType) const
{
    if (isPublicParty(partyType_)) {
        return ecPartyInvalidPartyType;
    }

    if (! isSoloParty(partyType)) {
        return ecPartyInvalidPartyType;
    }

    if (maxPartyGroupMember < memberMap_.size()) {
        return ecPartyIsOverPartyMemberRaidPartyToParty;
    }

    return ecOk;
}


bool GroupParty::canInviteGrade(ObjectId playerId) const
{
    if (isPublicParty(partyType_)) {
        return true;
    }

    if (master_ && master_->getPlayerId() == playerId) {
        return true;
    }

    return false;
}


bool GroupParty::canMoveOtherParty() const
{
    if (isPublicParty(partyType_)) {
        return memberMap_.size() < 2;
    }
    return false;
}


ErrorCode GroupParty::addMember(const PartyMemberInfo& memberInfo, CommunityUser* user)
{
    if (! canAddMember()) {
        return ecPartyMaxOverUser;
    }

    const ErrorCode errorCode = Party::addMember(memberInfo, user);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    changePartyGroup(memberInfo.partyPosition_);
    
    return ecOk;
}


void GroupParty::removeMember(ObjectId memberId)
{ 
    const PartyMemberInfo* memberInfo = getMemberInfo(memberId);
    if (memberInfo == nullptr) {
        return;
    }
    bool isPublicPartyGroup = false;
    if (isPublicParty(partyType_)) {
        isPublicPartyGroup = memberMap_.size() > maxPartyGroupMember;
    }
    
    PartyPosition position = memberInfo->partyPosition_;
    Party::removeMember(memberId);
    if (isPublicPartyGroup && memberMap_.size() <= maxPartyGroupMember) {
        groups_.fill(PartyGroup());        
        notifyInitializePosition(groups_[startPartyGroupIndex]);
    }
    else {
        changePartyGroup(position);
    }  
}


ErrorCode GroupParty::moveRaidPartyGroupPosition(ObjectId memberId, const PartyPosition& position)
{
    if (! position.isValid()) {
        return ecPartyInvalidGroupPosition;
    }

    if (groups_[position.groupIndex_].position_[position.position_]) {
        return ecPartyInvalidGroupPosition;
    }

    PartyMemberInfo* partyMember = getMemberInfo(memberId);
    if (partyMember == nullptr) {
        return ecPartyIsNotMember;
    }

    changePartyGroup(partyMember->partyPosition_);
    changePartyGroup(position);
    partyMember->partyPosition_ = position;


    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->partyGroupPositionMoved(memberId, position);        
    }
    return ecOk;
}


ErrorCode GroupParty::switchRaidPartyGroupPosition(ObjectId srcId, ObjectId descId)
{
    PartyMemberInfo* srcMember = getMemberInfo(srcId);
    if (srcMember == nullptr) {
        return ecPartyIsNotMember;
    }

    PartyMemberInfo* descMember = getMemberInfo(descId);
    if (descMember == nullptr) {
        return ecPartyIsNotMember;
    }

    if (srcMember == descMember) {
        return ecPartyGroupNotSwitchSameMember;
    }

    std::swap(srcMember->partyPosition_, descMember->partyPosition_);

    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->partyGroupPositionSwitched(srcId, descId);        
    }
    return ecOk;
}


void GroupParty::closePublicEvent()
{
    if (isPublicParty(partyType_)) {
        isClosePublicEvent_ = true;
    }
}


void GroupParty::changePartyGroup(const PartyPosition& position)
{
    groups_[position.groupIndex_].toggle(position.position_);
}


}} // namespace gideon { namespace communityserver {
