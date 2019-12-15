#pragma once

#include "Party.h"

namespace gideon { namespace communityserver {


/***
 * @class SoloParty
 ***/
class SoloParty : public Party
{
public:
    SoloParty(PartyId partyId);

    virtual void initialize(const OnlineUserMap& onlineUserMap, const PartyMemberInfoMap& memberMap);
    virtual PartyPosition getJoinPartyPosition() const;
    virtual bool shouldDestroy() const;
    virtual bool canAddMember() const;
    virtual ErrorCode canChangeParty(PartyType partyType) const;
    virtual bool canInviteGrade(ObjectId playerId) const;
    virtual bool canMoveOtherParty() const {
        return false;
    }
    virtual ErrorCode addMember(const PartyMemberInfo& memberInfo, CommunityUser* user);
    virtual void removeMember(ObjectId memberId);

    virtual PartyType getPartyType() const {
        return ptSolo;
    }

private:
    PartyGroup group_;
};

}} // namespace gideon { namespace communityserver {