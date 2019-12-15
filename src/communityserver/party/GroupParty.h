#pragma once

#include "Party.h"

namespace gideon { namespace communityserver {
    

/***
 * @class GroupParty
 ***/
class GroupParty : public Party
{
public:
    typedef std::array<PartyGroup, maxPartyGroupCount> PartyGroups;
    GroupParty(PartyId partyId, PartyType partyType);
private:
    virtual void initialize(const OnlineUserMap& onlineUserMap, const PartyMemberInfoMap& memberMap);
    virtual PartyPosition getJoinPartyPosition() const;
    virtual bool shouldDestroy() const;
    virtual bool canAddMember() const;
    virtual ErrorCode canChangeParty(PartyType partyType) const;
    virtual bool canInviteGrade(ObjectId playerId) const;
    virtual bool canMoveOtherParty() const;
    virtual PartyType getPartyType() const {
        return partyType_;
    }
protected:
    virtual ErrorCode addMember(const PartyMemberInfo& memberInfo, CommunityUser* user);
    virtual void removeMember(ObjectId memberId);
    virtual ErrorCode moveRaidPartyGroupPosition(ObjectId memberId, const PartyPosition& position);
    virtual ErrorCode switchRaidPartyGroupPosition(ObjectId src, ObjectId desc);
    virtual void closePublicEvent();

protected:
    void changePartyGroup(const PartyPosition& position);

protected:
    PartyType partyType_;
    PartyGroups groups_;    
    bool isClosePublicEvent_; /// 퍼블릭 파티일경우에만 사용한다
};

}} // namespace gideon { namespace communityserver {