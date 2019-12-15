#pragma once

#include <gideon/cs/shared/data/PlayerInfo.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace communityserver {

class CommunityUser;

struct PartyGroup
{
    typedef std::array<bool, maxPartyGroupMember> GroupPosition;
    GroupPosition position_;

    PartyGroup() {
        position_.fill(false);
    }

    int8_t getEmptyPosition() const {
        for (int8_t i = 0; i < maxPartyGroupMember; ++i) {
            if (! position_[i]) {
                return i;
            }
        }
        return -1;
    }

    void toggle(int8_t index) {
        position_[index] = (! position_[index]); 
    }
};


/***
 * @class Party
 ***/
class Party
{
public:
    typedef sne::core::Map<ObjectId, CommunityUser*> OnlineUserMap;

public:
    Party(PartyId partyId);

    /// 외부에서 정보를 얻어올때
    virtual void initialize(const OnlineUserMap& onlineUserMap, const PartyMemberInfoMap& memberMap) = 0;

    virtual PartyPosition getJoinPartyPosition() const = 0;
    virtual bool shouldDestroy() const = 0;

    /// 맴버 추가 가능한가?
    virtual bool canAddMember() const = 0;

    /// 파티 타입을 변경할수 있는가?
    virtual ErrorCode canChangeParty(PartyType partyType) const = 0;
    
    /// 파티 초개 가능한가?
    virtual bool canInviteGrade(ObjectId playerId) const = 0;

    /// 다른 파티로 이동이 가능한가
    virtual bool canMoveOtherParty() const = 0;

    virtual PartyType getPartyType() const = 0;
public:
    virtual ErrorCode addMember(const PartyMemberInfo& memberInfo, CommunityUser* user);
    virtual void removeMember(ObjectId memberId);
    virtual ErrorCode moveRaidPartyGroupPosition(ObjectId /*memberId*/, const PartyPosition& /*position*/) {
        return ecPartyGroupNotChangeInvalidPartyType;
    }
    virtual ErrorCode switchRaidPartyGroupPosition(ObjectId /*src*/, ObjectId /*desc*/) {
        return ecPartyGroupNotChangeInvalidPartyType;
    }
    virtual void closePublicEvent() {}

    PartyMemberGrade getJoinMemberGrade() const;
    const PartyMemberInfo* getMemberInfo(ObjectId memberId) const;
    
    PartyMemberInfo* getMemberInfo(ObjectId memberId);   
    PartyMemberInfos getPartyMembers() const;

    const CommunityUser* getOnlineUser(ObjectId memberId) const;
    CommunityUser* getOnlineUser(ObjectId memberId);
    CommunityUser* getFirstOnlineUser();
    CommunityUser* getMaster() {
        return master_;
    }

    const OnlineUserMap& getOnlineUserMap() const {
        return onlineUserMap_;
    }
    const PartyMemberInfoMap& getPartyMemberInfoMap() const {
        return memberMap_;
    }

    PartyId getPartyId() const {
        return partyId_;
    }

    bool isMaster(ObjectId playerId) const;

    void setMaster(CommunityUser* master) {
        master_ = master;
    }

    bool isEmptyMember() const {
        return memberMap_.empty();
    }
protected:
    void notifyInitializePosition(PartyGroup& positionMap);
protected:
    PartyId partyId_;
    OnlineUserMap onlineUserMap_;
    PartyMemberInfoMap memberMap_;
    CommunityUser* master_; 
};

}} // namespace gideon { namespace communityserver {
