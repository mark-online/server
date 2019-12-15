#include "CommunityServerPCH.h"
#include "Party.h"
#include "../user/CommunityUser.h"
#include "../s2s/CommunityServerSideProxyManager.h"

namespace gideon { namespace communityserver {

Party::Party(PartyId partyId) :
    partyId_(partyId),
    master_(nullptr)
{
}


ErrorCode Party::addMember(const PartyMemberInfo& memberInfo, CommunityUser* user)
{
    if (memberMap_.find(memberInfo.playerId_) != memberMap_.end()) {
        return ecPartyAlreadyMember;
    }

    memberMap_.emplace(memberInfo.playerId_, memberInfo);
    onlineUserMap_.emplace(memberInfo.playerId_, user);

    COMMUNITYSERVERSIDEPROXY_MANAGER->partyMemberJoined(partyId_, memberInfo.playerId_);

    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        if (memberUser->getPlayerId() == user->getPlayerId()) {
            memberUser->partyJoined(getPartyMembers(), getPartyType());
        }
        else {
            memberUser->partyMemberAdded(memberInfo);
        }
    }

    return ecOk;
}


void Party::removeMember(ObjectId memberId)
{
    OnlineUserMap::const_iterator pos = onlineUserMap_.find(memberId);
    if (pos != onlineUserMap_.end()) {
        COMMUNITYSERVERSIDEPROXY_MANAGER->partyMemberLeft(partyId_, memberId);
        memberMap_.erase(memberId);
        onlineUserMap_.erase(memberId);
    }
}


PartyMemberGrade Party::getJoinMemberGrade() const
{
    return master_ != nullptr ? pmgGeneral : pmgMaster;
}


const PartyMemberInfo* Party::getMemberInfo(ObjectId memberId) const
{
    PartyMemberInfoMap::const_iterator pos = memberMap_.find(memberId);
    if (pos != memberMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


PartyMemberInfo* Party::getMemberInfo(ObjectId memberId)
{
    PartyMemberInfoMap::iterator pos = memberMap_.find(memberId);
    if (pos != memberMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


PartyMemberInfos Party::getPartyMembers() const
{
    PartyMemberInfos memberInfos;
    memberInfos.reserve(memberMap_.size());
    for (const PartyMemberInfoMap::value_type& value : memberMap_) {
        memberInfos.push_back(value.second);
    }
    return memberInfos;
}


const CommunityUser* Party::getOnlineUser(ObjectId memberId) const
{
    OnlineUserMap::const_iterator pos = onlineUserMap_.find(memberId);
    if (pos != onlineUserMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


CommunityUser* Party::getOnlineUser(ObjectId memberId)
{
    OnlineUserMap::iterator pos = onlineUserMap_.find(memberId);
    if (pos != onlineUserMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


CommunityUser* Party::getFirstOnlineUser()
{
    OnlineUserMap::iterator pos = onlineUserMap_.begin();
    if (pos != onlineUserMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


bool Party::isMaster(ObjectId playerId) const
{
    if (master_ == nullptr) {
        return false;
    }
    return master_->getPlayerId() == playerId;
}


void Party::notifyInitializePosition(PartyGroup& group)
{
    int8_t i = 0;
    PartyPositionMap positionMap;
    for (PartyMemberInfoMap::value_type& value : memberMap_) {
        PartyMemberInfo& info = value.second;
        info.partyPosition_ = PartyPosition(startPartyGroupIndex, i);
        positionMap.emplace(info.playerId_, info.partyPosition_);
        group.toggle(i++);
    }

    for (const OnlineUserMap::value_type& value : onlineUserMap_) {
        CommunityUser* memberUser = value.second;
        memberUser->partyGroupPositionInitialized(positionMap);        
    }
}

}} // namespace gideon { namespace communityserver {
